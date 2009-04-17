// WonArpMgr.cpp: implementation of the CArpMgr class.
//
//////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include "ArpMgr.h"
#include "../WonArp5/Share.h"
#include "../WonArp6/Share60.h"

#pragma comment(lib,"ws2_32.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

typedef DWORD (WINAPI *GETADAPTERSINFO)(PIP_ADAPTER_INFO pAdapterInfo,PULONG pOutBufLen);
typedef DWORD (WINAPI *SENDARP)(IPAddr DestIP,IPAddr SrcIP,PULONG pMacAddr,PULONG PhyAddrLen);
typedef DWORD (WINAPI *DELETEIPNETENTRY)(PMIB_IPNETROW pArpEntry);
typedef DWORD (WINAPI *GETIPNETTABLE)(PMIB_IPNETTABLE pIpNetTable,PULONG pdwSize,BOOL bOrder);
typedef DWORD (WINAPI *NOTIFYADDRCHANGE)(PHANDLE Handle,LPOVERLAPPED overlapped);

GETADAPTERSINFO		g_GetAdaptersInfo;
SENDARP				g_SendARP;
DELETEIPNETENTRY	g_DeleteIpNetEntry;
GETIPNETTABLE		g_GetIpNetTable;
HMODULE				g_Iphlpapi;
NOTIFYADDRCHANGE	g_NotifyAddrChange;

CArpMgr::CArpMgr()
{
	g_Iphlpapi = LoadLibrary(_T("IPHLPAPI.DLL"));

	if(g_Iphlpapi)
	{
		g_GetAdaptersInfo = (GETADAPTERSINFO)GetProcAddress(g_Iphlpapi,_T("GetAdaptersInfo"));
		g_SendARP		  = (SENDARP)GetProcAddress(g_Iphlpapi,_T("SendARP"));
		g_GetIpNetTable   = (GETIPNETTABLE)GetProcAddress(g_Iphlpapi,_T("GetIpNetTable"));
		g_DeleteIpNetEntry= (DELETEIPNETENTRY)GetProcAddress(g_Iphlpapi,_T("DeleteIpNetEntry"));
		g_NotifyAddrChange= (NOTIFYADDRCHANGE)GetProcAddress(g_Iphlpapi,_T("NotifyAddrChange"));
	}
	else
	{
		TerminateProcess(GetCurrentProcess(),-1);
	}

	m_ObjRefCount = 0;

	m_h_sys			= INVALID_HANDLE_VALUE;
	m_h_event		= NULL;
	m_Share_Mem     = NULL;
	m_Items			= NULL;
	m_h_addr_change_event = NULL;

	//检测Windows版本做不同的IO操作
	IsVistaOrLater = FALSE;
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	
	GetVersionEx((OSVERSIONINFO*)&osvi);
	switch (osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
			{
				if(osvi.dwMajorVersion == 6)
				{
					IsVistaOrLater = TRUE;
				}
			}
			break;
		default:
			break;
	}

}

CArpMgr::~CArpMgr()
{

}

STDMETHODIMP CArpMgr::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;

	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
	{
	   *ppReturn = this;
	}

	if(*ppReturn)
	{
	   (*(LPUNKNOWN*)ppReturn)->AddRef();
	   return S_OK;
	}

	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CArpMgr::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CArpMgr::Release()
{
	return  --m_ObjRefCount;
}

STDMETHODIMP CArpMgr::Open()
{
	m_h_sys = CreateFile(TEXT("\\\\.\\WonArp"),0,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0); // GENERIC_READ|GENERIC_WRITE , FILE_ATTRIBUTE_SYSTEM
	if(m_h_sys == INVALID_HANDLE_VALUE)
	{
		return S_FALSE;
	}
	// 设置共享内存
	if(!SetShareMem())
	{
		Close();
		return S_FALSE;
	}
	return S_OK;
}

