// TrayIcon.cpp: implementation of the CTrayIcon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrayIcon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrayIcon::CTrayIcon()
{
	Initialize();
}

CTrayIcon::~CTrayIcon()
{
	RemoveIcon();
}

CTrayIcon::CTrayIcon(CWnd* pParent,UINT uCallbackMessage,LPCTSTR szToolTip,HICON icon,UINT nID)
{
	Initialize();
	Create(pParent,uCallbackMessage,szToolTip,icon,nID);
}

BOOL CTrayIcon::Create(CWnd* pParent,UINT uCallbackMessage,LPCTSTR szToolTip,HICON icon,UINT nID)
{
	m_bEnabled = (GetVersion()&0xff) >= 4;
	if(!m_bEnabled)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT(uCallbackMessage >= WM_USER );

	ASSERT(_tcslen(szToolTip) <= 64);

	m_tnd.cbSize = sizeof(NOTIFYICONDATA);
	m_tnd.hWnd   = pParent->GetSafeHwnd();
	m_tnd.uID    = nID;
	m_tnd.hIcon  = icon;
	m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
	m_tnd.uCallbackMessage = uCallbackMessage;
	_tcscpy_s(m_tnd.szTip,szToolTip);

	m_bEnabled = Shell_NotifyIcon(NIM_ADD,&m_tnd);
	ASSERT(m_bEnabled);

	return m_bEnabled;

}
BOOL CTrayIcon::SetIcon(UINT nIDResource)
{

	HICON hIcon = AfxGetApp()->LoadIcon(nIDResource);
	if(!m_bEnabled) return FALSE;

	m_tnd.uFlags = NIF_ICON;
	m_tnd.hIcon  = hIcon;

	return Shell_NotifyIcon(NIM_MODIFY,&m_tnd);

}
void CTrayIcon::HideIcon()
{
	if(m_bEnabled && !m_bHidden)
	{
		m_tnd.uFlags = NIF_ICON;
		Shell_NotifyIcon(NIM_DELETE,&m_tnd);
		m_bHidden = TRUE;
	}
}
void CTrayIcon::RemoveIcon()
{
	if(!m_bEnabled) return;

	m_tnd.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE,&m_tnd);

	m_bEnabled = FALSE;

}

LRESULT CTrayIcon::OnTrayNotification(WPARAM wParam,LPARAM lParam)
{
	//在这里执行消息映射
	CWnd* pTarget = AfxGetMainWnd();
	//
	//	处理鼠标右键单击事件
	//
	if(LOWORD(lParam) == WM_LBUTTONDBLCLK)
	{
		pTarget->SetForegroundWindow();
		pTarget->ShowWindow(SW_SHOW);
	}

	return 1;
}	

void CTrayIcon::Initialize()
{
	memset(&m_tnd,0,sizeof(m_tnd));
	m_bEnabled = FALSE;
	m_bHidden  = FALSE;
	m_DefaultMenuItemID = 0;
	m_DefaultMenuItemByPos = TRUE;

}