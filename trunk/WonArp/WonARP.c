/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/


#include "precomp.h"
#pragma hdrstop

NTSTATUS	DispatchCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS	DispatchClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS	DispatchIoCtrl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS	DispatchRead(IN PDEVICE_OBJECT  DeviceObject,IN PIRP  Irp);
NTSTATUS	DispatchWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_INIT_FUNCTION(LoadDynamicFunction)

#pragma NDIS_PAGEABLE_FUNCTION(ProtocolUnload)

#pragma NDIS_PAGEABLE_FUNCTION(DispatchCreate)
#pragma NDIS_PAGEABLE_FUNCTION(DispatchClose)
#pragma NDIS_PAGEABLE_FUNCTION(DispatchIoCtrl)
#pragma NDIS_PAGEABLE_FUNCTION(DispatchRead)
#pragma NDIS_PAGEABLE_FUNCTION(DispatchWrite)

NDIS_HANDLE         ProtHandle		= NULL;
NDIS_HANDLE         DriverHandle	= NULL;

NDIS_MEDIUM         MediumArray[4]	= {                         
						NdisMedium802_3,    // Ethernet
                        NdisMedium802_5,    // Token-ring
                        NdisMediumFddi,     // Fddi
                        NdisMediumWan       // NDISWAN
						};

NDIS_SPIN_LOCK		GlobalLock;
PADAPTER			g_AdaptList		= NULL;
LONG				MiniportCount	= 0;
NDIS_HANDLE			NdisWrapperHandle;

PDRIVER_OBJECT		g_CurrentDriver = NULL;

PARPFW_SHARE_MEM	g_ArpFw_ShareMem = NULL;

enum _DEVICE_STATE
{
    PS_DEVICE_STATE_READY = 0,    // ready for create/delete
    PS_DEVICE_STATE_CREATING,    // create operation in progress
    PS_DEVICE_STATE_DELETING    // delete operation in progress
} ControlDeviceState = PS_DEVICE_STATE_READY;

//
// To support ioctls from user-mode:
//
NDIS_HANDLE     NdisDeviceHandle	= NULL;
PDEVICE_OBJECT  ControlDeviceObject = NULL;


//根据操作系统的NDIS版本加载不同的函数 
//操作系统版本资料
ULONG gSfOsMajorVersion;
ULONG gSfOsMinorVersion;

//NDIS版本加载不同的函数

NDISCANCELSENDPACKETS					MyNdisCancelSendPackets;
NDISGETPOOLFROMPACKET					MyNdisGetPoolFromPacket;
NDISIMGETCURRENTPACKETSTACK				MyNdisIMGetCurrentPacketStack;
NDISIMNOTIFYPNPEVENT					MyNdisIMNotifyPnPEvent;

VOID LoadDynamicFunction()
{
    NDIS_STRING   CancelSendPackets,GetPoolFromPacket,IMGetCurrentPacketStack,IMNotifyPnPEvent;

	PVOID		  pNdisBaseAddress = NULL;
	PsGetVersion(&gSfOsMajorVersion,&gSfOsMinorVersion,NULL,NULL);

	// 为了兼容没有SP补丁的Windows 2000 系统，
	// 我们使用 自定义的函数取代 NdisGetRoutineAddress函数功能

    if(	IS_NDIS51() )
	{
		RtlInitUnicodeString(&CancelSendPackets,L"NdisCancelSendPackets");
		MyNdisCancelSendPackets	= (NDISCANCELSENDPACKETS)NdisGetRoutineAddress(&CancelSendPackets);

		RtlInitUnicodeString(&GetPoolFromPacket,L"NdisGetPoolFromPacket");
		MyNdisGetPoolFromPacket	= (NDISGETPOOLFROMPACKET)NdisGetRoutineAddress(&GetPoolFromPacket);

		RtlInitUnicodeString(&IMGetCurrentPacketStack,L"NdisIMGetCurrentPacketStack");
		MyNdisIMGetCurrentPacketStack = (NDISIMGETCURRENTPACKETSTACK)NdisGetRoutineAddress(&IMGetCurrentPacketStack);

		RtlInitUnicodeString(&IMNotifyPnPEvent,L"NdisIMNotifyPnPEvent");
		MyNdisIMNotifyPnPEvent	= (NDISIMNOTIFYPNPEVENT)NdisGetRoutineAddress(&IMNotifyPnPEvent);
	   
	}
}

