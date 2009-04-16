// SetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WonFW.h"
#include "SetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg dialog


CSetupDlg::CSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_back_brush = ::CreateSolidBrush(RGB(255,255,255));

}


void CSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupDlg)
	DDX_Control(pDX, IDC_SETUP_USE, m_setup_use);
	DDX_Control(pDX, IDC_ANTI_USE, m_anti_use);
	DDX_Control(pDX, IDC_IMPORT_CONFIG, m_import_config);
	DDX_Control(pDX, IDC_EXPORT_CONFIG, m_export_config);
	DDX_Control(pDX, IDC_AUTO_MAC, m_auto_mac);
	DDX_Control(pDX, IDC_SYSTEM_START, m_system_start);
	DDX_Control(pDX, IDC_SEND_CHECK, m_send_check);
	DDX_Control(pDX, IDC_POPUP_NOTIFY, m_popup_notify);
	DDX_Control(pDX, IDC_ANTI_SAMEIP, m_anti_sameip);
	DDX_Control(pDX, IDC_ANTI_GATEWAY, m_anti_gateway);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupDlg, CDialog)
	//{{AFX_MSG_MAP(CSetupDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ANTI_USE, OnAntiUse)
	ON_BN_CLICKED(IDC_SETUP_USE, OnSetupUse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg message handlers

HBRUSH CSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	return m_back_brush;
}

void CSetupDlg::OnAntiUse() 
{
	PGLOBAL_CONFIG	 CurrentConfig;
	
	g_Config->GetCurrentConfig(&CurrentConfig);
	CurrentConfig->ANTIGATEWAY = m_anti_gateway.GetCheck();
	CurrentConfig->ANTISAMEIP  = m_anti_sameip.GetCheck();
	CurrentConfig->ANTISEND    = m_send_check.GetCheck();
	g_state_dlg.SetSafeLevel();

	g_Config->SaveCurrentConfig(CurrentConfig);
}

void CSetupDlg::OnSetupUse() 
{

	PGLOBAL_CONFIG	 CurrentConfig;

	g_Config->GetCurrentConfig(&CurrentConfig);

	CurrentConfig->POPUP_NOTIFY = m_popup_notify.GetCheck();
	CurrentConfig->SYSTEM_BOOT_AUTOSTART = m_system_start.GetCheck();
	
	g_Config->SaveCurrentConfig(CurrentConfig);

}

BOOL CSetupDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//非管理员用户禁止修改配置
	if(g_ArpMgr->IsAdmin() != S_OK)
	{
		m_popup_notify.EnableWindow(FALSE);
		m_system_start.EnableWindow(FALSE);
		m_anti_gateway.EnableWindow(FALSE);
		m_anti_sameip.EnableWindow(FALSE);
		m_send_check.EnableWindow(FALSE);
		m_export_config.EnableWindow(FALSE);
		m_import_config.EnableWindow(FALSE);
		
		m_anti_use.EnableWindow(FALSE);
		m_setup_use.EnableWindow(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
