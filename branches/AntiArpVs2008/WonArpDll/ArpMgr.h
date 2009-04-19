
#ifndef _WONARPMGR_H
#define _WONARPMGR_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "../share/ShareStruct.h"

#ifndef IN
#define IN
#define OUT
#endif

#ifndef MIB_IF_TYPE_OTHER

	#define MIB_IF_TYPE_OTHER               1
	#define MIB_IF_TYPE_ETHERNET            6
	#define MIB_IF_TYPE_TOKENRING           9
	#define MIB_IF_TYPE_FDDI                15
	#define MIB_IF_TYPE_PPP                 23
	#define MIB_IF_TYPE_LOOPBACK            24
	#define MIB_IF_TYPE_SLIP                28

#endif


typedef struct _NETCARD_ITEM
{
	struct _NETCARD_ITEM*	Next;
	WCHAR					NetcardDeviceName[MAX_PATH];
	ULONG					AdapterIndex;
	ULONG					AdapterType;
	UCHAR					CurrentMacAddress[6];
	UCHAR					IPAddress[4];
	UCHAR					IPAddressMac[6];
	UCHAR					GatewayIP[4];

} NETCARD_ITEM,*PNETCARD_ITEM;

#define ARPMGR_CLASS	"CArpMgr" 

class CArpMgr 
{
public:
	CArpMgr();
	~CArpMgr();

	//IUnknown methods
    STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
    STDMETHOD_ (ULONG, AddRef) (void);
    STDMETHOD_ (ULONG, Release) (void);

public:
	STDMETHOD(Open)();
	STDMETHOD(Close)();
	STDMETHOD(IsOpened)();
	STDMETHOD(IsAdmin)();
	STDMETHOD(GetIPInfo)(PNETCARD_ITEM* Items);
	STDMETHOD(GetNotifyPacket)(NOTIFY_PACKET* pNotifyPacket);
	STDMETHOD(GetGatewayMac)(ULONG IPAddress,UCHAR* MacAddress,BOOL bAutoRegconize = FALSE);
	STDMETHOD(SafeIPAddress)(ULONG IPAddress,UCHAR* MacAddress);
	STDMETHOD(SafeWANAddress)(ULONG WanAddress,UCHAR* MacAddress);
	STDMETHOD(SafeGatewayAddress)(ULONG GatewayAddress,UCHAR* GatewayMac);
	STDMETHOD(WaitIPAddressChange)();
	STDMETHOD(QueryCurrentState)();
	STDMETHOD(EnableGatewaySafe)(IN BOOL bSafe);
	STDMETHOD(EnableLanIPSafe)(IN BOOL bSafe);
	STDMETHOD(RemoveSafeLanInfo)();
	STDMETHOD(RemoveSafeGatewayInfo)();
	STDMETHOD(RemoveSafeWanInfo)();
	STDMETHOD(RenewGatewayMac)(unsigned long GatewayIPAddress);
	STDMETHOD(MapMacAddressToIPAddress)(UCHAR* MacAddress,unsigned long* IPAddress);

private:
	BOOL    SetShareMem();
	VOID	ReleaseAllIPInfo();
	HRESULT RemoveARPEntryByIP(unsigned long IPAddress);
	BOOL	StartQueryGateway(unsigned long	GatewayIPAddress);
protected:
	DWORD					m_ObjRefCount;
	BOOL					IsVistaOrLater;
	HANDLE					m_h_sys;
	HANDLE					m_h_event;
	HANDLE					m_h_addr_change_event;
	PARPFW_SHARE_MEM		m_Share_Mem;
	PNETCARD_ITEM			m_Items;
	REPLY_RECORD*			m_Reply_Record;
};

#endif


