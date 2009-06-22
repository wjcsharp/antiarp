#if !defined(AFX_SETUPDLG_H__4BB3640D_4B58_4533_AE2A_C75B3376A69C__INCLUDED_)
#define AFX_SETUPDLG_H__4BB3640D_4B58_4533_AE2A_C75B3376A69C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg dialog

class CSetupDlg : public CDialog
{
// Construction
public:
	CSetupDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetupDlg)
	enum { IDD = IDD_SETUP };
	CButton	m_setup_use;
	CButton	m_anti_use;
	CButton	m_import_config;
	CButton	m_export_config;
	CButton	m_auto_mac;
	CButton	m_system_start;
	CButton	m_send_check;
	CButton	m_popup_notify;
	CButton	m_anti_sameip;
	CButton	m_anti_gateway;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetupDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnAntiUse();
	afx_msg void OnSetupUse();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBRUSH		m_back_brush;
public:
	afx_msg void OnBnClickedImportConfig();
	afx_msg void OnBnClickedExportConfig();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETUPDLG_H__4BB3640D_4B58_4533_AE2A_C75B3376A69C__INCLUDED_)