BOOL CArpMgr::SetShareMem()
{
	DWORD BytesReturned = 0;
	BOOL bRet = FALSE;

	if(m_h_sys == INVALID_HANDLE_VALUE) return FALSE;

	if(m_Share_Mem)
	{
		VirtualUnlock(m_Share_Mem,sizeof(ARPFW_SHARE_MEM));
		VirtualFree( m_Share_Mem,sizeof(ARPFW_SHARE_MEM),MEM_DECOMMIT);
		m_Share_Mem  = NULL;
	}

	m_h_addr_change_event = CreateEvent(NULL,TRUE,FALSE,NULL);

	if(m_h_addr_change_event == NULL)
	{
		Close();
		return FALSE;
	}

	m_h_event = CreateEvent(NULL,TRUE,FALSE,NULL);

	if(m_h_event == NULL)
	{
		Close();
		return FALSE;
	}

	m_Share_Mem = (ARPFW_SHARE_MEM*)VirtualAlloc(NULL,sizeof(ARPFW_SHARE_MEM),MEM_COMMIT,PAGE_READWRITE);

	if(m_Share_Mem)
	{
		memset(m_Share_Mem,0,sizeof(ARPFW_SHARE_MEM));

		if(VirtualLock(m_Share_Mem,sizeof(ARPFW_SHARE_MEM)))
		{
			m_Share_Mem->NotifyEvent = m_h_event;

			if(IsVistaOrLater)
			{
				bRet = DeviceIoControl( m_h_sys,IOCTL_SET_SHARE_MEMORY_60,
										NULL,0,m_Share_Mem,sizeof(ARPFW_SHARE_MEM),
										&BytesReturned,NULL);
			}
			else
			{
				bRet = DeviceIoControl( m_h_sys,IOCTL_SET_SHARE_MEMORY,
										NULL,0,m_Share_Mem,sizeof(ARPFW_SHARE_MEM),
										&BytesReturned,NULL);
			}
		}
		else
		{
			bRet = FALSE;
		}
	}

	if(bRet)
	{
		return TRUE;
	}
	
	if(m_Share_Mem)
	{
		VirtualFree( m_Share_Mem,sizeof(ARPFW_SHARE_MEM),MEM_DECOMMIT);
		m_Share_Mem  = NULL;
	
	}

	if(m_h_event != NULL)
	{
		CloseHandle(m_h_event);
		m_h_event = NULL;
	}

	return FALSE;

}

// 获取报文使用阻塞式操作
STDMETHODIMP CArpMgr::GetNotifyPacket(NOTIFY_PACKET* pNotifyPacket)
{
	DWORD dwRet;
	if(m_h_sys == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	dwRet = WaitForSingleObject(m_h_event,INFINITE);
	ResetEvent(m_h_event);

	if(dwRet == WAIT_OBJECT_0)
	{
		memcpy(pNotifyPacket,&m_Share_Mem->NotifyPacket,sizeof(NOTIFY_PACKET));
		return S_OK;
	}
	
	return S_FALSE;
}


STDMETHODIMP CArpMgr::Close()
{
	if(m_h_addr_change_event != NULL)
	{
		CloseHandle(m_h_addr_change_event);
		m_h_addr_change_event = NULL;
	}

	if(m_h_event != NULL)
	{
		CloseHandle(m_h_event);
		m_h_event = NULL;
	}

	//关闭设备
	if(m_h_sys != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_h_sys);
		m_h_sys = INVALID_HANDLE_VALUE;
	}

	if(m_Share_Mem)
	{
		VirtualUnlock(m_Share_Mem,sizeof(ARPFW_SHARE_MEM));
		VirtualFree( m_Share_Mem,sizeof(ARPFW_SHARE_MEM),MEM_DECOMMIT);
		m_Share_Mem  = NULL;
	}

	return S_OK;
}

STDMETHODIMP CArpMgr::IsOpened()
{
	if(m_h_sys != INVALID_HANDLE_VALUE)
	{
		return S_OK;
	}
	return S_FALSE;
}

VOID CArpMgr::ReleaseAllIPInfo()
{
	PNETCARD_ITEM		Items = m_Items;
	PNETCARD_ITEM		Temp  = NULL;
	while(Items)
	{
		Temp = Items;
		Items = Items->Next;
		free(Temp);
	}
	m_Items  = NULL;
}