/***********************************************************************************************




************************************************************************************************/

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT        DriverObject,
    IN PUNICODE_STRING       RegistryPath
    )
{
    NDIS_STATUS                        Status;
    NDIS40_PROTOCOL_CHARACTERISTICS    PChars40;
    NDIS40_MINIPORT_CHARACTERISTICS    MChars40;
    NDIS50_PROTOCOL_CHARACTERISTICS    PChars50;
    NDIS51_MINIPORT_CHARACTERISTICS    MChars51;
    NDIS_STRING                        Name;

    Status = NDIS_STATUS_SUCCESS;

	g_ArpFw_ShareMem = NULL;

    NdisAllocateSpinLock(&GlobalLock);

	g_CurrentDriver = DriverObject;

	LoadDynamicFunction();  //识别版本并加载额外函数

//	Status = InitPacketList();

//	if( Status != STATUS_SUCCESS)
//	{
//		return Status;
//	}

    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, RegistryPath, NULL);

	if(IS_NDIS51())
	{
	    do
	    {
        	//
        	// Register the miniport with NDIS. Note that it is the miniport
        	// which was started as a driver and not the protocol. Also the miniport
        	// must be registered prior to the protocol since the protocol's BindAdapter
        	// handler can be initiated anytime and when it is, it must be ready to
        	// start driver instances.
        	//
        	NdisZeroMemory(&MChars51, sizeof(NDIS51_MINIPORT_CHARACTERISTICS));

        	MChars51.MajorNdisVersion = 5;
        	MChars51.MinorNdisVersion = 1;

        	MChars51.InitializeHandler 			= MiniportInitialize5;
        	MChars51.QueryInformationHandler 	= MiniportQueryInformation5;
        	MChars51.SetInformationHandler 		= MiniportSetInformation5;
        	MChars51.ResetHandler 				= NULL;
        	MChars51.TransferDataHandler 		= MiniportTransferData5;
        	MChars51.HaltHandler 				= MiniportHalt5;

			//NDIS51++
        	MChars51.CancelSendPacketsHandler 	= MiniportCancelSendPackets5;
        	MChars51.PnPEventNotifyHandler 		= MiniportDevicePnPEvent5;
        	MChars51.AdapterShutdownHandler 	= MiniportAdapterShutdown5;
			//--NDIS51

        	//
        	// We will disable the check for hang timeout so we do not
        	// need a check for hang handler!
        	//
        	MChars51.CheckForHangHandler 		= MiniportCheckForHang5;
        	MChars51.ReturnPacketHandler 		= MiniportReturnPacket5;

        	//
        	// Either the Send or the SendPackets handler should be specified.
        	// If SendPackets handler is specified, SendHandler is ignored
        	//
        	MChars51.SendHandler 				= NULL;
        	MChars51.SendPacketsHandler 		= MiniportSendPackets5;

        	Status = NdisIMRegisterLayeredMiniport(NdisWrapperHandle,
                                                  (PVOID)&MChars51,
                                                  sizeof(NDIS51_MINIPORT_CHARACTERISTICS),
                                                  &DriverHandle);
        	if (Status != NDIS_STATUS_SUCCESS)
        	{
            	break;
        	}

        	NdisMRegisterUnloadHandler(NdisWrapperHandle, ProtocolUnload);


        	//
        	// Now register the protocol.
        	//
        	NdisZeroMemory(&PChars50, sizeof(NDIS50_PROTOCOL_CHARACTERISTICS));
        	PChars50.MajorNdisVersion = 5;
        	PChars50.MinorNdisVersion = 0;

        	//
        	// Make sure the protocol-name matches the service-name
        	// (from the INF) under which this protocol is installed.
        	// This is needed to ensure that NDIS can correctly determine
        	// the binding and call us to bind to miniports below.
        	//
        	NdisInitUnicodeString(&Name, PROTOCOL_NAME);    // Protocol name
        	PChars50.Name 							= Name;
        	PChars50.OpenAdapterCompleteHandler 	= ProtocolOpenAdapterComplete5;
        	PChars50.CloseAdapterCompleteHandler 	= ProtocolCloseAdapterComplete5;
        	PChars50.SendCompleteHandler 			= ProtocolSendComplete5;
        	PChars50.TransferDataCompleteHandler 	= ProtocolTransferDataComplete5;
    
        	PChars50.ResetCompleteHandler 			= ProtocolResetComplete5;
        	PChars50.RequestCompleteHandler 		= ProtocolRequestComplete5;
        	PChars50.ReceiveHandler 				= ProtocolReceive5;
        	PChars50.ReceiveCompleteHandler 		= ProtocolReceiveComplete5;
        	PChars50.StatusHandler 					= ProtocolStatus5;
        	PChars50.StatusCompleteHandler 			= ProtocolStatusComplete5;
        	PChars50.BindAdapterHandler 			= ProtocolBindAdapter5;
        	PChars50.UnbindAdapterHandler 			= ProtocolUnbindAdapter5;
        	PChars50.UnloadHandler 					= ProtocolUnloadProtocol5;

        	PChars50.ReceivePacketHandler 			= ProtocolReceivePacket5;
        	PChars50.PnPEventHandler				= ProtocolPNPHandler5;

        	NdisRegisterProtocol(&Status,
                             &ProtHandle,
                             (PVOID)&PChars50,
                             sizeof(NDIS50_PROTOCOL_CHARACTERISTICS));

        	if (Status != NDIS_STATUS_SUCCESS)
        	{
            	NdisIMDeregisterLayeredMiniport(DriverHandle);
            	break;
        	}

        	NdisIMAssociateMiniport(DriverHandle, ProtHandle);
    	}
    	while (FALSE);

	}
	else
	{
		do
		{
			//注册 Miniport
			NdisZeroMemory(&MChars40, sizeof(NDIS40_MINIPORT_CHARACTERISTICS));

			MChars40.MajorNdisVersion = 4;
			MChars40.MinorNdisVersion = 0;

			MChars40.InitializeHandler 			= MiniportInitialize4;
			MChars40.QueryInformationHandler 	= MiniportQueryInformation4;
			MChars40.SetInformationHandler 		= MiniportSetInformation4;
			MChars40.ResetHandler 				= NULL;
			MChars40.TransferDataHandler 		= MiniportTransferData4;
			MChars40.HaltHandler 				= MiniportHalt4;

			MChars40.CheckForHangHandler 		= MiniportCheckForHang4;
			MChars40.ReturnPacketHandler 		= MiniportReturnPacket4;

			MChars40.SendHandler 				= NULL;
			MChars40.SendPacketsHandler 		= MiniportSendPackets4;

			Status = NdisIMRegisterLayeredMiniport(NdisWrapperHandle,
													  (PVOID)&MChars40,
													  sizeof(MChars40),
													  &DriverHandle);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				break;
			}

			NdisMRegisterUnloadHandler(NdisWrapperHandle, ProtocolUnload);

			//
			// Now register the protocol.
			//
			NdisZeroMemory(&PChars40, sizeof(NDIS40_PROTOCOL_CHARACTERISTICS));
			PChars40.MajorNdisVersion = 4;
			PChars40.MinorNdisVersion = 0;

			NdisInitUnicodeString(&Name, PROTOCOL_NAME);    // Protocol name
			PChars40.Name 							= Name;
			PChars40.OpenAdapterCompleteHandler 	= ProtocolOpenAdapterComplete4;
			PChars40.CloseAdapterCompleteHandler 	= ProtocolCloseAdapterComplete4;
			PChars40.SendCompleteHandler 			= ProtocolSendComplete4;
			PChars40.TransferDataCompleteHandler 	= ProtocolTransferDataComplete4;
    
			PChars40.ResetCompleteHandler 			= ProtocolResetComplete4;
			PChars40.RequestCompleteHandler 		= ProtocolRequestComplete4;
			PChars40.ReceiveHandler 				= ProtocolReceive4;
			PChars40.ReceiveCompleteHandler 		= ProtocolReceiveComplete4;
			PChars40.StatusHandler 					= ProtocolStatus4;
			PChars40.StatusCompleteHandler 			= ProtocolStatusComplete4;
			PChars40.BindAdapterHandler 			= ProtocolBindAdapter4;
			PChars40.UnbindAdapterHandler 			= ProtocolUnbindAdapter4;
			PChars40.UnloadHandler 					= ProtocolUnloadProtocol4;

			PChars40.ReceivePacketHandler 			= ProtocolReceivePacket4;
			PChars40.PnPEventHandler				= ProtocolPNPHandler4;

			NdisRegisterProtocol(&Status,
								 &ProtHandle,
								 (PVOID)&PChars40,
								 sizeof(NDIS40_PROTOCOL_CHARACTERISTICS));
			if (Status != NDIS_STATUS_SUCCESS)
			{
				NdisIMDeregisterLayeredMiniport(DriverHandle);
				break;
			}

			NdisIMAssociateMiniport(DriverHandle, ProtHandle);

		}
		while (FALSE);
	}

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
		NdisFreeSpinLock(&GlobalLock);
    }

    return(Status);
}

