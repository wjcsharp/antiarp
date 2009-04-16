// WonArpFWDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WonFW.h"
#include "WonFWDlg.h"

#include <Winsock2.h>
#include ".\WonFWdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWonArpFWDlg dialog

CWonArpFWDlg::CWonArpFWDlg(CWnd* pParent /*=NULL*/)
   : CNewDialog(CWonArpFWDlg::IDD, pParent), m_bAttachNow(FALSE)
{
	//{{AFX_DATA_INIT(CWonArpFWDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ARP_ENABLE);

	m_back_brush = ::CreateSolidBrush(RGB(255,255,255));

	m_tab_focus_bitmap.LoadBitmap(IDB_TABFOCUS);
	m_tab_not_focus_bitmap.LoadBitmap(IDB_TABNOTFOCUS);

	m_Notify_Thread_Work_Event = CreateEvent(NULL,TRUE,FALSE,NULL);

}

void CWonArpFWDlg::DoDataExchange(CDataExchange* pDX)
{
	CNewDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWonArpFWDlg)
	DDX_Control(pDX, IDC_CHILD_FRAME, m_child_frame);
	DDX_Control(pDX, IDB_LOG, m_tab_log);
	DDX_Control(pDX, IDB_STATE, m_tab_state);
	DDX_Control(pDX, IDB_SETUP, m_tab_setup);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWonArpFWDlg, CNewDialog)
	//{{AFX_MSG_MAP(CWonArpFWDlg)
	ON_MESSAGE(WM_TRAYICON ,OnTrayIconMessage)
	ON_BN_CLICKED(WM_SHOWSTATE, OnShowState)
	ON_BN_CLICKED(WM_SHOWLOG,   OnShowLog)
	ON_BN_CLICKED(WM_SHOWSETUP, OnShowSetup)
	ON_BN_CLICKED(WM_CLOSEFW,   OnMyExit)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDB_STATE, OnState)
	ON_BN_CLICKED(IDB_SETUP, OnSetup)
	ON_BN_CLICKED(IDB_LOG, OnLog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CWonArpFWDlg::OnShowState()
{
	OnState();
	ShowWindow(SW_SHOW);
}

void CWonArpFWDlg::OnShowSetup()
{
	OnSetup();
	ShowWindow(SW_SHOW);
}

void CWonArpFWDlg::OnShowLog()
{
	OnLog();
	ShowWindow(SW_SHOW);
}

void CWonArpFWDlg::OnMyExit()
{
	OnCancel();
}


/////////////////////////////////////////////////////////////////////////////
// CWonArpFWDlg message handlers

BOOL CWonArpFWDlg::OnInitDialog()
{
	CNewDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	InitControls();

	if(g_StartupHide)
	{
		SetTimer(AUTO_HIDE_TIME,1000,NULL);
	}

	//创建网关处理线程
	CThread::CreateThread(GatewayThread,this);

	//创建攻击包处理线程
	CThread::CreateThread(AttachPacketNotify,this);

	m_trayicon.Create(this,WM_TRAYICON,"玩火 AntiARP",m_hIcon,5000);

	SetTimer(MODIFY_SHELLICON_TIMER,2000,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CWonArpFWDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
//	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	if (nID == SC_MINIMIZE)
	{
//		CAboutDlg dlgAbout;
//		dlgAbout.DoModal();
		ShowWindow(SW_HIDE);
	}
	else
	{
		CNewDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWonArpFWDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC	dc(this);
        CRect		rect;
		CDC			dcMem;
		CBitmap		bmpBackground;

		GetClientRect(&rect);

        dcMem.CreateCompatibleDC(&dc);

        bmpBackground.LoadBitmap(IDB_BACKMAP);

        BITMAP bitMap;

        bmpBackground.GetBitmap(&bitMap);

        CBitmap *pbmpOld=dcMem.SelectObject(&bmpBackground);

        dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitMap.bmWidth,bitMap.bmHeight,SRCCOPY);
		
		dcMem.DeleteDC();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWonArpFWDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CWonArpFWDlg::OnCancel() 
{
	if(MessageBox("关闭防火墙将失去抵御ARP攻击的能力，确实要关闭吗?","玩火AntiARP",MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON2) == IDYES)
	{
		CNewDialog::OnCancel();
	}
}

HBRUSH CWonArpFWDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CNewDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: Return a different brush if the default is not desired
	return m_back_brush;
}

void CWonArpFWDlg::OnState() 
{
	m_tab_state.RedrawButton(TRUE);
	m_tab_setup.RedrawButton(FALSE);
	m_tab_log.RedrawButton(FALSE);

	g_state_dlg.SetSafeLevel();

	g_state_dlg.ShowWindow(SW_SHOW);
	g_setup_dlg.ShowWindow(SW_HIDE);
	g_log_dlg.ShowWindow(SW_HIDE);

	m_show_id = 1;
	
}

void CWonArpFWDlg::OnSetup() 
{
	m_tab_state.RedrawButton(FALSE);
	m_tab_setup.RedrawButton(TRUE);
	m_tab_log.RedrawButton(FALSE);

	g_setup_dlg.m_auto_mac.SetCheck(m_Config->AUTO_REGCOGNIZE_GATEWAY_MAC);
	g_setup_dlg.m_anti_sameip.SetCheck(m_Config->ANTISAMEIP);
	g_setup_dlg.m_anti_gateway.SetCheck(m_Config->ANTIGATEWAY);
	g_setup_dlg.m_popup_notify.SetCheck(m_Config->POPUP_NOTIFY);
	g_setup_dlg.m_send_check.SetCheck(m_Config->ANTISEND);
	g_setup_dlg.m_system_start.SetCheck(m_Config->SYSTEM_BOOT_AUTOSTART);

	g_state_dlg.ShowWindow(SW_HIDE);
	g_setup_dlg.ShowWindow(SW_SHOW);
	g_log_dlg.ShowWindow(SW_HIDE);

	m_show_id = 2;

}

void CWonArpFWDlg::OnLog() 
{
	m_tab_state.RedrawButton(FALSE);
	m_tab_setup.RedrawButton(FALSE);
	m_tab_log.RedrawButton(TRUE);

	g_state_dlg.ShowWindow(SW_HIDE);
	g_setup_dlg.ShowWindow(SW_HIDE);
	g_log_dlg.ShowWindow(SW_SHOW);
	
	m_show_id = 3;
}

void CWonArpFWDlg::InitControls()
{
	m_tab_state.LoadBitmaps(IDB_TABNOTFOCUS,IDB_TABFOCUS,IDB_TABNOTFOCUS);
	m_tab_setup.LoadBitmaps(IDB_TABNOTFOCUS,IDB_TABFOCUS,IDB_TABNOTFOCUS);
	m_tab_log.LoadBitmaps(IDB_TABNOTFOCUS,IDB_TABFOCUS,IDB_TABNOTFOCUS);

	g_state_dlg.Create(CStateDlg::IDD,this);
	g_setup_dlg.Create(CSetupDlg::IDD,this);
	g_log_dlg.Create(CLogDlg::IDD,this);

	CRect FrameRect;
	CRect ChildDlgRect;

	m_child_frame.GetWindowRect(&FrameRect);

	ScreenToClient(&FrameRect);

	ChildDlgRect.left  = FrameRect.left		+ 1;
	ChildDlgRect.right = FrameRect.right	- 1;
	ChildDlgRect.top   = FrameRect.top		+ 1;
	ChildDlgRect.bottom= FrameRect.bottom	-1;
	
	g_state_dlg.MoveWindow(&ChildDlgRect);

	g_setup_dlg.MoveWindow(&ChildDlgRect);
	
	g_log_dlg.MoveWindow(&ChildDlgRect);

	//更新当前设置状态
	g_Config->GetCurrentConfig(&m_Config);

	OnState();

	g_state_dlg.SetSafeLevel();

	g_setup_dlg.m_auto_mac.SetCheck(m_Config->AUTO_REGCOGNIZE_GATEWAY_MAC);
	g_setup_dlg.m_anti_sameip.SetCheck(m_Config->ANTISAMEIP);
	g_setup_dlg.m_anti_gateway.SetCheck(m_Config->ANTIGATEWAY);
	g_setup_dlg.m_popup_notify.SetCheck(m_Config->POPUP_NOTIFY);
	g_setup_dlg.m_send_check.SetCheck(m_Config->ANTISEND);
	g_setup_dlg.m_system_start.SetCheck(m_Config->SYSTEM_BOOT_AUTOSTART);
	
}

void CWonArpFWDlg::GatewayThread(void *pParam)
{
	CWonArpFWDlg* pThis = (CWonArpFWDlg*)pParam;
	pThis->GatewayWork();
}

void CWonArpFWDlg::GatewayWork()
{
	TCHAR   strText[256];

	UCHAR	GatewayMac[6]	= { 0,0,0,0,0,0};
	HRESULT hResult			= S_FALSE;
	int     iItem			= 0;
	PNETCARD_ITEM			Items;
	ULONG	GatewayAddress  = 0;
	ULONG	LanIPAddress    = 0;
	ULONG   WanIPAddress;
	BOOL	bHaveGateway	= FALSE;

ReWork:
	if(g_ArpMgr)
	{
		hResult = g_ArpMgr->GetIPInfo(&Items);
		if(hResult != S_OK)
		{
			Sleep(1000);
			goto ReWork;
		}
		while(Items)
		{
			if( Items->AdapterType == MIB_IF_TYPE_OTHER		||
				Items->AdapterType == MIB_IF_TYPE_LOOPBACK	)
			{
				Items = Items->Next;
				continue;
			}

			if( Items->AdapterType == MIB_IF_TYPE_PPP ||
				Items->AdapterType == MIB_IF_TYPE_SLIP )
			{
				if(m_Config->ANTISAMEIP )
				{
					memcpy(&WanIPAddress,Items->IPAddress,4);
					if(WanIPAddress != 0)
						g_ArpMgr->SafeWANAddress(WanIPAddress,Items->IPAddressMac);
				}

				Items = Items->Next;
				continue;
			}
			
			//设置界面显示
			sprintf(strText,"%d.%d.%d.%d",
				Items->IPAddress[0],Items->IPAddress[1],Items->IPAddress[2],Items->IPAddress[3]);

			iItem = g_state_dlg.m_local_address.InsertItem(g_state_dlg.m_local_address.GetItemCount() + 1,strText);
			
			if(iItem != -1)
			{
				sprintf(strText,_T("%02X-%02X-%02X-%02X-%02X-%02X"),
						Items->IPAddressMac[0],Items->IPAddressMac[1],
						Items->IPAddressMac[2],Items->IPAddressMac[3],
						Items->IPAddressMac[4],Items->IPAddressMac[5]);
				g_state_dlg.m_local_address.SetItemText(iItem,1,strText);
			}

			memcpy(&GatewayAddress,Items->GatewayIP,4);

			if(GatewayAddress)
			{
				sprintf(strText,"%d.%d.%d.%d",Items->GatewayIP[0],Items->GatewayIP[1],Items->GatewayIP[2],Items->GatewayIP[3]);
				iItem = g_state_dlg.m_gateway_address.InsertItem(
							g_state_dlg.m_gateway_address.GetItemCount() + 1,strText);
				if(iItem != -1)
				{
					g_state_dlg.m_gateway_address.SetItemText(iItem,1,_T("正在检测..."));
				}

				//开始自动检测网关的Mac地址
				if(memcmp(Items->GatewayIP,Items->IPAddress,4) == 0)
				{
					memcpy(GatewayMac,Items->IPAddressMac,6);
					sprintf(strText,_T("%02X-%02X-%02X-%02X-%02X-%02X"),
							Items->IPAddressMac[0],Items->IPAddressMac[1],Items->IPAddressMac[2],
							Items->IPAddressMac[3],Items->IPAddressMac[4],Items->IPAddressMac[5]);
					if(iItem != -1)
					{
						g_state_dlg.m_gateway_address.SetItemText(iItem,1,strText);
					}

					bHaveGateway = TRUE;
				}
				else
				{
					if(m_Config->AUTO_REGCOGNIZE_GATEWAY_MAC)
					{
						hResult = g_ArpMgr->GetGatewayMac(GatewayAddress,GatewayMac,TRUE);
						if(hResult != S_OK)
						{
							Sleep(2000);
							g_state_dlg.m_gateway_address.DeleteAllItems();
							g_state_dlg.m_local_address.DeleteAllItems();

							//删除当前驱动中的保护信息
							g_ArpMgr->RemoveSafeGatewayInfo();
							g_ArpMgr->RemoveSafeLanInfo();
							g_ArpMgr->RemoveSafeWanInfo();
							
							goto ReWork;

						}

						g_ArpMgr->RenewGatewayMac(GatewayAddress);

						sprintf(strText,_T("%02X-%02X-%02X-%02X-%02X-%02X"),GatewayMac[0],GatewayMac[1],GatewayMac[2],GatewayMac[3],GatewayMac[4],GatewayMac[5]);
						if(iItem != -1)
						{
							g_state_dlg.m_gateway_address.SetItemText(iItem,1,strText);
						}
					}
					else
					{
						hResult = g_ArpMgr->GetGatewayMac(GatewayAddress,GatewayMac,FALSE);
						if(hResult != S_OK)
						{
							if(iItem != -1)
							{
								g_state_dlg.m_gateway_address.SetItemText(iItem,1,TEXT("网关地址不可用"));
							}
							Sleep(2000);
							g_state_dlg.m_gateway_address.DeleteAllItems();
							g_state_dlg.m_local_address.DeleteAllItems();

							//删除当前驱动中的保护信息
							g_ArpMgr->RemoveSafeGatewayInfo();
							g_ArpMgr->RemoveSafeLanInfo();
							g_ArpMgr->RemoveSafeWanInfo();
							
							goto ReWork;
						}

						g_ArpMgr->RenewGatewayMac(GatewayAddress);

						sprintf(strText,_T("%02X-%02X-%02X-%02X-%02X-%02X"),GatewayMac[0],GatewayMac[1],GatewayMac[2],GatewayMac[3],GatewayMac[4],GatewayMac[5]);
						if(iItem != -1)
						{
							g_state_dlg.m_gateway_address.SetItemText(iItem,1,strText);
						}
					}

					bHaveGateway = TRUE;
				}
			}
	
			//局域网

			if(m_Config->ANTISAMEIP)
			{
				memcpy(&LanIPAddress,Items->IPAddress,4);
				g_ArpMgr->SafeIPAddress(LanIPAddress,Items->IPAddressMac);
			}
			
			if(m_Config->ANTIGATEWAY && GatewayAddress)
			{
				g_ArpMgr->SafeGatewayAddress(GatewayAddress,GatewayMac);
			}

			Items = Items->Next;
		}
		
		if(!bHaveGateway)
		{
			Sleep(1000);
			g_state_dlg.m_gateway_address.DeleteAllItems();
			g_state_dlg.m_local_address.DeleteAllItems();
			
			//删除当前驱动中的保护信息
			g_ArpMgr->RemoveSafeGatewayInfo();
			g_ArpMgr->RemoveSafeLanInfo();
			g_ArpMgr->RemoveSafeWanInfo();

			bHaveGateway = FALSE;
			
			goto ReWork;
			
		}

		if(m_Config->ANTIGATEWAY)
			g_ArpMgr->EnableGatewaySafe(TRUE);
		if(m_Config->ANTISAMEIP)
			g_ArpMgr->EnableLanIPSafe(TRUE);

		//启动攻击包处理线程
		SetEvent(m_Notify_Thread_Work_Event);

		// 等待IP地址修改消息
		g_ArpMgr->WaitIPAddressChange();

		g_state_dlg.m_gateway_address.DeleteAllItems();
		g_state_dlg.m_local_address.DeleteAllItems();
		//删除当前驱动中的保护信息
		g_ArpMgr->RemoveSafeGatewayInfo();
		g_ArpMgr->RemoveSafeLanInfo();
		g_ArpMgr->RemoveSafeWanInfo();

		goto ReWork;

	}
	else
	{
		Sleep(1000);
		InitObjects();
		goto ReWork;
	}

}

void CWonArpFWDlg::AttachPacketNotify(void* pParam)
{
	CWonArpFWDlg* pThis = (CWonArpFWDlg*)pParam;
	pThis->NotifyPacketWork();
}

void CWonArpFWDlg::NotifyPacketWork()
{
	NOTIFY_PACKET	NotifyPacket;
	SYSTEMTIME		SysTime;
	TCHAR			szTime[MAX_PATH];
	int				iItem = -1;
	ULONG			IPAddress;

	BOOL			bKnownIP;
	TCHAR			szAttachType[MAX_PATH];
	TCHAR			szSrcAddr[MAX_PATH];

	WaitForSingleObject(this->m_Notify_Thread_Work_Event,INFINITE);
	
	while(true)
	{
		if( g_ArpMgr->GetNotifyPacket(&NotifyPacket)  == S_OK)
		{
			GetLocalTime(&SysTime);

			wsprintf(szTime,"%04d-%02d-%02d %02d:%02d:%02d",
							SysTime.wYear,SysTime.wMonth,SysTime.wDay,
							SysTime.wHour,SysTime.wMinute,SysTime.wSecond);
			
			if( g_ArpMgr->MapMacAddressToIPAddress(
				NotifyPacket.ArpPacket.SrcAddr,&IPAddress) == S_OK)
			{
				bKnownIP = TRUE;
			}
			else
			{
				bKnownIP = FALSE;
			}
			
			switch(NotifyPacket.AttachType)
			{
			case GATEWAY_ARP_QUERY_ATTACH:
			case GATEWAY_ARP_REPLY_ATTACH:

				m_bAttachNow = TRUE;

				//将状态栏图标改变为红色
				m_trayicon.SetIcon(IDI_ARP_DISABLE);

				_tcscpy(szAttachType,TEXT("网关欺骗"));

				wsprintf(szSrcAddr,TEXT("%02X-%02X-%02X-%02X-%02X-%02X"),
					NotifyPacket.ArpPacket.SrcAddr[0],
					NotifyPacket.ArpPacket.SrcAddr[1],
					NotifyPacket.ArpPacket.SrcAddr[2],
					NotifyPacket.ArpPacket.SrcAddr[3],
					NotifyPacket.ArpPacket.SrcAddr[4],
					NotifyPacket.ArpPacket.SrcAddr[5]);
				
				g_log_dlg.InsertLog(szTime,szAttachType,IPAddress,szSrcAddr,1,FALSE,bKnownIP);
				
				break;
			case LAN_SAMEIP_ATTACH:
			case WAN_SAMEIP_ATTACH:

				m_bAttachNow = TRUE;

				//将状态栏图标改变为红色
				m_trayicon.SetIcon(IDI_ARP_DISABLE);

				_tcscpy(szAttachType,TEXT("IP冲突"));
				
				wsprintf(szSrcAddr,TEXT("%02X-%02X-%02X-%02X-%02X-%02X"),
					NotifyPacket.ArpPacket.SrcAddr[0],
					NotifyPacket.ArpPacket.SrcAddr[1],
					NotifyPacket.ArpPacket.SrcAddr[2],
					NotifyPacket.ArpPacket.SrcAddr[3],
					NotifyPacket.ArpPacket.SrcAddr[4],
					NotifyPacket.ArpPacket.SrcAddr[5]);
							
				g_log_dlg.InsertLog(szTime,szAttachType,IPAddress,szSrcAddr,1,FALSE,bKnownIP);
				
				break;
			case WRONG_PROTOCOL_ATTACH:

				m_bAttachNow = TRUE;

				//将状态栏图标改变为红色
				m_trayicon.SetIcon(IDI_ARP_DISABLE);

				_tcscpy(szAttachType,TEXT("错误协议"));
				
				wsprintf(szSrcAddr,TEXT("%02X-%02X-%02X-%02X-%02X-%02X"),
					NotifyPacket.ArpPacket.SrcAddr[0],
					NotifyPacket.ArpPacket.SrcAddr[1],
					NotifyPacket.ArpPacket.SrcAddr[2],
					NotifyPacket.ArpPacket.SrcAddr[3],
					NotifyPacket.ArpPacket.SrcAddr[4],
					NotifyPacket.ArpPacket.SrcAddr[5]);
				
				g_log_dlg.InsertLog(szTime,szAttachType,IPAddress,szSrcAddr,1,FALSE,bKnownIP);
								
				break;
			default:
				break;
			}
		}
	}

}


void CWonArpFWDlg::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case AUTO_HIDE_TIME:
		{
			//自动隐藏到状态栏
			ShowWindow(SW_HIDE);
			KillTimer(nIDEvent);
			break;
		}
	case MODIFY_SHELLICON_TIMER:
		{
			if(m_bAttachNow)
			{
				m_trayicon.SetIcon(IDI_ARP_DISABLE);
			}
			else
			{
				m_trayicon.SetIcon(IDI_ARP_ENABLE);
			}
			
		}
	default:
		break;
	}

}