char Empty_Gateway[4*4]	= 
				{'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

STDMETHODIMP CArpMgr::GetIPInfo(PNETCARD_ITEM* Items)
 //ULONG* IPAddress,UCHAR* IPAddressMac,ULONG* GatewayAddress)
{
	DWORD				dwRet				= ERROR_SUCCESS;
	ULONG				ulAdapterInfoSize	= 0;
	PIP_ADAPTER_INFO	pAdapterInfo		= NULL;
	PIP_ADAPTER_INFO	pAdapter			= NULL;

	IP_ADDR_STRING*		pIPAddressList		= NULL;
	IP_ADDR_STRING*		pGatewayList		= NULL;

	char				NetcardDeviceName[MAX_PATH];
	
	ULONG				IPAddress			= 0;
	ULONG				GatewayAddress		= 0;
	PNETCARD_ITEM		pNetcardItem		= NULL;

	ReleaseAllIPInfo();

	if (g_GetAdaptersInfo( pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW) 
	{
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulAdapterInfoSize);

		dwRet = g_GetAdaptersInfo(pAdapterInfo,&ulAdapterInfoSize);

		if(dwRet == ERROR_SUCCESS)
		{
			pAdapter = pAdapterInfo;

			while(pAdapter)
			{
				//跳过广域网接口和本机环回接口和其他未知接口
				if(	pAdapter->Type == MIB_IF_TYPE_LOOPBACK ||
					pAdapter->Type == MIB_IF_TYPE_OTHER    )
				{
					pAdapter = pAdapter->Next;
					continue;
				}

				pIPAddressList = &pAdapter->IpAddressList;
				pGatewayList   = &pAdapter->GatewayList;

				while(pIPAddressList)
				{
					IPAddress = inet_addr((const char*)pIPAddressList->IpAddress.String);

					if(IPAddress)
					{
						pNetcardItem = (PNETCARD_ITEM)malloc(sizeof(NETCARD_ITEM));
						if(pNetcardItem)
						{
							memset(pNetcardItem,0,sizeof(NETCARD_ITEM));

							// 记录网卡的本地IP信息
							lstrcpy(NetcardDeviceName,"\\DEVICE\\");
							lstrcat(NetcardDeviceName,pAdapter->AdapterName);
							MultiByteToWideChar(CP_ACP,0,NetcardDeviceName,-1,pNetcardItem->NetcardDeviceName,MAX_PATH);
							
							pNetcardItem->AdapterIndex = pAdapter->Index;
							pNetcardItem->AdapterType  = pAdapter->Type;

							//本地IP地址
							memcpy(pNetcardItem->IPAddress,&IPAddress,4);
							//本地Mac地址
							memcpy(pNetcardItem->IPAddressMac,pAdapter->Address,6);

							if( pGatewayList												  &&
								memcmp(Empty_Gateway,pGatewayList->IpAddress.String,4*4) != 0 )
							{
								GatewayAddress = inet_addr((const char*)pGatewayList->IpAddress.String);
								//网关地址
								memcpy(pNetcardItem->GatewayIP,&GatewayAddress,4);
							}
							
							pNetcardItem->Next  = m_Items;
							m_Items			    = pNetcardItem;
						}
					}

					pIPAddressList = pIPAddressList->Next;
					pGatewayList   = pGatewayList->Next;
				}
				pAdapter = pAdapter->Next;
			}

		}
	}

	if(m_Items != NULL)
	{
		* Items = m_Items;
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CArpMgr::SafeIPAddress(ULONG IPAddress,UCHAR* MacAddress)
{
	BOOL		bRet = FALSE;
	IP_ITEM		Item;
	ULONG		ulBufferSize = sizeof(IP_ITEM);

	if(m_h_sys != INVALID_HANDLE_VALUE)
	{
		memcpy(&Item.IPAddress,&IPAddress,4);
		memcpy(&Item.MacAddress,MacAddress,6);
		Item.Gateway	= FALSE;
		Item.WanAddress = FALSE;
		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(	m_h_sys,IOCTL_ADDIPINFO_60,
									(LPVOID)&Item,ulBufferSize,NULL,
									0,&ulBufferSize,NULL);
		}
		else
		{
			bRet = DeviceIoControl(	m_h_sys,IOCTL_ADDIPINFO,
									(LPVOID)&Item,ulBufferSize,NULL,
									0,&ulBufferSize,NULL);
		}
	}

	return bRet == TRUE ? S_OK : S_FALSE;
}

STDMETHODIMP CArpMgr::SafeWANAddress(ULONG WanAddress,UCHAR* WanAddressMac)
{
	BOOL		bRet = FALSE;
	IP_ITEM		Item;
	ULONG		ulBufferSize = sizeof(IP_ITEM);

	if(m_h_sys != INVALID_HANDLE_VALUE)
	{
		memcpy(&Item.IPAddress,&WanAddress,4);
		if(WanAddressMac != NULL)
		{
			memcpy(&Item.MacAddress,WanAddressMac,6);
		}
		Item.Gateway	= FALSE;
		Item.WanAddress = TRUE;

		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(	m_h_sys,IOCTL_ADDIPINFO_60,
									(LPVOID)&Item,ulBufferSize,NULL,
									0,&ulBufferSize,NULL);
		}
		else
		{
			bRet = DeviceIoControl(	m_h_sys,IOCTL_ADDIPINFO,
									(LPVOID)&Item,ulBufferSize,NULL,
									0,&ulBufferSize,NULL);
		}

	}

	return bRet == TRUE ? S_OK : S_FALSE;

}

STDMETHODIMP CArpMgr::SafeGatewayAddress(ULONG IPAddress,UCHAR* MacAddress)
{
	BOOL		bRet = FALSE;
	IP_ITEM		Item;
	ULONG		ulBufferSize = sizeof(IP_ITEM);

	if(m_h_sys != INVALID_HANDLE_VALUE)
	{
		memcpy(&Item.IPAddress,&IPAddress,4);
		memcpy(&Item.MacAddress,MacAddress,6);
		Item.Gateway	= TRUE;
		Item.WanAddress = FALSE;

		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(	m_h_sys,IOCTL_ADDIPINFO_60,
									(LPVOID)&Item,ulBufferSize,NULL,
									0,&ulBufferSize,NULL);
		}
		else
		{
			bRet = DeviceIoControl(	m_h_sys,IOCTL_ADDIPINFO,
									(LPVOID)&Item,ulBufferSize,NULL,
									0,&ulBufferSize,NULL);
		}

	}

	return bRet == TRUE ? S_OK : S_FALSE;

}

