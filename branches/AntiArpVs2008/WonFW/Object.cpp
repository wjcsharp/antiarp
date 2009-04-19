
#include "stdafx.h"

#ifdef _DEBUG
#pragma comment(lib,"../WonArpDll/Debug/WonArp.lib")
#else
#pragma comment(lib,"../WonArpDll/Release/WonArp.lib")
#endif

CArpMgr		*g_ArpMgr = NULL;
CConfig		*g_Config    = NULL;
CNetInstall *g_Install   = NULL;

CStateDlg	g_state_dlg;
CSetupDlg	g_setup_dlg;
CLogDlg		g_log_dlg;

BOOL InitObjects()
{
	if( GetClassObject(ARPMGR_CLASS,(LPVOID*)&g_ArpMgr)  &&
		GetClassObject(ARP_CONFIG_CLASS,(LPVOID*)&g_Config) &&
		GetClassObject(NETINSTALL_CLASS,(LPVOID*)&g_Install)	  )
	{
		return TRUE;
	}
	
	return FALSE;
}

