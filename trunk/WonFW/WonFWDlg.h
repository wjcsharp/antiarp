// WonArpFWDlg.h : header file
//

#if !defined(AFX_WONARPFWDLG_H__480FA9A7_AEA6_4E1B_83B0_93039D66E542__INCLUDED_)
#define AFX_WONARPFWDLG_H__480FA9A7_AEA6_4E1B_83B0_93039D66E542__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	AUTO_HIDE_TIME		10

#define WM_TRAYICON			WM_USER + 113
#define WM_SHOWLOG			WM_USER + 114
#define WM_SHOWSETUP		WM_USER + 115
#define WM_SHOWSTATE		WM_USER + 116
#define WM_CLOSEFW			WM_USER + 117


/////////////////////////////////////////////////////////////////////////////
// CWonArpFWDlg dialog

#include "TabButton.h"
#include "StateDlg.h"
#include "LogDlg.h"
#include "Setupdlg.h"
#include "trayicon.h"

class CWonArpFWDlg : public CNewDialog ,
					 public CThread
{
// Construction
public:

	CWonArpFWDlg(CWnd* pParent = NULL);	// standard constructor

//	CStateDlg	m_state_dlg;
//	CSetupDlg	m_setup_dlg;
//	CLogDlg		m_log_dlg;

// Dialog Data
	//{{AFX_DATA(CWonArpFWDlg)
	enum { IDD = IDD_WONARPFW_DIALOG };
	CStatic		m_child_frame;
	CTabButton	m_tab_log;
	CTabButton	m_tab_state;
	CTabButton	m_tab_setup;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWonArpFWDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CTrayIcon m_trayicon;

	// Generated message map functions
	//{{AFX_MSG(CWonArpFWDlg)
	afx_msg LRESULT OnTrayIconMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowState();
	afx_msg void OnShowLog();
	afx_msg void OnShowSetup();
	afx_msg void OnMyExit();
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnState();
	afx_msg void OnSetup();
	afx_msg void OnLog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	static void	 GatewayThread(void* pParam);
	void   GatewayWork();
	static void	 AttachPacketNotify(void* pParam);	
	void	NotifyPacketWork();
private:

	void InitControls();
//	void InitControlInfo();
	HBRUSH		m_back_brush;
	CBitmap		m_tab_focus_bitmap;
	CBitmap		m_tab_not_focus_bitmap;

	PGLOBAL_CONFIG			m_Config;

	HANDLE		m_Notify_Thread_Work_Event;

	ULONG		m_show_id;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WONARPFWDLG_H__480FA9A7_AEA6_4E1B_83B0_93039D66E542__INCLUDED_)
