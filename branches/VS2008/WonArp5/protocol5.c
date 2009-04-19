/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop


#pragma NDIS_PAGEABLE_FUNCTION(ProtocolBindAdapter5)
#pragma NDIS_PAGEABLE_FUNCTION(ProtocolUnbindAdapter5)
#pragma NDIS_PAGEABLE_FUNCTION(ProtocolOpenAdapterComplete5)
#pragma NDIS_PAGEABLE_FUNCTION(ProtocolUnloadProtocol5)

#pragma NDIS_PAGEABLE_FUNCTION(ProtocolCloseAdapterComplete5)
#pragma NDIS_PAGEABLE_FUNCTION(ProtocolPNPHandler5)
#pragma NDIS_PAGEABLE_FUNCTION(ProtocolPnPNetEventReconfigure5)
#pragma NDIS_PAGEABLE_FUNCTION(ProtocolPnPNetEventSetPower5)


#define MAX_PACKET_POOL_SIZE 0x0000FFFF
#define MIN_PACKET_POOL_SIZE 0x000000FF

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolBindAdapter5(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )
{
    NDIS_HANDLE                     ConfigHandle = NULL;
    PNDIS_CONFIGURATION_PARAMETER   Param;
    NDIS_STRING                     DeviceStr = NDIS_STRING_CONST("UpperBindings");
    PADAPTER                        pAdapt = NULL;
    NDIS_STATUS                     Sts;
    UINT                            MediumIndex;
    ULONG                           TotalSize;
    BOOLEAN                         LockAllocated = FALSE;

    UNREFERENCED_PARAMETER(BindContext);
    UNREFERENCED_PARAMETER(SystemSpecific2);

	KdPrint((" BindAdapter : %S \n",DeviceName->Buffer));

    do
    {
        NdisOpenProtocolConfiguration(Status,
                                       &ConfigHandle,
                                       SystemSpecific1);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }


        NdisReadConfiguration(Status,
                              &Param,
                              ConfigHandle,
                              &DeviceStr,
                              NdisParameterString);
        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }


        TotalSize = sizeof(ADAPTER) + DeviceName->MaximumLength + 
									Param->ParameterData.StringData.MaximumLength ;

        NdisAllocateMemoryWithTag(&pAdapt, TotalSize, TAG);

        if (pAdapt == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(pAdapt, TotalSize);

        pAdapt->DeviceName.MaximumLength = DeviceName->MaximumLength;
        pAdapt->DeviceName.Length = DeviceName->Length;
        pAdapt->DeviceName.Buffer = (PWCHAR)((ULONG_PTR)pAdapt + sizeof(ADAPTER));
        NdisMoveMemory(pAdapt->DeviceName.Buffer,
                       DeviceName->Buffer,
                       DeviceName->MaximumLength);

		_wcsupr(pAdapt->DeviceName.Buffer);

        pAdapt->UpperBindingDevice.MaximumLength = Param->ParameterData.StringData.MaximumLength;
        pAdapt->UpperBindingDevice.Length = Param->ParameterData.StringData.Length;
        pAdapt->UpperBindingDevice.Buffer = 
				(PWCHAR)((ULONG_PTR)pAdapt + sizeof(ADAPTER) + DeviceName->MaximumLength );

        NdisMoveMemory(pAdapt->UpperBindingDevice.Buffer,
                       Param->ParameterData.StringData.Buffer,
                       Param->ParameterData.StringData.MaximumLength);




        NdisInitializeEvent(&pAdapt->Event);
        NdisAllocateSpinLock(&pAdapt->Lock);
        LockAllocated = TRUE;

        NdisAllocatePacketPoolEx(Status,
                                   &pAdapt->SendPacketPoolHandle,
                                   MIN_SEND_PACKET_POOL_SIZE,
                                   MAX_SEND_PACKET_POOL_SIZE - MIN_SEND_PACKET_POOL_SIZE,
                                   sizeof(SEND_RSVD));

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        NdisAllocatePacketPoolEx(Status,
                                   &pAdapt->RecvPacketPoolHandle,
                                   MIN_RECV_PACKET_POOL_SIZE,
                                   MAX_RECV_PACKET_POOL_SIZE - MIN_RECV_PACKET_POOL_SIZE,
                                   PROTOCOL_RESERVED_SIZE_IN_PACKET);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

		NdisAllocateBufferPool(Status,
								&pAdapt->RecvBufferPool,
								MAX_RECV_PACKET_POOL_SIZE);

		if(*Status != NDIS_STATUS_SUCCESS)
		{
			pAdapt->RecvBufferPool = NULL;
			break;
		}

		NdisAllocateBufferPool(Status,
								&pAdapt->SendBufferPool,
								MAX_SEND_PACKET_POOL_SIZE);

		if(*Status != NDIS_STATUS_SUCCESS)
		{
			KdPrint((" Allocate SendBufferPool Failed"));

			pAdapt->SendBufferPool = NULL;
			break;
		}

        //初始化网卡状态
        pAdapt->PTDeviceState		= NdisDeviceStateD0;
        pAdapt->MiniportDeviceState = NdisDeviceStateD0 + 1;  //Miniport不可用

        NdisOpenAdapter(Status,
                          &Sts,
                          &pAdapt->BindingHandle,
                          &MediumIndex,
                          MediumArray,
                          sizeof(MediumArray)/sizeof(NDIS_MEDIUM),
                          ProtHandle,
                          pAdapt,
                          DeviceName,
                          0,
                          NULL);

        if (*Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&pAdapt->Event, 0);
            *Status = pAdapt->Status;
        }

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pAdapt->Medium = MediumArray[MediumIndex];

		//加入网卡列表
        NdisAcquireSpinLock(&GlobalLock);
        pAdapt->Next	= g_AdaptList;
        g_AdaptList		= pAdapt;
        NdisReleaseSpinLock(&GlobalLock);

		//启动中间层绑定
        pAdapt->MiniportInitPending = TRUE;
        NdisInitializeEvent(&pAdapt->MiniportInitEvent);

        *Status = NdisIMInitializeDeviceInstanceEx(DriverHandle,
                                           &pAdapt->UpperBindingDevice,
                                           pAdapt);

        if (*Status != NDIS_STATUS_SUCCESS)
        {

            break;
        }

    } while(FALSE);

    if (ConfigHandle != NULL)
    {
        NdisCloseConfiguration(ConfigHandle);
    }

    if (*Status != NDIS_STATUS_SUCCESS)
    {
        if (pAdapt != NULL)
        {
            if (pAdapt->BindingHandle != NULL)
            {
                NDIS_STATUS    LocalStatus;

                NdisResetEvent(&pAdapt->Event);
                
                NdisCloseAdapter(&LocalStatus, pAdapt->BindingHandle);
                pAdapt->BindingHandle = NULL;

                if (LocalStatus == NDIS_STATUS_PENDING)
                {
                     NdisWaitEvent(&pAdapt->Event, 0);
                     LocalStatus = pAdapt->Status;
                }
            }

            if (pAdapt->SendPacketPoolHandle != NULL)
            {
                 NdisFreePacketPool(pAdapt->SendPacketPoolHandle);
            }

            if (pAdapt->RecvPacketPoolHandle != NULL)
            {
                 NdisFreePacketPool(pAdapt->RecvPacketPoolHandle);
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
            
            if (LockAllocated == TRUE)
            {
                NdisFreeSpinLock(&pAdapt->Lock);
            }

            NdisFreeMemory(pAdapt, 0, 0);
            pAdapt = NULL;
        }
    }

}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolOpenAdapterComplete5(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    )
{
    PADAPTER      pAdapt =(PADAPTER)ProtocolBindingContext;
    
    UNREFERENCED_PARAMETER(OpenErrorStatus);

    pAdapt->Status = Status;
    NdisSetEvent(&pAdapt->Event);
}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolUnbindAdapter5(
    OUT PNDIS_STATUS		   Status,
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_HANDLE            UnbindContext
    )
{
    PADAPTER       pAdapt =(PADAPTER)ProtocolBindingContext;
    NDIS_STATUS    LocalStatus;
    PNDIS_PACKET   PacketArray[MAX_RECV_PACKET_POOL_SIZE];
    ULONG          NumberOfPackets = 0, i;
    BOOLEAN        CompleteRequest = FALSE;
    BOOLEAN        ReturnPackets = FALSE;
    PADAPTER       *ppCursor;

    UNREFERENCED_PARAMETER(UnbindContext);
    
    NdisAcquireSpinLock(&pAdapt->Lock);
    pAdapt->UnbindingInProcess = TRUE;
    if (pAdapt->QueuedRequest == TRUE)
    {
        pAdapt->QueuedRequest = FALSE;
        CompleteRequest = TRUE;
    }
    if (pAdapt->ReceivedPacketCount > 0)
    {

        NdisMoveMemory(PacketArray,
                      pAdapt->ReceivedPackets,
                      pAdapt->ReceivedPacketCount * sizeof(PNDIS_PACKET));

        NumberOfPackets = pAdapt->ReceivedPacketCount;

        pAdapt->ReceivedPacketCount = 0;
        ReturnPackets = TRUE;
    }
     
    NdisReleaseSpinLock(&pAdapt->Lock);

    if (CompleteRequest == TRUE)
    {
        ProtocolRequestComplete5(pAdapt,
                         (PVOID)&pAdapt->Request5,
                         NDIS_STATUS_FAILURE );

    }
    if (ReturnPackets == TRUE)
    {
        for (i = 0; i < NumberOfPackets; i++)
        {
            MiniportReturnPacket5(pAdapt, PacketArray[i]);
        }
    }

	//从全局绑定网卡表中移除给网卡
    NdisAcquireSpinLock(&GlobalLock);
    for (ppCursor = &g_AdaptList; *ppCursor != NULL; ppCursor = &(*ppCursor)->Next)
    {
        if (*ppCursor == pAdapt)
        {
            *ppCursor = pAdapt->Next;
            break;
        }
    }
    NdisReleaseSpinLock(&GlobalLock);


    if (pAdapt->MiniportInitPending == TRUE)
    {

        LocalStatus = NdisIMCancelInitializeDeviceInstance(
                        DriverHandle,
                        &pAdapt->UpperBindingDevice);

        if (LocalStatus == NDIS_STATUS_SUCCESS)
        {
            pAdapt->MiniportInitPending = FALSE;
            ASSERT(pAdapt->MiniportHandle == NULL);
        }
        else
        {
            NdisWaitEvent(&pAdapt->MiniportInitEvent, 0);
            ASSERT(pAdapt->MiniportInitPending == FALSE);
        }

    }
    
    if (pAdapt->MiniportHandle != NULL)
    {
        *Status = NdisIMDeInitializeDeviceInstance(pAdapt->MiniportHandle);

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            *Status = NDIS_STATUS_FAILURE;
        }
    }
    else
    {

        if(pAdapt->BindingHandle != NULL)
        {

            NdisResetEvent(&pAdapt->Event);

            NdisCloseAdapter(Status, pAdapt->BindingHandle);

            if(*Status == NDIS_STATUS_PENDING)
            {
                 NdisWaitEvent(&pAdapt->Event, 0);
                 *Status = pAdapt->Status;
            }
            pAdapt->BindingHandle = NULL;
        }
        else
        {
            *Status = NDIS_STATUS_FAILURE;
            ASSERT(0);
        }

        MiniportFreeAllPacketPools5(pAdapt);
        NdisFreeSpinLock(&pAdapt->Lock);
        NdisFreeMemory(pAdapt, 0, 0);
    }

}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolUnloadProtocol5(
    VOID
	)
{
    NDIS_STATUS Status;

    if (ProtHandle != NULL)
    {
        NdisDeregisterProtocol(&Status, ProtHandle);
        ProtHandle = NULL;
    }

}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolCloseAdapterComplete5(
    IN    NDIS_HANDLE            ProtocolBindingContext,
    IN    NDIS_STATUS            Status
    )
{
    PADAPTER      pAdapt =(PADAPTER)ProtocolBindingContext;

    pAdapt->Status = Status;
    NdisSetEvent(&pAdapt->Event);
}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolResetComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_STATUS            Status
    )
{

    UNREFERENCED_PARAMETER(ProtocolBindingContext);
    UNREFERENCED_PARAMETER(Status);

    ASSERT(0);
}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolRequestComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PVOID				   Request,//PNDIS_REQUEST50        NdisRequest,
    IN  NDIS_STATUS            Status
    )
{
    PADAPTER        pAdapt = (PADAPTER)ProtocolBindingContext;
	PNDIS_REQUEST50 NdisRequest = (PNDIS_REQUEST50)Request;
    NDIS_OID        Oid = pAdapt->Request5.DATA.SET_INFORMATION.Oid ;

    ASSERT(pAdapt->OutstandingRequests == TRUE);

    pAdapt->OutstandingRequests = FALSE;


    switch (NdisRequest->RequestType)
    {
      case NdisRequestQueryInformation:

        ASSERT(Oid != OID_PNP_QUERY_POWER);

        if ((Oid == OID_PNP_CAPABILITIES) && (Status == NDIS_STATUS_SUCCESS))
        {
            MiniportQueryPNPCapabilities5(pAdapt, &Status);
        }
        *pAdapt->BytesReadOrWritten = NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
        *pAdapt->BytesNeeded = NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

        if ((Oid == OID_GEN_MAC_OPTIONS) && (Status == NDIS_STATUS_SUCCESS))
        {
            *(PULONG)NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer &= ~NDIS_MAC_OPTION_NO_LOOPBACK;
        }

        NdisMQueryInformationComplete(pAdapt->MiniportHandle,
                                      Status);
        break;

      case NdisRequestSetInformation:

        ASSERT( Oid != OID_PNP_SET_POWER);

        *pAdapt->BytesReadOrWritten = NdisRequest->DATA.SET_INFORMATION.BytesRead;
        *pAdapt->BytesNeeded = NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
        NdisMSetInformationComplete(pAdapt->MiniportHandle,
                                    Status);
        break;

      default:
        ASSERT(0);
        break;
    }
    
}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolStatus5(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  UINT                StatusBufferSize
    )
{
    PADAPTER      pAdapt = (PADAPTER)ProtocolBindingContext;

    if ((pAdapt->MiniportHandle != NULL)  &&
        (pAdapt->MiniportDeviceState == NdisDeviceStateD0) &&
        (pAdapt->PTDeviceState == NdisDeviceStateD0))    
    {
        if ((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || 
            (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT))
        {
            
            pAdapt->LastIndicatedStatus = GeneralStatus;
        }
        NdisMIndicateStatus(pAdapt->MiniportHandle,
                            GeneralStatus,
                            StatusBuffer,
                            StatusBufferSize);
    }
    else
    {
        if ((pAdapt->MiniportHandle != NULL) && 
        ((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || 
            (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT)))
        {
            pAdapt->LatestUnIndicateStatus = GeneralStatus;
        }
    }
    
}

/*************************************************************************************


*************************************************************************************/
VOID
ProtocolStatusComplete5(
    IN NDIS_HANDLE            ProtocolBindingContext
    )
{
    PADAPTER      pAdapt = (PADAPTER)ProtocolBindingContext;

    if ((pAdapt->MiniportHandle != NULL)  &&
        (pAdapt->MiniportDeviceState == NdisDeviceStateD0) &&
        (pAdapt->PTDeviceState == NdisDeviceStateD0))    
    {
        NdisMIndicateStatusComplete(pAdapt->MiniportHandle);
    }

}

/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
ProtocolPNPHandler5(
    IN NDIS_HANDLE        ProtocolBindingContext,
    IN PNET_PNP_EVENT     pNetPnPEvent
    )
{
    PADAPTER            pAdapt  =(PADAPTER)ProtocolBindingContext;
    NDIS_STATUS       Status  = NDIS_STATUS_SUCCESS;


    switch (pNetPnPEvent->NetEvent)
    {
        case NetEventSetPower:
            Status = ProtocolPnPNetEventSetPower5(pAdapt, pNetPnPEvent);
            break;

         case NetEventReconfigure:
            Status = ProtocolPnPNetEventReconfigure5(pAdapt, pNetPnPEvent);
            break;

         default:
            if (pAdapt && pAdapt->MiniportHandle)
            {
                Status = MyNdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
            }

            break;
    }

    return Status;
}

/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
ProtocolPnPNetEventReconfigure5(
    IN PADAPTER            pAdapt,
    IN PNET_PNP_EVENT      pNetPnPEvent
    )
{
    NDIS_STATUS    ReconfigStatus = NDIS_STATUS_SUCCESS;
    NDIS_STATUS    ReturnStatus = NDIS_STATUS_SUCCESS;

    do
    {
        if (pAdapt == NULL)
        {
            NdisReEnumerateProtocolBindings (ProtHandle);        
            break;
        }

        if (pAdapt->MiniportHandle)
        {
            ReturnStatus = MyNdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
        }

        ReconfigStatus = NDIS_STATUS_SUCCESS;

    } while(FALSE);

    ReconfigStatus = ReturnStatus;

    return ReconfigStatus;
}

/*************************************************************************************


*************************************************************************************/
NDIS_STATUS
ProtocolPnPNetEventSetPower5(
    IN PADAPTER            pAdapt,
    IN PNET_PNP_EVENT      pNetPnPEvent
    )
{
    PNDIS_DEVICE_POWER_STATE       pDeviceState  =(PNDIS_DEVICE_POWER_STATE)(pNetPnPEvent->Buffer);
    NDIS_DEVICE_POWER_STATE        PrevDeviceState = pAdapt->PTDeviceState;  
    NDIS_STATUS                    Status;
    NDIS_STATUS                    ReturnStatus;
    ULONG                          PendingIoCount = 0;

    ReturnStatus = NDIS_STATUS_SUCCESS;

    NdisAcquireSpinLock(&pAdapt->Lock);
    pAdapt->PTDeviceState = *pDeviceState;

    if (pAdapt->PTDeviceState > NdisDeviceStateD0)
    {

        if (PrevDeviceState == NdisDeviceStateD0)
        {
            pAdapt->StandingBy = TRUE;
        }

        NdisReleaseSpinLock(&pAdapt->Lock);


        if (pAdapt->MiniportHandle != NULL)
        {
            ReturnStatus = MyNdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
        }

        while (pAdapt->OutstandingSends != 0)
        {
            NdisMSleep(2);
        }

        while (pAdapt->OutstandingRequests == TRUE)
        {

            NdisMSleep(2);
        }

        NdisAcquireSpinLock(&pAdapt->Lock);
        if (pAdapt->QueuedRequest)
        {
            pAdapt->QueuedRequest = FALSE;
            NdisReleaseSpinLock(&pAdapt->Lock);
            ProtocolRequestComplete5(pAdapt, (PVOID)&pAdapt->Request5, NDIS_STATUS_FAILURE);
        }
        else
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
        }
        
        ASSERT(NdisPacketPoolUsage(pAdapt->SendPacketPoolHandle) == 0);
        ASSERT(pAdapt->OutstandingRequests == FALSE);
    }
    else
    {

        if (PrevDeviceState > NdisDeviceStateD0)
        {
            pAdapt->StandingBy = FALSE;
        }

        if (pAdapt->QueuedRequest == TRUE)
        {
            pAdapt->QueuedRequest = FALSE;
        
            pAdapt->OutstandingRequests = TRUE;
            NdisReleaseSpinLock(&pAdapt->Lock);

            NdisRequest(&Status,
                        pAdapt->BindingHandle,
                        (PVOID)&pAdapt->Request5);

            if (Status != NDIS_STATUS_PENDING)
            {
                ProtocolRequestComplete5(pAdapt,
                                  (PVOID)&pAdapt->Request5,
                                  Status);
            }
        }
        else
        {
            NdisReleaseSpinLock(&pAdapt->Lock);
        }


        if (pAdapt->MiniportHandle)
        {
            ReturnStatus = MyNdisIMNotifyPnPEvent(pAdapt->MiniportHandle, pNetPnPEvent);
        }

    }

    return ReturnStatus;
}