/***********************************************************************************************


************************************************************************************************/
VOID
ProtocolUnload(
    IN PDRIVER_OBJECT        DriverObject
    )
{
    NDIS_STATUS Status;

    if (ProtHandle != NULL)
    {
        NdisDeregisterProtocol(&Status, ProtHandle);
        ProtHandle = NULL;
    }

    NdisIMDeregisterLayeredMiniport(DriverHandle);

//	UnInitPacketList();

    NdisFreeSpinLock(&GlobalLock);

	g_ArpFw_ShareMem = NULL;

	UninitUserShareMemory(&g_Share_User_Mem);

}


/****************************************************************************




*****************************************************************************/

NDIS_STATUS
ProtocolRegisterDevice(
    VOID
    )
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING         DeviceName;
    UNICODE_STRING         DeviceLinkUnicodeString;
    PDRIVER_DISPATCH       DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    NdisAcquireSpinLock(&GlobalLock);

    ++MiniportCount;
    
    if (1 == MiniportCount)
    {
        ASSERT(ControlDeviceState != PS_DEVICE_STATE_CREATING);

        //
        // Another thread could be running PtDeregisterDevice on
        // behalf of another miniport instance. If so, wait for
        // it to exit.
        //
        while (ControlDeviceState != PS_DEVICE_STATE_READY)
        {
            NdisReleaseSpinLock(&GlobalLock);
            NdisMSleep(1);
            NdisAcquireSpinLock(&GlobalLock);
        }

        ControlDeviceState = PS_DEVICE_STATE_CREATING;

        NdisReleaseSpinLock(&GlobalLock);

        NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));

        DispatchTable[IRP_MJ_CREATE]			= DispatchCreate;
        DispatchTable[IRP_MJ_CLOSE]				= DispatchClose;
        DispatchTable[IRP_MJ_DEVICE_CONTROL]	= DispatchIoCtrl;
		DispatchTable[IRP_MJ_WRITE]				= DispatchWrite;

        NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
        NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);

        //
        // Create a device object and register our dispatch handlers
        //
        
        Status = NdisMRegisterDevice(
                    NdisWrapperHandle, 
                    &DeviceName,
                    &DeviceLinkUnicodeString,
                    &DispatchTable[0],
                    &ControlDeviceObject,
                    &NdisDeviceHandle
                    );

        NdisAcquireSpinLock(&GlobalLock);

		ControlDeviceObject->Flags |= DO_BUFFERED_IO;

        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    return (Status);
}

/****************************************************************************




*****************************************************************************/

NDIS_STATUS
ProtocolDeregisterDevice(
    VOID
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    NdisAcquireSpinLock(&GlobalLock);

    ASSERT(MiniportCount > 0);

    --MiniportCount;
    
    if (0 == MiniportCount)
    {
        //
        // All miniport instances have been halted. Deregister
        // the control device.
        //

        ASSERT(ControlDeviceState == PS_DEVICE_STATE_READY);

        //
        // Block PtRegisterDevice() while we release the control
        // device lock and deregister the device.
        // 
        ControlDeviceState = PS_DEVICE_STATE_DELETING;

        NdisReleaseSpinLock(&GlobalLock);

        if (NdisDeviceHandle != NULL)
        {
            Status = NdisMDeregisterDevice(NdisDeviceHandle);
            NdisDeviceHandle = NULL;
        }

        NdisAcquireSpinLock(&GlobalLock);
        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    return Status;
    
}