STDMETHODIMP CArpMgr::RemoveSafeLanInfo()
{
	BOOL	bRet			= FALSE;
	ULONG	ulBytesReturned = 0;

	if(IsVistaOrLater)
	{
		bRet = DeviceIoControl(m_h_sys,IOCTL_REMOVE_ALL_LAN_INFO_60,NULL,0,
			NULL,0,&ulBytesReturned,NULL);
	}
	else
	{
		bRet = DeviceIoControl(m_h_sys,IOCTL_REMOVE_ALL_LAN_INFO,NULL,0,
			NULL,0,&ulBytesReturned,NULL);
	}

	return bRet == TRUE ? S_OK : S_FALSE;
}

STDMETHODIMP CArpMgr::RemoveSafeGatewayInfo()
{
	BOOL	bRet			= FALSE;
	ULONG	ulBytesReturned = 0;

	if(IsVistaOrLater)
	{
		bRet = DeviceIoControl(m_h_sys,IOCTL_REMOVE_ALL_GATEWAY_INFO_60,NULL,0,
			NULL,0,&ulBytesReturned,NULL);
	}
	else
	{
		bRet = DeviceIoControl(m_h_sys,IOCTL_REMOVE_ALL_GATEWAY_INFO,NULL,0,
			NULL,0,&ulBytesReturned,NULL);
	}

	return bRet == TRUE ? S_OK : S_FALSE;
}

