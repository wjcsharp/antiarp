// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__61E50DE3_D66E_4EBB_A859_7BCF6CF45AE8__INCLUDED_)
#define AFX_CONFIG_H__61E50DE3_D66E_4EBB_A859_7BCF6CF45AE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define		ARP_CONFIG_CLASS		"CConfig"

typedef struct _GLOBAL_CONFIG
{
	BOOL AUTO_REGCOGNIZE_GATEWAY_MAC;
	BOOL POPUP_NOTIFY;
	BOOL SYSTEM_BOOT_AUTOSTART;
	BOOL ANTIGATEWAY;
	BOOL ANTISAMEIP;
	BOOL ANTISEND;
} GLOBAL_CONFIG,*PGLOBAL_CONFIG;

class CConfig
{
public:
	CConfig();
	virtual ~CConfig();
public:
	STDMETHOD(GetCurrentConfig)(PGLOBAL_CONFIG* Config);
	STDMETHOD(SaveCurrentConfig)(GLOBAL_CONFIG* Config);
};

#endif // !defined(AFX_CONFIG_H__61E50DE3_D66E_4EBB_A859_7BCF6CF45AE8__INCLUDED_)
