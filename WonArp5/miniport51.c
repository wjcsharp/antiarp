/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop

#pragma NDIS_PAGEABLE_FUNCTION(MiniportInitialize5)
#pragma NDIS_PAGEABLE_FUNCTION(MiniportHalt5)
#pragma NDIS_PAGEABLE_FUNCTION(MiniportFreeAllPacketPools5)

/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
MiniportInitialize5(
    OUT PNDIS_STATUS             OpenErrorStatus,
    OUT PUINT                    SelectedMediumIndex,
    IN  PNDIS_MEDIUM             MediumArray,
    IN  UINT                     MediumArraySize,
    IN  NDIS_HANDLE              MiniportAdapterHandle,
    IN  NDIS_HANDLE              WrapperConfigurationContext
    )
{
    UINT            i;
    PADAPTER        pAdapt;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    NDIS_MEDIUM     Medium;

    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
    
    do
    {
        pAdapt = NdisIMGetDeviceContext(MiniportAdapterHandle);
        pAdapt->MiniportHandle = MiniportAdapterHandle;

        Medium = pAdapt->Medium;

        if (Medium == NdisMediumWan)
        {
            Medium = NdisMedium802_3;
			pAdapt->bWanAdapter = TRUE;
        }

        for (i = 0; i < MediumArraySize; i++)
        {
            if (MediumArray[i] == Medium)
            {
                *SelectedMediumIndex = i;
                break;
            }
        }

        if (i == MediumArraySize)
        {
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }

        NdisMSetAttributesEx(MiniportAdapterHandle,
                             pAdapt,
                             0,              // CheckForHangTimeInSeconds
                             NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT    |
                                NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT|
                                NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
                                NDIS_ATTRIBUTE_DESERIALIZE |
                                NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                             0);


        pAdapt->LastIndicatedStatus = NDIS_STATUS_MEDIA_CONNECT;
        
        pAdapt->MiniportDeviceState = NdisDeviceStateD0;

        //
        // Create an ioctl interface
        //
        (VOID)ProtocolRegisterDevice();
        
		Status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    ASSERT(pAdapt->MiniportInitPending == TRUE);
    pAdapt->MiniportInitPending = FALSE;
    NdisSetEvent(&pAdapt->MiniportInitEvent);

    *OpenErrorStatus = Status;
    
    return Status;
}


/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
MiniportQueryInformation5(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesWritten,
    OUT PULONG                    BytesNeeded
    )
{
    PADAPTER      pAdapt = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS   Status = NDIS_STATUS_FAILURE;

    do
    {
        if (Oid == OID_PNP_QUERY_POWER)
        {
            //
            //  Do not forward this.
            //
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        if (Oid == OID_GEN_SUPPORTED_GUIDS)
        {
            //
            //  Do not forward this, otherwise we will end up with multiple
            //  instances of private GUIDs that the underlying miniport
            //  supports.
            //
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        if (Oid == OID_TCP_TASK_OFFLOAD)
        {
            //
            // Fail this -if- this driver performs data transformations
            // that can interfere with a lower driver's ability to offload
            // TCP tasks.
            //
            // Status = NDIS_STATUS_NOT_SUPPORTED;
            // break;
            //
        }
        //
        // If the miniport below is unbinding, just fail any request
        //
        NdisAcquireSpinLock(&pAdapt->Lock);
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pAdapt->Lock);
        //
        // All other queries are failed, if the miniport is not at D0,
        //
        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0) 
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        pAdapt->Request5.RequestType = NdisRequestQueryInformation;
        pAdapt->Request5.DATA.QUERY_INFORMATION.Oid = Oid;
        pAdapt->Request5.DATA.QUERY_INFORMATION.InformationBuffer = InformationBuffer;
        pAdapt->Request5.DATA.QUERY_INFORMATION.InformationBufferLength = InformationBufferLength;
        pAdapt->BytesNeeded = BytesNeeded;
        pAdapt->BytesReadOrWritten = BytesWritten;

        //
        // If the miniport below is binding, fail the request
        //
        NdisAcquireSpinLock(&pAdapt->Lock);
            
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        //
        // If the Protocol device state is OFF, mark this request as being 
        // pended. We queue this until the device state is back to D0. 
        //
        if ((pAdapt->PTDeviceState > NdisDeviceStateD0) 
                && (pAdapt->StandingBy == FALSE))
        {
            pAdapt->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }
        //
        // This is in the process of powering down the system, always fail the request
        // 
        if (pAdapt->StandingBy == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        pAdapt->OutstandingRequests = TRUE;
        
        NdisReleaseSpinLock(&pAdapt->Lock);

        //
        // default case, most requests will be passed to the miniport below
        //
        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    (PVOID)&pAdapt->Request5);


        if (Status != NDIS_STATUS_PENDING)
        {
            ProtocolRequestComplete5(pAdapt, (PVOID)&pAdapt->Request5, Status);
            Status = NDIS_STATUS_PENDING;
        }

    } while (FALSE);

    return(Status);

}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportQueryPNPCapabilities5(
    IN OUT PADAPTER            pAdapt,
    OUT PNDIS_STATUS         pStatus
    )
{
    PNDIS_PNP_CAPABILITIES           pPNPCapabilities;
    PNDIS_PM_WAKE_UP_CAPABILITIES    pPMstruct;

    if (pAdapt->Request5.DATA.QUERY_INFORMATION.InformationBufferLength >= sizeof(NDIS_PNP_CAPABILITIES))
    {
        pPNPCapabilities = (PNDIS_PNP_CAPABILITIES)(pAdapt->Request5.DATA.QUERY_INFORMATION.InformationBuffer);

        //
        // The following fields must be overwritten by an IM driver.
        //
        pPMstruct= & pPNPCapabilities->WakeUpCapabilities;
        pPMstruct->MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinPatternWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinLinkChangeWakeUp = NdisDeviceStateUnspecified;
        *pAdapt->BytesReadOrWritten = sizeof(NDIS_PNP_CAPABILITIES);
        *pAdapt->BytesNeeded = 0;

        *pStatus = NDIS_STATUS_SUCCESS;

    }
    else
    {
        *pAdapt->BytesNeeded= sizeof(NDIS_PNP_CAPABILITIES);
        *pStatus = NDIS_STATUS_RESOURCES;
    }
}

/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
MiniportSetInformation5(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN NDIS_OID                Oid,
    IN PVOID                   InformationBuffer,
    IN ULONG                   InformationBufferLength,
    OUT PULONG                 BytesRead,
    OUT PULONG                 BytesNeeded
    )
{
    PADAPTER        pAdapt = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS   Status;

    Status = NDIS_STATUS_FAILURE;

    do
    {
        //
        // The Set Power should not be sent to the miniport below the Passthru, but is handled internally
        //
        if (Oid == OID_PNP_SET_POWER)
        {
            MiniportProcessSetPowerOid5(&Status, 
                                 pAdapt, 
                                 InformationBuffer, 
                                 InformationBufferLength, 
                                 BytesRead, 
                                 BytesNeeded);
            break;

        }

        //
        // If the miniport below is unbinding, fail the request
        //
        NdisAcquireSpinLock(&pAdapt->Lock);     
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pAdapt->Lock);
        //
        // All other Set Information requests are failed, if the miniport is
        // not at D0 or is transitioning to a device state greater than D0.
        //
        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        // Set up the Request and return the result
        pAdapt->Request5.RequestType = NdisRequestSetInformation;
        pAdapt->Request5.DATA.SET_INFORMATION.Oid = Oid;
        pAdapt->Request5.DATA.SET_INFORMATION.InformationBuffer = InformationBuffer;
        pAdapt->Request5.DATA.SET_INFORMATION.InformationBufferLength = InformationBufferLength;
        pAdapt->BytesNeeded = BytesNeeded;
        pAdapt->BytesReadOrWritten = BytesRead;

        //
        // If the miniport below is unbinding, fail the request
        //
        NdisAcquireSpinLock(&pAdapt->Lock);     
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
            
        //
        // If the device below is at a low power state, we cannot send it the
        // request now, and must pend it.
        //
        if ((pAdapt->PTDeviceState > NdisDeviceStateD0) 
                && (pAdapt->StandingBy == FALSE))
        {
            pAdapt->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }
        //
        // This is in the process of powering down the system, always fail the request
        // 
        if (pAdapt->StandingBy == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        pAdapt->OutstandingRequests = TRUE;
        
        NdisReleaseSpinLock(&pAdapt->Lock);
        //
        // Forward the request to the device below.
        //
        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    (PVOID)&pAdapt->Request5);

        if (Status != NDIS_STATUS_PENDING)
        {
            *BytesRead = pAdapt->Request5.DATA.SET_INFORMATION.BytesRead;
            *BytesNeeded = pAdapt->Request5.DATA.SET_INFORMATION.BytesNeeded;
            pAdapt->OutstandingRequests = FALSE;
        }

    } while (FALSE);

    return(Status);
}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportProcessSetPowerOid5(
    IN OUT PNDIS_STATUS          pNdisStatus,
    IN PADAPTER                  pAdapt,
    IN PVOID                     InformationBuffer,
    IN ULONG                     InformationBufferLength,
    OUT PULONG                   BytesRead,
    OUT PULONG                   BytesNeeded
    )
{
    
    NDIS_DEVICE_POWER_STATE NewDeviceState;

    ASSERT (InformationBuffer != NULL);

    *pNdisStatus = NDIS_STATUS_FAILURE;

    do 
    {
        //
        // Check for invalid length
        //
        if (InformationBufferLength < sizeof(NDIS_DEVICE_POWER_STATE))
        {
            *pNdisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        NewDeviceState = (*(PNDIS_DEVICE_POWER_STATE)InformationBuffer);

        //
        // Check for invalid device state
        //
        if ((pAdapt->MiniportDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0))
        {
            //
            // If the miniport is in a non-D0 state, the miniport can only receive a Set Power to D0
            //
            ASSERT (!(pAdapt->MiniportDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0));

            *pNdisStatus = NDIS_STATUS_FAILURE;
            break;
        }    

        //
        // Is the miniport transitioning from an On (D0) state to an Low Power State (>D0)
        // If so, then set the StandingBy Flag - (Block all incoming requests)
        //
        if (pAdapt->MiniportDeviceState == NdisDeviceStateD0 && NewDeviceState > NdisDeviceStateD0)
        {
            pAdapt->StandingBy = TRUE;
        }

        //
        // If the miniport is transitioning from a low power state to ON (D0), then clear the StandingBy flag
        // All incoming requests will be pended until the physical miniport turns ON.
        //
        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0 &&  NewDeviceState == NdisDeviceStateD0)
        {
            pAdapt->StandingBy = FALSE;
        }
        
        //
        // Now update the state in the pAdapt structure;
        //
        pAdapt->MiniportDeviceState = NewDeviceState;
        
        *pNdisStatus = NDIS_STATUS_SUCCESS;
    

    } while (FALSE);    
        
    if (*pNdisStatus == NDIS_STATUS_SUCCESS)
    {
        //
        // The miniport resume from low power state
        // 
        if (pAdapt->StandingBy == FALSE)
        {
            //
            // If we need to indicate the media connect state
            // 
            if (pAdapt->LastIndicatedStatus != pAdapt->LatestUnIndicateStatus)
            {
               NdisMIndicateStatus(pAdapt->MiniportHandle,
                                        pAdapt->LatestUnIndicateStatus,
                                        (PVOID)NULL,
                                        0);
               NdisMIndicateStatusComplete(pAdapt->MiniportHandle);
               pAdapt->LastIndicatedStatus = pAdapt->LatestUnIndicateStatus;
            }
        }
        else
        {
            //
            // Initialize LatestUnIndicatedStatus
            //
            pAdapt->LatestUnIndicateStatus = pAdapt->LastIndicatedStatus;
        }
        *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
        *BytesNeeded = 0;
    }
    else
    {
        *BytesRead = 0;
        *BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE);
    }

}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportReturnPacket5(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PNDIS_PACKET            Packet
    )
{
    PADAPTER            pAdapt = (PADAPTER)MiniportAdapterContext;

    //
    // Packet stacking: Check if this packet belongs to us.
    //
    if (MyNdisGetPoolFromPacket(Packet) != pAdapt->RecvPacketPoolHandle)
    {
        //
        // We reused the original packet in a receive indication.
        // Simply return it to the miniport below us.
        //
        NdisReturnPackets(&Packet, 1);
    }
    else
    {
        //
        // This is a packet allocated from this IM's receive packet pool.
        // Reclaim our packet, and return the original to the driver below.
        //

        PNDIS_PACKET    MyPacket;
        PRECV_RSVD      RecvRsvd;
    
        RecvRsvd = (PRECV_RSVD)(Packet->MiniportReserved);
        MyPacket = RecvRsvd->OriginalPkt;
    
        NdisFreePacket(Packet);

		if(MyPacket)
			NdisReturnPackets(&MyPacket, 1);
    }
}

