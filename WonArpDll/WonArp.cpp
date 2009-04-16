// WonArpNet.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include "ArpMgr.h"
#include "Config.h"
#include "NetInstall.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

CArpMgr		*pmgr	= NULL;
CConfig	    *pconfig = NULL;
CNetInstall *pInstall = NULL;

BOOL WINAPI GetClassObject(char* szClassName,LPVOID* pObject)
{

	if(_strnicmp(szClassName,ARPMGR_CLASS,strlen(ARPMGR_CLASS)) == 0)
	{
		if(!pmgr)
		{
			pmgr = new CArpMgr;
		}
		*pObject = pmgr;

	}
	else if(_strnicmp(szClassName,ARP_CONFIG_CLASS,strlen(ARP_CONFIG_CLASS)) == 0)
	{
		if(!pconfig)
		{
			pconfig = new CConfig;
		}
		*pObject = pconfig;
	}
	else if(_strnicmp(szClassName,NETINSTALL_CLASS,strlen(NETINSTALL_CLASS)) == 0)
	{
		if(!pInstall)
		{
			pInstall = new CNetInstall;
		}
		*pObject = pInstall;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

