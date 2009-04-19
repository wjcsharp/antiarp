// State.cpp : implementation file
//

#include "stdafx.h"
#include "WonFW.h"
#include "StateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStateDlg dialog


CStateDlg::CStateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStateDlg)
	//}}AFX_DATA_INIT

	m_back_brush = ::CreateSolidBrush(RGB(255,255,255));

}

void CStateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStateDlg)
	DDX_Control(pDX, IDC_LOCAL_ADDRESS, m_local_address);
	DDX_Control(pDX, IDC_GATEWAY_ADDRESS, m_gateway_address);
	DDX_Control(pDX, IDC_LAST_TIME, m_last_time);
	DDX_Control(pDX, IDC_LAST_PC, m_last_pc);
	DDX_Control(pDX, IDC_LAST_MAC, m_last_mac);
	DDX_Control(pDX, IDC_LAST_IP, m_last_ip);
	DDX_Control(pDX, IDC_INTERNAL, m_internal);
	DDX_Control(pDX, IDC_EXTERN, m_extern);
	DDX_Control(pDX, IDC_LEVEL_SLIDER, m_level_slider);
	DDX_Control(pDX, IDC_LEVEL_LOW_TEXT, m_level_low_text);
	DDX_Control(pDX, IDC_LEVEL_HIGH_TEXT, m_level_high_text);
	DDX_Control(pDX, IDC_SAFE_LEVEL, m_safe_level);
	DDX_Control(pDX, IDC_STATE_TEXT, m_state_text);
	DDX_Control(pDX, IDC_LASTTIME_TEXT, m_lasttime_text);
	DDX_Control(pDX, IDC_LAST_PC_TEXT, m_last_pc_text);
	DDX_Control(pDX, IDC_LAST_MAC_TEXT, m_last_mac_text);
	DDX_Control(pDX, IDC_INTERNAL_TEXT, m_internal_text);
	DDX_Control(pDX, IDC_LAST_IP_TEXT, m_last_ip_text);
	DDX_Control(pDX, IDC_EXTERN_TEXT, m_extern_text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStateDlg, CDialog)
	//{{AFX_MSG_MAP(CStateDlg)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStateDlg message handlers

int CStateDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_state_text.SetColor(RGB(255,0,0));
	m_level_high_text.SetColor(RGB(255,0,0));
	m_level_low_text.SetColor(RGB(255,0,0));

	m_last_time.SetColor(RGB(255,0,0));
	m_last_pc.SetColor(RGB(255,0,0));
	m_last_ip.SetColor(RGB(255,0,0));
	m_last_mac.SetColor(RGB(255,0,0));
	m_internal.SetColor(RGB(255,0,0));
	m_extern.SetColor(RGB(255,0,0));

	m_safe_level_0_bitmap.LoadBitmap(IDB_SAFE_LEVEL_0);
	m_safe_level_1_bitmap.LoadBitmap(IDB_SAFE_LEVEL_1);
	m_safe_level_2_bitmap.LoadBitmap(IDB_SAFE_LEVEL_2);
	m_safe_level_3_bitmap.LoadBitmap(IDB_SAFE_LEVEL_3);

	m_slider_0_bitmap.LoadBitmap(IDB_SLIDER_0);
	m_slider_1_bitmap.LoadBitmap(IDB_SLIDER_1);
	m_slider_2_bitmap.LoadBitmap(IDB_SLIDER_2);
	m_slider_3_bitmap.LoadBitmap(IDB_SLIDER_3);

	return 0;
}

HBRUSH CStateDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	return m_back_brush;
}

void CStateDlg::SetSafeLevel()
{
	UINT  iLevel = 0;
	PGLOBAL_CONFIG CurrentConfig;
	g_Config->GetCurrentConfig(&CurrentConfig);
	if(CurrentConfig->ANTIGATEWAY)
	{
		iLevel ++;
	}
	if(CurrentConfig->ANTISAMEIP)
	{
		iLevel ++;
	}
	if(CurrentConfig->ANTISEND)
	{
		iLevel ++;
	}

	switch(iLevel)
	{
	case 0:
		{	
			m_state_text.SetWindowText(TEXT("未启用任何保护措施"));
			m_safe_level.SetBitmap(m_safe_level_0_bitmap);
			m_level_slider.SetBitmap(m_slider_0_bitmap);
			break;
		}
	case 1:
		{
			if(CurrentConfig->ANTIGATEWAY)
			{
				m_state_text.SetWindowText(TEXT("抵御网关劫持攻击"));
			}
			else if(CurrentConfig->ANTISAMEIP)
			{
				m_state_text.SetWindowText(TEXT("抵御IP冲突攻击"));
			}
			else if(CurrentConfig->ANTISEND)
			{
				m_state_text.SetWindowText(TEXT("禁止本机对外攻击"));
			}

			m_safe_level.SetBitmap(m_safe_level_1_bitmap);
			m_level_slider.SetBitmap(m_slider_1_bitmap);

			break;
		}
	case 2:
		{
			if(CurrentConfig->ANTIGATEWAY && CurrentConfig->ANTISAMEIP)
			{
				m_state_text.SetWindowText(TEXT("抵御网关劫持与IP冲突攻击"));
			}
			else if(CurrentConfig->ANTIGATEWAY && CurrentConfig->ANTISEND)
			{
				m_state_text.SetWindowText(TEXT("抵御网关劫持与对外攻击"));
			}
			else if(CurrentConfig->ANTISAMEIP && CurrentConfig->ANTISEND)
			{
				m_state_text.SetWindowText(TEXT("抵御IP冲突与对外攻击"));
			}
			else
			{
				m_state_text.SetWindowText(TEXT("未启用任何保护措施"));
			}

			m_safe_level.SetBitmap(m_safe_level_2_bitmap);
			m_level_slider.SetBitmap(m_slider_2_bitmap);
			break;
		}
	case 3:
		{
			m_state_text.SetWindowText(TEXT("启用所有的保护措施"));
			m_safe_level.SetBitmap(m_safe_level_3_bitmap);
			m_level_slider.SetBitmap(m_slider_3_bitmap);

			break;
		}
	default:
		break;
	}
}

BOOL CStateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ListView_SetExtendedListViewStyle(m_gateway_address.m_hWnd,LVS_EX_FULLROWSELECT|LVS_EX_FLATSB);
	ListView_SetExtendedListViewStyle(m_local_address.m_hWnd,LVS_EX_FULLROWSELECT|LVS_EX_FLATSB);

	CRect rc;

	m_gateway_address.GetClientRect(&rc);
	m_gateway_address.InsertColumn(1,_T("网关IP"),LVCFMT_LEFT,(rc.right-rc.left)/2);
	m_gateway_address.InsertColumn(2,_T("网关MAC"),LVCFMT_LEFT,(rc.right-rc.left)/2);

	m_local_address.GetClientRect(&rc);
	m_local_address.InsertColumn(1,_T("本机IP"),LVCFMT_LEFT,(rc.right-rc.left)/2);
	m_local_address.InsertColumn(2,_T("本机MAC"),LVCFMT_LEFT,(rc.right-rc.left)/2);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
