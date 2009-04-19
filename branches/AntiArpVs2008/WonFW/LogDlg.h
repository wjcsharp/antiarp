#if !defined(AFX_LOGDLG_H__ED911BD0_5789_456C_B7AB_A9802A18C5EF__INCLUDED_)
#define AFX_LOGDLG_H__ED911BD0_5789_456C_B7AB_A9802A18C5EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogDlg.h : header file
//

typedef struct _PC_NAME_ITEM
{
	struct _PC_NAME_ITEM*	Next;
	ULONG	IPAddress;
	TCHAR	szHostName[MAX_PATH];
	DWORD	LastQueryTime;
} PC_NAME_ITEM,*PPC_NAME_ITEM;


/////////////////////////////////////////////////////////////////////////////
// CLogDlg dialog

#include "ColorStatic.h"


class CLogDlg : public CDialog ,
				public CThread
{
// Construction
public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor
	~CLogDlg();

public:
	void InsertLog(	TCHAR*	LogTime,
						TCHAR*	AttachType,
						ULONG	IPAddress,
						TCHAR*	AttachSourceMac,
						ULONG	ulAttachCount,
						BOOL	bSend,
						BOOL	bKnownIP);

// Dialog Data
	//{{AFX_DATA(CLogDlg)
	enum { IDD = IDD_LOG };
	CColorStatic	m_logview_count;
	CColorStatic	m_logview_count_text;
	CListCtrl		m_log_list;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	static void	 QueryIPInfoThread(void* pParam);
	void   QueryIPInfoWork();

	// Generated message map functions
	//{{AFX_MSG(CLogDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnClearLogview();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBRUSH		m_back_brush;
	ULONG		m_record_count;
	ULONG		m_send_count;

	ULONG		m_recv_count;

	CRITICAL_SECTION		m_known_section;
	CRITICAL_SECTION		m_unknown_section;

	PPC_NAME_ITEM			m_known_names;
	PPC_NAME_ITEM			m_unknown_names;

	HANDLE					m_h_query_name_event;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGDLG_H__ED911BD0_5789_456C_B7AB_A9802A18C5EF__INCLUDED_)
