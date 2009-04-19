
#include "../WonArpDll/WonArp.h"
#include "../WonArpDll/ArpMgr.h"
#include "../WonArpDll/Config.h"
#include "../WonArpDll/NetInstall.h"
#include "resource.h"
#include "StateDlg.h"
#include "LogDlg.h"
#include "SetupDlg.h"

extern CArpMgr	    *g_ArpMgr;
extern CConfig		*g_Config;
extern CNetInstall  *g_Install;
extern CStateDlg	g_state_dlg;
extern CSetupDlg	g_setup_dlg;
extern CLogDlg		g_log_dlg;

BOOL InitObjects();