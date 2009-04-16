
#include "precomp.h"
#pragma hdrstop

                
NTSTATUS
FilterDeviceIoControl(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PIRP                  Irp
    )
{
    PIO_STACK_LOCATION          IrpSp;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PFILTER_DEVICE_EXTENSION    FilterDeviceExtension;
    PUCHAR                      InputBuffer;
    PUCHAR                      OutputBuffer;
    ULONG                       InputBufferLength, OutputBufferLength;
    PLIST_ENTRY                 Link;
    PUCHAR                      pInfo;
    ULONG                       InfoLength = 0;
    PMS_FILTER                  pFilter = NULL;

	// ++
	IP_ITEM*					pItem = NULL;
	GATEWAY_ITEM*				pGateway	= NULL;
	LAN_ITEM*					pLan		= NULL;
	WAN_ITEM*					pWan		= NULL;

	IP_ITEM*					pGatewayARPReplyPackets = NULL;
	BOOLEAN						bTimerCancel = FALSE;

	REPLY_RECORD*				Reply_Record = NULL;

	PARPFW_SHARE_MEM			pShareMemory = NULL;

	// --

    UNREFERENCED_PARAMETER(DeviceObject);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    if (IrpSp->FileObject == NULL)
    {
        return(STATUS_UNSUCCESSFUL);
    }

    FilterDeviceExtension = (PFILTER_DEVICE_EXTENSION)NdisGetDeviceReservedExtension(DeviceObject);

    ASSERT(FilterDeviceExtension->Signature == 'FTDR');
    
    Irp->IoStatus.Information = 0;

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
	case IOCTL_SET_SHARE_MEMORY_60: // Irp->MdlAddress
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ARPFW_SHARE_MEM))
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
	case IOCTL_CLEAR_SHARE_MEMORY_60:
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ARPFW_SHARE_MEM))
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
		case IOCTL_REMOVE_ALL_GATEWAY_INFO_60: // 移除网关信息
			{	
				RemoveAllGatewayInfo();
				Status = STATUS_SUCCESS;
				break;
			}
		case IOCTL_REMOVE_ALL_LAN_INFO_60:     //移除所有本机地址信息
			{
				RemoveAllLanInfo();
				Status = STATUS_SUCCESS;
				break;
			}
		case IOCTL_ADDIPINFO_60:			// 增加网关信息 , IP_ITEM
			{
				// Irp->AssociatedIrp.SystemBuffer

				if(IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(IP_ITEM))
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

		case IOCTL_BEGIN_ARP_QUERY_60:	// 开始由IP地址查询对应的MAC地址
			{
				if(IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(IP_ITEM))
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
					
					IrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
					
					Status = STATUS_SUCCESS;
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}

				break;
			}
		case IOCTL_ENABLE_GATEWAY_CHECK_60:
			{
				NdisAcquireSpinLock(&GlobalLock);
				g_EnableGatewayCheck = TRUE;
				NdisReleaseSpinLock(&GlobalLock);
				break;
			}
		case IOCTL_DISABLE_GATEWAY_CHECK_60:
			{
				NdisAcquireSpinLock(&GlobalLock);
				g_EnableGatewayCheck = FALSE;
				NdisReleaseSpinLock(&GlobalLock);
				break;
			}
		case IOCTL_ENABLE_SAMEIP_CHECK_60:
			{
				NdisAcquireSpinLock(&GlobalLock);
				g_EnableSameIPCheck = TRUE;
				NdisReleaseSpinLock(&GlobalLock);
				break;
			}
		case IOCTL_DISABLE_SAMEIP_CHECK_60:
			{
				NdisAcquireSpinLock(&GlobalLock);
				g_EnableSameIPCheck = FALSE;
				NdisReleaseSpinLock(&GlobalLock);
				break;
			}
        default:
            break;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = InfoLength;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
            

}