LRESULT CWonArpFWDlg::OnTrayIconMessage(WPARAM wParam, LPARAM lParam)
{
	if( LOWORD(lParam) == WM_RBUTTONUP )
	{
		if(!IsWindowVisible())
		{
			CNewMenu menu;
			menu.CreatePopupMenu();
			menu.InsertMenu(0, MF_BYPOSITION ,WM_SHOWSTATE,_T("显示监控状态(&S)"));
			menu.InsertMenu(1, MF_BYPOSITION|MF_SEPARATOR);
			menu.InsertMenu(2, MF_BYPOSITION ,WM_SHOWSETUP,_T("显示综合设置(&T)"));
			menu.InsertMenu(3, MF_BYPOSITION|MF_SEPARATOR);
			menu.InsertMenu(4, MF_BYPOSITION ,WM_SHOWLOG,_T("显示监控日志(&L)"));
			menu.InsertMenu(5, MF_BYPOSITION|MF_SEPARATOR);
			menu.InsertMenu(6, MF_BYPOSITION ,WM_CLOSEFW,_T("关闭防火墙(&X)"));

			if(m_show_id == 1)
			{
				menu.CheckMenuItem(0,MF_CHECKED|MF_BYPOSITION);
				menu.CheckMenuItem(2,MF_UNCHECKED|MF_BYPOSITION);
				menu.CheckMenuItem(4,MF_UNCHECKED|MF_BYPOSITION);
			}
			else if(m_show_id == 2)
			{
				menu.CheckMenuItem(0,MF_UNCHECKED|MF_BYPOSITION);
				menu.CheckMenuItem(2,MF_CHECKED|MF_BYPOSITION);
				menu.CheckMenuItem(4,MF_UNCHECKED|MF_BYPOSITION);
			}
			else
			{
				menu.CheckMenuItem(0,MF_UNCHECKED|MF_BYPOSITION);
				menu.CheckMenuItem(2,MF_UNCHECKED|MF_BYPOSITION);
				menu.CheckMenuItem(4,MF_CHECKED|MF_BYPOSITION);
			}
		
			CPoint pos;
			GetCursorPos(&pos);
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, this);
		}
	
	}
	else if(LOWORD(lParam) == WM_LBUTTONUP)
	{
		if(!IsWindowVisible())
		{
			ShowWindow(SW_SHOW);
		}
	}
	return 1;
}
