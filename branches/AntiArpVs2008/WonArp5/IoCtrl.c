
#include "precomp.h"
#pragma hdrstop

// 自动义数据
ULONG			g_ulCurOpenCount	= 0;

/****************************************************************************




*****************************************************************************/

NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpStack;

	IrpStack = IoGetCurrentIrpStackLocation(Irp);

#ifdef _DEBUG
	IrpStack->FileObject->FsContext = NULL;
#endif

	g_ulCurOpenCount ++;

	if(g_ulCurOpenCount == 1)
	{

	}

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information=0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


/****************************************************************************




*****************************************************************************/

NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PADAPTER			pAdapt   = NULL;

	PIO_STACK_LOCATION  IrpStack = IoGetCurrentIrpStackLocation(Irp);

#ifdef _DEBUG
    pAdapt = IrpStack->FileObject->FsContext;

	if( pAdapt  && pAdapt->FileObject == IrpStack->FileObject)
	{

		KdPrint(("Close: FileObject %p\n",IrpStack->FileObject));
		
		IrpStack->FileObject->FsContext = NULL;

		NdisAcquireSpinLock(&pAdapt->Lock);

		pAdapt->FileObject = NULL;

		NdisReleaseSpinLock(&pAdapt->Lock);
	}
#endif

	g_ulCurOpenCount --;

	if(IsUserShareMemoryProcess(&g_Share_User_Mem) == STATUS_SUCCESS)
	{
		UnInitArpCheck();

		NdisAcquireSpinLock(&GlobalLock);
		g_ArpFw_ShareMem = NULL;
		NdisReleaseSpinLock(&GlobalLock);

		UninitUserShareMemory(&g_Share_User_Mem);
	}

	UninitUserShareEvent(&g_NotifyEvent);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


/****************************************************************************

	Read 用于从当前攻击列表中读取 ARP 攻击报文记录

*****************************************************************************/

NTSTATUS DispatchRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0;


    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

/****************************************************************************




*****************************************************************************/

NTSTATUS DispatchWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			  Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION    IrpStack  = IoGetCurrentIrpStackLocation(Irp);

