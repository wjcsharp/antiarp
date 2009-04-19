// WonArpFW.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WonFW.h"
#include "WonFWDlg.h"
#include "Shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL					g_StartupHide = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CWonArpFWApp

BEGIN_MESSAGE_MAP(CWonArpFWApp, CWinApp)
	//{{AFX_MSG_MAP(CWonArpFWApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWonArpFWApp construction

CWonArpFWApp::CWonArpFWApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWonArpFWApp object

CWonArpFWApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWonArpFWApp initialization

BOOL CWonArpFWApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	//检测Windows版本做不同的IO操作
	BOOL  IsVista = FALSE;
	
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	
	GetVersionEx((OSVERSIONINFO*)&osvi);
	switch (osvi.dwPlatformId)
	{
				case VER_PLATFORM_WIN32_NT:
					{
						if( osvi.dwMajorVersion >= 6 )
						{
							IsVista = TRUE;
						}

					}
					break;
				default:
					break;
	}

	//加载网络和配置Dll库
	if(InitObjects() == FALSE)
	{
		MessageBox(NULL,TEXT("程序初始化失败"),TEXT("玩火 AntiArp"),MB_OK | MB_ICONERROR);
		return FALSE;
	}

	char		   szMiniprotInf[MAX_PATH];
	char		   szProtocolInf6[MAX_PATH];
	char		   szProtocolInf5[MAX_PATH];
	char		   szCurrentPath[MAX_PATH];
	char		   szPathDestMini[MAX_PATH];
	char		   szClearFile[MAX_PATH];

	PGLOBAL_CONFIG CurrentConfig;

	g_Config->GetCurrentConfig(&CurrentConfig);

	LPCWSTR pwszCmdLine = NULL;

	LPWSTR * argv = NULL;
	int		  argc = 0;

	pwszCmdLine = GetCommandLineW();

	argv = CommandLineToArgvW(pwszCmdLine,&argc);

	GetCurrentDirectoryA(MAX_PATH,szCurrentPath);

	GetWindowsDirectory(szPathDestMini,MAX_PATH);		
	lstrcat(szPathDestMini,"\\inf\\");
	lstrcat(szPathDestMini,"MPWONARP.inf");

	if(argc > 1)
	{
		// /Auto  用于自启动时自动隐藏到状态栏
		if(_wcsnicmp(argv[1],L"/Auto",wcslen(L"/Auto")) == 0)
		{
			//如果禁止开机运行则关闭本进程
			if(!CurrentConfig->SYSTEM_BOOT_AUTOSTART)
			{
				return FALSE;
			}
			g_StartupHide = TRUE;
		}
		// /DisableSign 禁用数字签名
		else if(_wcsnicmp(argv[1],L"/DisableSign",wcslen(L"/DisableSign")) == 0)
		{
			//如果禁止开机运行则关闭本进程
			g_Install->DriverSignCheck(FALSE);
			return TRUE;
		}
		// /EnableSign  启用数字签名
		else if(_wcsnicmp(argv[1],L"/EnableSign",wcslen(L"/EnableSign")) == 0)
		{
			//如果禁止开机运行则关闭本进程
			g_Install->DriverSignCheck(TRUE);
			return TRUE;
		}
		// /Install   用于安装程序
		else if(_wcsnicmp(argv[1],L"/Install",wcslen(L"/Install")) == 0)
		{
			lstrcpy(szMiniprotInf,szCurrentPath);
			lstrcat(szMiniprotInf,"\\MPWONARP.inf");

			if(!IsVista) // 2k/xp/2003
			{
				CopyFile(szMiniprotInf,szPathDestMini,FALSE);
				lstrcpy(szProtocolInf5,szCurrentPath);
				lstrcat(szProtocolInf5,"\\PTWONARP.inf");
				g_Install->InstallNetServiceDriver(szProtocolInf5);
			}
			else // Vista
			{
				lstrcpy(szProtocolInf6,szCurrentPath);
				lstrcat(szProtocolInf6,"\\WonArp6.inf");
				g_Install->InstallNetServiceDriver(szProtocolInf6);
			}
			
			//Vista 驱动文件
			lstrcpy(szClearFile,szCurrentPath);
			lstrcat(szClearFile,"\\WonArp6.sys");
			DeleteFile(szClearFile);
			lstrcpy(szClearFile,szCurrentPath);
			lstrcat(szClearFile,"\\WonArp6.inf");
			DeleteFile(szClearFile);
			//Windows 2k/XP/2003 驱动文件
			lstrcpy(szClearFile,szCurrentPath);
			lstrcat(szClearFile,"\\WonArp.sys");
			DeleteFile(szClearFile);
			lstrcpy(szClearFile,szCurrentPath);
			lstrcat(szClearFile,"\\PTWONARP.inf");
			DeleteFile(szClearFile);
			lstrcpy(szClearFile,szCurrentPath);
			lstrcat(szClearFile,"\\MPWONARP.inf");
			DeleteFile(szClearFile);

			return FALSE;
		}
		// /Remove	  用于卸载程序
		else if(_wcsnicmp(argv[1],L"/Remove",wcslen(L"/Remove")) == 0)
		{
			if(!IsVista)
			{
				if(g_Install->RemoveNetServiceDriver(TEXT("ras_arpim")) == S_OK)
				{
					//将Miniprot文件从inf目录删除
					DeleteFile(szPathDestMini);
					PathRenameExtension(szPathDestMini,TEXT(".PNF"));
					DeleteFile(szPathDestMini);
				}
			}
			else
			{
				g_Install->RemoveNetServiceDriver(TEXT("MS_WonArp"));
			}
			return TRUE;
		}
	}

	GlobalFree(argv);

	HANDLE h_mutex  = CreateMutex(NULL,TRUE,TEXT("WonArpFW_Process_Mutex"));
	if(h_mutex != NULL)
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(h_mutex);
			h_mutex = NULL;
			MessageBox(NULL,TEXT("玩火 AntiArp 正在运行..."),TEXT("玩火 AntiArp"),MB_OK|MB_ICONINFORMATION);
			TerminateProcess(GetCurrentProcess(),-1);
			return FALSE;
		}
	}

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,2);

	int err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		MessageBox(NULL,TEXT("Socket 网络接口初始化失败"),TEXT("玩火 AntiArp"),MB_OK | MB_ICONERROR);
		return FALSE;
	}

#if !DBGTestGUI

	//检测驱动是否存在
	if(g_ArpMgr->Open() != S_OK)
	{
		MessageBox(NULL,TEXT("防火墙功能模块丢失，请重新安装本程序"),
			TEXT("玩火 AntiARP"),MB_OK | MB_ICONERROR);
		return FALSE;
	}

#else

	MessageBox(NULL,TEXT("防火墙功能模块未加载，仅供测试!!"),
		TEXT("界面测试!!"),MB_OK | MB_ICONERROR);

#endif

	// 网关检测成功
	CWonArpFWDlg dlg;
	m_pMainWnd = &dlg;
	
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	g_ArpMgr->Close();

	WSACleanup();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