STDMETHODIMP CArpMgr::RemoveSafeWanInfo()
{


	return S_OK;
}

STDMETHODIMP CArpMgr::EnableGatewaySafe(IN BOOL bSafe)
{

	BOOL	bRet;
	ULONG	ulBytesReturned = 0;

	if(bSafe)
	{
		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_ENABLE_GATEWAY_CHECK_60,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
		else
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_ENABLE_GATEWAY_CHECK,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
	}
	else
	{
		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_DISABLE_GATEWAY_CHECK_60,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
		else
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_DISABLE_GATEWAY_CHECK,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
	}

	return bRet == TRUE ? S_OK : S_FALSE;

}


STDMETHODIMP  CArpMgr::EnableLanIPSafe(IN BOOL bSafe)
{
	BOOL	bRet;
	ULONG	ulBytesReturned = 0;

	if(bSafe)
	{
		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_ENABLE_SAMEIP_CHECK_60,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
		else
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_ENABLE_SAMEIP_CHECK,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
	}
	else
	{
		if(IsVistaOrLater)
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_DISABLE_SAMEIP_CHECK_60,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
		else
		{
			bRet = DeviceIoControl(m_h_sys,IOCTL_DISABLE_SAMEIP_CHECK,NULL,0,
							NULL,0,&ulBytesReturned,NULL);
		}
	}

	return bRet == TRUE ? S_OK : S_FALSE;

}


STDMETHODIMP CArpMgr::WaitIPAddressChange()
{
	if( g_NotifyAddrChange(&m_h_addr_change_event,NULL) == NO_ERROR ) //&&
//		g_NotifyAddrChange(&m_h_addr_change_event,NULL) == NO_ERROR )
	{
		return TRUE;
	}
	return S_FALSE;
}


HRESULT CArpMgr::RemoveARPEntryByIP(unsigned long IPAddress)
{
	HRESULT				Result				= S_FALSE;
	DWORD				dwError				= NO_ERROR;
	//当前机器的ARP表数据
	PMIB_IPNETTABLE		pIpNetTable			= NULL;
	ULONG				ulIpNetTableSize	= 0;
	MIB_IPNETROW		DeleteNetTable;

	// 通过 GetIpNetTable 函数获取 IP - Mac 的对应关系
	// 如果找不到网关的Mac地址，则我们需要发送 Arp 查询包
	g_GetIpNetTable(pIpNetTable,&ulIpNetTableSize,FALSE);
	if(ulIpNetTableSize > 0)
	{
		pIpNetTable = (PMIB_IPNETTABLE) malloc(ulIpNetTableSize);

		if(g_GetIpNetTable(pIpNetTable,&ulIpNetTableSize,FALSE) == NO_ERROR)
		{
			ulIpNetTableSize = pIpNetTable->dwNumEntries;

			for(ULONG i = 0 ; i< ulIpNetTableSize; i++)
			{
				if( pIpNetTable->table[i].dwAddr == IPAddress )
				{

					DeleteNetTable.dwIndex = pIpNetTable->table[i].dwIndex;
					DeleteNetTable.dwAddr  = pIpNetTable->table[i].dwAddr;

					dwError = g_DeleteIpNetEntry(&DeleteNetTable);

					Result = S_OK;
				}
			}
		}
	}

	if(pIpNetTable)
	{
		free(pIpNetTable);
		pIpNetTable = NULL;
	}

	return Result;
}