#ifdef _DEBUG
	ULONG				  ulInputLen ;
	PADAPTER			  pAdapt  = IrpStack->FileObject->FsContext;

	PSEND_RSVD			  SendRsvd;

	PUCHAR				  PacketMem = NULL;
	PNDIS_BUFFER		  Buffer = NULL;
	PNDIS_PACKET		  Packet = NULL;

	NDIS_STATUS			  NdisStatus;

	if(pAdapt)
	{
		ulInputLen = IrpStack->Parameters.Write.Length;

		// Irp->AssociatedIrp.SystemBuffer
		
		if(ulInputLen >= sizeof(ARP_PACKET) && ulInputLen < 1500 )
		{
			
			do
			{
				NdisAcquireSpinLock(&pAdapt->Lock);
				//
				// If the below miniport is going to low power state, stop sending down any packet.
				//
				if (pAdapt->PTDeviceState > NdisDeviceStateD0)
				{
					NdisReleaseSpinLock(&pAdapt->Lock);
					Status = STATUS_DEVICE_POWER_FAILURE ;
					break;
				}
				pAdapt->OutstandingSends++;
				NdisReleaseSpinLock(&pAdapt->Lock);

				PacketMem = 
					ExAllocatePoolWithTag(NonPagedPool,IrpStack->Parameters.Write.Length,TAG);

				if(PacketMem)
				{
					RtlCopyMemory(PacketMem,Irp->AssociatedIrp.SystemBuffer,
												IrpStack->Parameters.Write.Length);

					NdisAllocateBuffer(&NdisStatus,&Buffer,pAdapt->SendBufferPool,
										PacketMem,IrpStack->Parameters.Write.Length);
					
					if(NdisStatus != NDIS_STATUS_SUCCESS)
					{
						ExFreePool(PacketMem);
						PacketMem = NULL;
						break;
					}

					NdisAllocatePacket(&NdisStatus,&Packet,pAdapt->SendPacketPoolHandle);

					if(NdisStatus != NDIS_STATUS_SUCCESS)
					{
						NdisFreeBuffer(Buffer);
						Buffer = NULL;
						ExFreePool(PacketMem);
						PacketMem = NULL;
						break;
					}

					NdisChainBufferAtFront(Packet, Buffer);

					SendRsvd = (PSEND_RSVD)(Packet->ProtocolReserved);
					SendRsvd->OriginalPkt = NULL;
					SendRsvd->PacketMem   = PacketMem;

					NdisSend(&NdisStatus,pAdapt->BindingHandle,Packet);

					if (NdisStatus != NDIS_STATUS_PENDING)
					{
						MyFreeNdisSendPacket(Packet);
						ExFreePool(PacketMem);
						
						ADAPT_DECR_PENDING_SENDS(pAdapt);
					}

					Status = STATUS_SUCCESS;

				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;

				}
			}
			while(FALSE);


		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		Status = STATUS_DEVICE_NOT_READY;
	}
#endif

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information=0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


/****************************************************************************




*****************************************************************************/

NTSTATUS
DispatchIoCtrl(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
{
    NTSTATUS              Status = STATUS_NOT_SUPPORTED;
    PIO_STACK_LOCATION    IrpStack  = IoGetCurrentIrpStackLocation(Irp);
	PADAPTER			  pAdapt   = NULL;

	IP_ITEM*			  pItem = NULL;
	GATEWAY_ITEM*		  pGateway	= NULL;
	LAN_ITEM*			  pLan		= NULL;
	WAN_ITEM*			  pWan		= NULL;

	IP_ITEM*			  pGatewayARPReplyPackets = NULL;
	BOOLEAN				  bTimerCancel = FALSE;

	REPLY_RECORD*		  Reply_Record = NULL;

	PARPFW_SHARE_MEM	  pShareMemory = NULL;

#ifdef _DEBUG
	pAdapt = IrpStack->FileObject->FsContext;
#endif

    switch(IrpStack->Parameters.DeviceIoControl.IoControlCode)
    {
	case IOCTL_SET_SHARE_MEMORY: // Irp->MdlAddress
		{
			if(IrpStack->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ARPFW_SHARE_MEM))
			{
				break;
			}

			pShareMemory = (PARPFW_SHARE_MEM)MmGetSystemAddressForMdlSafe(Irp->MdlAddress,NormalPagePriority);

			if(!pShareMemory) break;

			// 共享Event未初始化
			if( pShareMemory->NotifyEvent == 0x0)
			{
				break;
			}

			Status = InitUserShareEvent(&g_NotifyEvent,pShareMemory->NotifyEvent);

			if(Status != STATUS_SUCCESS)
			{
				KdPrint((" 打开共享Event失败 "));
				break;
			}

			Status = InitUserShareMemory(&g_Share_User_Mem,Irp->MdlAddress);

			if(Status == STATUS_SUCCESS)
			{
				NdisAcquireSpinLock(&GlobalLock);
				g_ArpFw_ShareMem = (PARPFW_SHARE_MEM)g_Share_User_Mem.ShareMemory;
				NdisReleaseSpinLock(&GlobalLock);
				// PASSIVE_LEVEL
				InitArpCheck();
			}
			else
			{
				UninitUserShareEvent(&g_NotifyEvent);
			}

			break;
		}
	case IOCTL_CLEAR_SHARE_MEMORY:
		{
			if(IrpStack->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ARPFW_SHARE_MEM))
			{
				break;
			}

			if(IsUserShareMemoryProcess(&g_Share_User_Mem) == STATUS_SUCCESS)
			{
				// PASSIVE_LEVEL
				UnInitArpCheck();

				NdisAcquireSpinLock(&GlobalLock);
				g_ArpFw_ShareMem = NULL;
				NdisReleaseSpinLock(&GlobalLock);
			
				Status = UninitUserShareMemory(&g_Share_User_Mem);
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}

			break;
		}
	case IOCTL_REMOVE_ALL_GATEWAY_INFO: // 移除网关信息
		{	
			RemoveAllGatewayInfo();
			Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_REMOVE_ALL_LAN_INFO:     //移除所有本机地址信息
		{
			RemoveAllLanInfo();
			Status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_ADDIPINFO:			// 增加网关信息 , IP_ITEM
		{
			// Irp->AssociatedIrp.SystemBuffer

			if(IrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(IP_ITEM))
			{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}

			pItem = (IP_ITEM*)Irp->AssociatedIrp.SystemBuffer;

			if(pItem)
			{
				if(pItem->WanAddress)		// 广域网地址
				{
					pWan = ExAllocatePoolWithTag(NonPagedPool,sizeof(WAN_ITEM),TAG);

					if(!pWan)
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
					
					RtlCopyMemory(pWan->IPAddress,pItem->IPAddress,4);
					RtlCopyMemory(pWan->MacAddress,pItem->MacAddress,6);

					NdisAcquireSpinLock(&GlobalLock);
					
					pWan->Before		= NULL;
					pWan->Next			= g_Wan_List;
					if(g_Wan_List)
					{
						g_Wan_List->Before	= pWan;
					}
					g_Wan_List			= pWan;

					NdisReleaseSpinLock(&GlobalLock);

				}
				else if(pItem->Gateway )	// 网关地址
				{
					pGateway = ExAllocatePoolWithTag(NonPagedPool,
													 sizeof(GATEWAY_ITEM),
													 TAG);
					if(!pGateway)
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
					
					RtlCopyMemory(pGateway->IPAddress,pItem->IPAddress,4);
					RtlCopyMemory(pGateway->MacAddress,pItem->MacAddress,6);

					NdisAcquireSpinLock(&GlobalLock);
					
					pGateway->Before		= NULL;
					pGateway->Next			= g_Gateway_List;
					if(g_Gateway_List)
					{
						g_Gateway_List->Before	= pGateway;
					}
					g_Gateway_List			= pGateway;

					NdisReleaseSpinLock(&GlobalLock);

				}
				else						// 局域网地址
				{
					pLan = ExAllocatePoolWithTag(NonPagedPool,
													 sizeof(LAN_ITEM),
													 TAG);
					if(!pLan)
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
					
					RtlCopyMemory(pLan->IPAddress,pItem->IPAddress,4);
					RtlCopyMemory(pLan->MacAddress,pItem->MacAddress,6);

					NdisAcquireSpinLock(&GlobalLock);
					
					pLan->Before		= NULL;
					pLan->Next			= g_Lan_List;
					if(g_Lan_List)
					{
						g_Lan_List->Before	= pLan;
					}
					g_Lan_List			= pLan;

					NdisReleaseSpinLock(&GlobalLock);
				}
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}

			break;
		}
	case IOCTL_BEGIN_ARP_QUERY:	// 开始由IP地址查询对应的MAC地址
		{
			if(IrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(IP_ITEM))
			{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
			
			pItem = (IP_ITEM*)Irp->AssociatedIrp.SystemBuffer;
			
			NdisAcquireSpinLock(&GlobalLock);
			
			if(g_bRecord_ARP_Reply || !g_Reply_Record)
			{
				Status = STATUS_DEVICE_BUSY;
				NdisReleaseSpinLock(&GlobalLock);
				break;
			}
			NdisReleaseSpinLock(&GlobalLock);
			
			if(pItem)
			{
				NdisMoveMemory(g_Want_ARP_Reply_IP,pItem->IPAddress,4);

				g_Reply_Record->ulItemCount = 0;

				g_Reply_Record->ulQueryCount = 0;
				
				g_bRecord_ARP_Reply  = TRUE;
				
				IrpStack->Parameters.DeviceIoControl.OutputBufferLength = 0;
				
				Status = STATUS_SUCCESS;
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}


			break;
		}
	case IOCTL_ENABLE_GATEWAY_CHECK:
		{
			NdisAcquireSpinLock(&GlobalLock);
			g_EnableGatewayCheck = TRUE;
			NdisReleaseSpinLock(&GlobalLock);
			break;
		}
	case IOCTL_DISABLE_GATEWAY_CHECK:
		{
			NdisAcquireSpinLock(&GlobalLock);
			g_EnableGatewayCheck = FALSE;
			NdisReleaseSpinLock(&GlobalLock);
			break;
		}
	case IOCTL_ENABLE_SAMEIP_CHECK:
		{
			NdisAcquireSpinLock(&GlobalLock);
			g_EnableSameIPCheck = TRUE;
			NdisReleaseSpinLock(&GlobalLock);
			break;
		}
	case IOCTL_DISABLE_SAMEIP_CHECK:
		{
			NdisAcquireSpinLock(&GlobalLock);
			g_EnableSameIPCheck = FALSE;
			NdisReleaseSpinLock(&GlobalLock);
			break;
		}
#ifdef _DEBUG
	case IOCTL_OPENADAPTER:
		{
            if (pAdapt != NULL)
            {
                KdPrint(("IoControl: OPEN_DEVICE: FileObj %p already associated with open %p\n",
									IrpStack->FileObject, pAdapt));

                Status = STATUS_DEVICE_BUSY;
                break;
            }

			_wcsupr(Irp->AssociatedIrp.SystemBuffer);
			
			NdisAcquireSpinLock(&GlobalLock);

			pAdapt = g_AdaptList;

			while(pAdapt)
			{
				if( pAdapt->DeviceName.Length == IrpStack->Parameters.DeviceIoControl.InputBufferLength				&&
					!memcmp(pAdapt->DeviceName.Buffer,Irp->AssociatedIrp.SystemBuffer,pAdapt->DeviceName.Length)  )
				{
					break;
				}
				pAdapt  = pAdapt->Next;
			}

			NdisReleaseSpinLock(&GlobalLock);

			if(pAdapt)
			{
				KdPrint(("Open: FileObject %p\n", IrpStack->FileObject));

				NdisAcquireSpinLock(&pAdapt->Lock);

				IrpStack->FileObject->FsContext  = pAdapt;
				pAdapt->FileObject				 = IrpStack->FileObject;

				NdisReleaseSpinLock(&pAdapt->Lock);

				Status = STATUS_SUCCESS;
			}
			else
			{
				Status = STATUS_OBJECT_NAME_NOT_FOUND;
			}

			break;
		}
	case IOCTL_CLOSEADAPTER:
		{
			IrpStack->FileObject->FsContext = NULL;

			if(pAdapt)
			{
				NdisAcquireSpinLock(&pAdapt->Lock);

				pAdapt->FileObject = NULL;

				NdisReleaseSpinLock(&pAdapt->Lock);
			}

			break;
		}
#endif
    default:
        break;
    }

    Irp->IoStatus.Status = Status;
   
    if(Status == STATUS_SUCCESS)
        Irp->IoStatus.Information = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    else
        Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;

} 
