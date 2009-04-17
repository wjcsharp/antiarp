/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop

#pragma NDIS_PAGEABLE_FUNCTION(MiniportInitialize4)
#pragma NDIS_PAGEABLE_FUNCTION(MiniportHalt4)
#pragma NDIS_PAGEABLE_FUNCTION(MiniportFreeAllPacketPools4)

NDIS_STATUS
MiniportInitialize4(
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
                             0,                                        // CheckForHangTimeInSeconds
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

NDIS_STATUS
MiniportQueryInformation4(
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
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        if (Oid == OID_GEN_SUPPORTED_GUIDS)
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        if (Oid == OID_TCP_TASK_OFFLOAD)
        {

        }

        NdisAcquireSpinLock(&pAdapt->Lock);
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pAdapt->Lock);

        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0) 
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        pAdapt->Request4.RequestType = NdisRequestQueryInformation;
        pAdapt->Request4.DATA.QUERY_INFORMATION.Oid = Oid;
        pAdapt->Request4.DATA.QUERY_INFORMATION.InformationBuffer = InformationBuffer;
        pAdapt->Request4.DATA.QUERY_INFORMATION.InformationBufferLength = InformationBufferLength;
        pAdapt->BytesNeeded = BytesNeeded;
        pAdapt->BytesReadOrWritten = BytesWritten;

        NdisAcquireSpinLock(&pAdapt->Lock);
            
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if ((pAdapt->PTDeviceState > NdisDeviceStateD0) 
                && (pAdapt->StandingBy == FALSE))
        {
            pAdapt->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }

        if (pAdapt->StandingBy == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        pAdapt->OutstandingRequests = TRUE;
        
        NdisReleaseSpinLock(&pAdapt->Lock);

        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    (PVOID)&pAdapt->Request4);


        if (Status != NDIS_STATUS_PENDING)
        {
            ProtocolRequestComplete4(pAdapt, &pAdapt->Request4, Status);
            Status = NDIS_STATUS_PENDING;
        }

    } while (FALSE);

    return(Status);

}


VOID
MiniportQueryPNPCapabilities4(
    IN OUT PADAPTER          pAdapt,
    OUT PNDIS_STATUS         pStatus
    )
{
    PNDIS_PNP_CAPABILITIES           pPNPCapabilities;
    PNDIS_PM_WAKE_UP_CAPABILITIES    pPMstruct;

    if (pAdapt->Request4.DATA.QUERY_INFORMATION.InformationBufferLength >= sizeof(NDIS_PNP_CAPABILITIES))
    {
        pPNPCapabilities = (PNDIS_PNP_CAPABILITIES)(pAdapt->Request4.DATA.QUERY_INFORMATION.InformationBuffer);

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


NDIS_STATUS
MiniportSetInformation4(
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
        if (Oid == OID_PNP_SET_POWER)
        {
            MiniportProcessSetPowerOid4(&Status, 
                                 pAdapt, 
                                 InformationBuffer, 
                                 InformationBufferLength, 
                                 BytesRead, 
                                 BytesNeeded);
            break;

        }

        NdisAcquireSpinLock(&pAdapt->Lock);     
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pAdapt->Lock);

        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        pAdapt->Request4.RequestType = NdisRequestSetInformation;
        pAdapt->Request4.DATA.SET_INFORMATION.Oid = Oid;
        pAdapt->Request4.DATA.SET_INFORMATION.InformationBuffer = InformationBuffer;
        pAdapt->Request4.DATA.SET_INFORMATION.InformationBufferLength = InformationBufferLength;
        pAdapt->BytesNeeded = BytesNeeded;
        pAdapt->BytesReadOrWritten = BytesRead;

        NdisAcquireSpinLock(&pAdapt->Lock);     
        if (pAdapt->UnbindingInProcess == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        
        if ((pAdapt->PTDeviceState > NdisDeviceStateD0) 
                && (pAdapt->StandingBy == FALSE))
        {
            pAdapt->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }

        if (pAdapt->StandingBy == TRUE)
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        pAdapt->OutstandingRequests = TRUE;
        
        NdisReleaseSpinLock(&pAdapt->Lock);

        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    (PVOID)&pAdapt->Request4);

        if (Status != NDIS_STATUS_PENDING)
        {
            *BytesRead = pAdapt->Request4.DATA.SET_INFORMATION.BytesRead;
            *BytesNeeded = pAdapt->Request4.DATA.SET_INFORMATION.BytesNeeded;
            pAdapt->OutstandingRequests = FALSE;
        }

    } while (FALSE);

    return(Status);
}


VOID
MiniportProcessSetPowerOid4(
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

        if (InformationBufferLength < sizeof(NDIS_DEVICE_POWER_STATE))
        {
            *pNdisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        NewDeviceState = (*(PNDIS_DEVICE_POWER_STATE)InformationBuffer);

        if ((pAdapt->MiniportDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0))
        {

            ASSERT (!(pAdapt->MiniportDeviceState > NdisDeviceStateD0) && (NewDeviceState != NdisDeviceStateD0));

            *pNdisStatus = NDIS_STATUS_FAILURE;
            break;
        }    

        if (pAdapt->MiniportDeviceState == NdisDeviceStateD0 && NewDeviceState > NdisDeviceStateD0)
        {
            pAdapt->StandingBy = TRUE;
        }

        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0 &&  NewDeviceState == NdisDeviceStateD0)
        {
            pAdapt->StandingBy = FALSE;
        }
        
        pAdapt->MiniportDeviceState = NewDeviceState;
        
        *pNdisStatus = NDIS_STATUS_SUCCESS;
    

    } while (FALSE);    
        
    if (*pNdisStatus == NDIS_STATUS_SUCCESS)
    {
        if (pAdapt->StandingBy == FALSE)
        {
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


VOID
MiniportReturnPacket4(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PNDIS_PACKET            Packet
    )
{
    PADAPTER            pAdapt = (PADAPTER)MiniportAdapterContext;

    {
        PNDIS_PACKET    MyPacket;
        PRECV_RSVD      RecvRsvd;
    
        RecvRsvd = (PRECV_RSVD)(Packet->MiniportReserved);
        MyPacket = RecvRsvd->OriginalPkt;
    
        NdisFreePacket(Packet);

		if(MyPacket)
			NdisReturnPackets(&MyPacket, 1);
    }
}


NDIS_STATUS
MiniportTransferData4(
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

VOID
MiniportHalt4(
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
    if (pAdapt->BindingHandle != NULL)
    {
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

    MiniportFreeAllPacketPools4(pAdapt);
    NdisFreeSpinLock(&pAdapt->Lock);
    NdisFreeMemory(pAdapt, 0, 0);

}


VOID
MiniportFreeAllPacketPools4(
    IN PADAPTER           pAdapt
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
				MiniportReturnPacket4(pAdapt, PacketArray[i].pNdisPacket);
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