STDMETHODIMP CArpMgr::GetGatewayMac(ULONG GatewayAddress,UCHAR* GatewayMac,BOOL bAutoRegconize)
{
	DWORD		dwRet;
	HRESULT		hr ;
	BOOL		bRet = FALSE;
	ULONG		pulMac[2];
	ULONG		ulLen = 6;
	
	ULONG		pulMac1[2];
	ULONG		ulLen1 = 6;

	ULONG		pulMac2[2];
	ULONG		ulLen2 = 6;
	
	if(GatewayAddress == 0)
	{
		return S_FALSE;
	}

	if(!bAutoRegconize)
	{
		memset (pulMac,0xff,sizeof(pulMac));
		RemoveARPEntryByIP(GatewayAddress);
		hr	=	g_SendARP(GatewayAddress, 0, pulMac, &ulLen);
		if(hr != NO_ERROR)
		{
			return bRet == TRUE ? S_OK : S_FALSE;
		}

		memset (pulMac1,0xff,sizeof(pulMac1));
		RemoveARPEntryByIP(GatewayAddress);
		hr	=	g_SendARP(GatewayAddress, 0, pulMac1, &ulLen1);
		if(hr != NO_ERROR)
		{
			return bRet == TRUE ? S_OK : S_FALSE;
		}

		memset (pulMac2,0xff,sizeof(pulMac2));
		RemoveARPEntryByIP(GatewayAddress);
		hr	=	g_SendARP(GatewayAddress, 0, pulMac2, &ulLen2);
		if(hr != NO_ERROR)
		{
			return bRet == TRUE ? S_OK : S_FALSE;
		}

		if( memcmp(pulMac,pulMac1,6) == 0 &&
			memcmp(pulMac,pulMac2,6) == 0 )
		{
			memcpy(GatewayMac,pulMac,6);
			bRet = TRUE;
		}
	}
	else // 启用内核智能网关欺骗性识别
	{
		if(StartQueryGateway(GatewayAddress))
		{
			memset (pulMac,0xff,sizeof(pulMac));

			//当用户权限不足时执行此操作会失败
			RemoveARPEntryByIP(GatewayAddress);
			hr = g_SendARP(GatewayAddress, 0, pulMac, &ulLen);

			dwRet = WaitForSingleObject(m_h_event,INFINITE);

			if( m_Share_Mem->Replay.ulQueryCount == 0  ||
				m_Share_Mem->Replay.ulItemCount  == 0  )
			{
				return bRet == TRUE ? S_OK : S_FALSE;
			}

			if(m_Share_Mem->Replay.ulItemCount == 1)
			{
				memcpy(GatewayMac,m_Share_Mem->Replay.Items[0].MacAddress,6);
				bRet = TRUE;				
			}

			// 当前网关地址正被欺骗,存在多个网关回应记录
			if(m_Share_Mem->Replay.ulItemCount > 1)
			{
				
				ULONG  ulQueryReplySameCount    = 0;
				
				ULONG  ulQueryReplyNotSameCount = 0;
				
				ULONG  iSameCountIndex			= 0;
				
				ULONG i;
				
				for(i = 0; i< m_Share_Mem->Replay.ulItemCount; i++)
				{
					if(m_Share_Mem->Replay.ulQueryCount != m_Share_Mem->Replay.Items[i].RecordCount)
					{
						ulQueryReplyNotSameCount ++;
					}
					else
					{
						ulQueryReplySameCount ++;
						iSameCountIndex = i;
					}
				}

				if(ulQueryReplySameCount == 1)
				{
					bRet = TRUE;
					memcpy(GatewayMac,m_Share_Mem->Replay.Items[iSameCountIndex].MacAddress,6);
				}
				else // 重新识别
				{
					bRet = FALSE;
				}
			}
		}

	}

	return bRet == TRUE ? S_OK : S_FALSE;
}

BOOL CArpMgr::StartQueryGateway(unsigned long	GatewayIPAddress)
{
	BOOL		bRet = FALSE;
	IP_ITEM		Item;

	ULONG		ulInputBufferSize  = sizeof(IP_ITEM);
	ULONG		ulOutputBufferSize = 0;

	memcpy(Item.IPAddress,&GatewayIPAddress,4);
	Item.Gateway = TRUE;

	if(m_h_sys == INVALID_HANDLE_VALUE) return bRet;

	memset(&m_Share_Mem->Replay,0,sizeof(REPLY_RECORD));

	if( GatewayIPAddress == 0) return bRet;

	if(IsVistaOrLater)
	{
		bRet = DeviceIoControl( m_h_sys,IOCTL_BEGIN_ARP_QUERY_60,
								&Item,ulInputBufferSize,NULL,0,&ulOutputBufferSize,NULL);
	}
	else
	{
		bRet = DeviceIoControl( m_h_sys,IOCTL_BEGIN_ARP_QUERY,
								&Item,ulInputBufferSize,NULL,0,&ulOutputBufferSize,NULL);

	}
	return bRet;
}