/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
MiniportTransferData5(
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred,
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_HANDLE              MiniportReceiveContext,
    IN UINT                     ByteOffset,
    IN UINT                     BytesToTransfer
    )
{
    PADAPTER      pAdapt = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS   Status;

    //
    // Return, if the device is OFF
    //

    if (IsIMDeviceStateOn(pAdapt) == FALSE)
    {
        return NDIS_STATUS_FAILURE;
    }

    NdisTransferData(&Status,
                     pAdapt->BindingHandle,
                     MiniportReceiveContext,
                     ByteOffset,
                     BytesToTransfer,
                     Packet,
                     BytesTransferred);

    return(Status);
}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportHalt5(
    IN NDIS_HANDLE                MiniportAdapterContext
    )
{
    PADAPTER           pAdapt = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS        Status;


    //
    // Delete the ioctl interface that was created when the miniport
    // was created.
    //
    (VOID)ProtocolDeregisterDevice();

#ifdef _DEBUG
	// 关闭直接包发送接口
	if(pAdapt->FileObject)
	{
		//
		//  Make sure any threads trying to send have finished.
		//
		pAdapt->FileObject->FsContext = NULL;
		pAdapt->FileObject = NULL;
	}
#endif

    //
    // If we have a valid bind, close the miniport below the protocol
    //
    if (pAdapt->BindingHandle != NULL)
    {
        //
        // Close the binding below. and wait for it to complete
        //
        NdisResetEvent(&pAdapt->Event);

        NdisCloseAdapter(&Status, pAdapt->BindingHandle);

        if (Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&pAdapt->Event, 0);
            Status = pAdapt->Status;
        }

        ASSERT (Status == NDIS_STATUS_SUCCESS);

        pAdapt->BindingHandle = NULL;
    }

    //
    //  Free all resources on this adapter structure.
    //
    MiniportFreeAllPacketPools5(pAdapt);
    NdisFreeSpinLock(&pAdapt->Lock);
    NdisFreeMemory(pAdapt, 0, 0);

}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportCancelSendPackets5(
    IN NDIS_HANDLE            MiniportAdapterContext,
    IN PVOID                  CancelId
    )
{
    PADAPTER    pAdapt = (PADAPTER)MiniportAdapterContext;

    MyNdisCancelSendPackets(pAdapt->BindingHandle, CancelId);

    return;
}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportDevicePnPEvent5(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_DEVICE_PNP_EVENT    DevicePnPEvent,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength
    )
{
    // TBD - add code/comments about processing this.

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(DevicePnPEvent);
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    
    return;
}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportAdapterShutdown5(
    IN NDIS_HANDLE                MiniportAdapterContext
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    
    return;
}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportFreeAllPacketPools5(
    IN PADAPTER                    pAdapt
    )
{
	RECEIVED_PACKET			PacketArray[MAX_RECV_PACKET_POOL_SIZE];
    ULONG					NumberOfPackets = 0, i;

	//释放所有已经接收并存储的包
	NdisAcquireSpinLock(&pAdapt->Lock);
	if(pAdapt->ReceivedPacketCount > 0)
	{
		NumberOfPackets = pAdapt->ReceivedPacketCount;
		
		NdisMoveMemory(PacketArray,
						pAdapt->ReceivedPackets,
						NumberOfPackets * sizeof(RECEIVED_PACKET));
		pAdapt->ReceivedPacketCount = 0;
		for(i=0;i<NumberOfPackets;i++)
		{
			if(PacketArray[i].pNdisPacket && PacketArray[i].bReturnPacket)
			{
				MiniportReturnPacket5(pAdapt, PacketArray[i].pNdisPacket);
			}
		}
	}
	NdisReleaseSpinLock(&pAdapt->Lock);

    if (pAdapt->RecvPacketPoolHandle != NULL)
    {
        NdisFreePacketPool(pAdapt->RecvPacketPoolHandle);
        pAdapt->RecvPacketPoolHandle = NULL;
    }

    if (pAdapt->SendPacketPoolHandle != NULL)
    {
        NdisFreePacketPool(pAdapt->SendPacketPoolHandle);
        pAdapt->SendPacketPoolHandle = NULL;
    }
	if(pAdapt->RecvBufferPool != NULL)
	{
		NdisFreeBufferPool(pAdapt->RecvBufferPool);
		pAdapt->RecvBufferPool  = NULL;
	}
	if(pAdapt->SendBufferPool != NULL)
	{
		NdisFreeBufferPool(pAdapt->SendBufferPool);
		pAdapt->SendBufferPool  = NULL;
	}
}