STDMETHODIMP CArpMgr::MapMacAddressToIPAddress(UCHAR* MacAddress,unsigned long* IPAddress)
{
	ULONG	ulItemCount = 0;

	if(!MacAddress || !m_Share_Mem) return S_FALSE;

	ulItemCount = m_Share_Mem->ulItemCount;

	if(ulItemCount == 0) return S_FALSE;

	for(ULONG i = 0; i< ulItemCount ; i++)
	{
		if(memcmp(m_Share_Mem->Items[i].MacAddress,MacAddress,6) == 0)
		{
			memcpy(IPAddress,m_Share_Mem->Items[i].IPAddress,4);
			return S_OK;
		}
	}

	return S_FALSE;
}

STDMETHODIMP CArpMgr::QueryCurrentState()
{



	return S_FALSE;
}

STDMETHODIMP CArpMgr::IsAdmin()
{

	HANDLE                   hAccessToken;
    BYTE                     *InfoBuffer;
    PTOKEN_GROUPS            ptgGroups;
    DWORD                    dwInfoBufferSize;
    PSID                     psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    UINT                     i;
    BOOL                  bRet = FALSE;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hAccessToken))
		goto cleanup;

    InfoBuffer = new BYTE[1024];
    if(!InfoBuffer)
		goto cleanup;

    bRet = GetTokenInformation(hAccessToken,
                               TokenGroups,
                               InfoBuffer,
                               1024,
                               &dwInfoBufferSize);

    CloseHandle(hAccessToken);

    if(!bRet)
       goto cleanup;

    if( !AllocateAndInitializeSid(&siaNtAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0,0,0,0,0,0,
                                 &psidAdministrators) )
       goto cleanup;

    bRet = FALSE;

    ptgGroups = (PTOKEN_GROUPS)InfoBuffer;

    for(i = 0; i < ptgGroups->GroupCount; i++)
    {
        if(EqualSid(psidAdministrators,ptgGroups->Groups[i].Sid))
        {
            bRet = TRUE;
            break;
        }
    }

    FreeSid(psidAdministrators);

cleanup:

    if(InfoBuffer)
		delete InfoBuffer;

	if(bRet)
	{
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CArpMgr::RenewGatewayMac(unsigned long GatewayIPAddress)
{
	DWORD				dwError				= NO_ERROR;
	//当前机器的ARP表数据
	PMIB_IPNETTABLE		pIpNetTable			= NULL;
	ULONG				ulIpNetTableSize	= 0;
	MIB_IPNETROW		DeleteNetTable;

	if(IsAdmin() != S_OK) return S_OK;

	// 通过 GetIpNetTable 函数获取 IP - Mac 的对应关系
	// 如果找不到网关的Mac地址，则我们需要发送 Arp 查询包
	g_GetIpNetTable(pIpNetTable,&ulIpNetTableSize,FALSE);
	if(ulIpNetTableSize > 0)
	{
		pIpNetTable = (PMIB_IPNETTABLE) malloc(ulIpNetTableSize);

		if(g_GetIpNetTable(pIpNetTable,&ulIpNetTableSize,FALSE) == NO_ERROR)
		{
			ulIpNetTableSize = pIpNetTable->dwNumEntries;

			for(ULONG i = 0 ; i< ulIpNetTableSize; i++)
			{
				DeleteNetTable.dwIndex = pIpNetTable->table[i].dwIndex;
				DeleteNetTable.dwAddr  = pIpNetTable->table[i].dwAddr;

				g_DeleteIpNetEntry(&DeleteNetTable);

			}
		}
	}

	if(pIpNetTable)
	{
		free(pIpNetTable);
		pIpNetTable = NULL;
	}

	return S_OK;

}