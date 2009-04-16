#if !defined(AFX_STATE_H__CB23162B_1D02_466C_863F_78B6696CBC60__INCLUDED_)
#define AFX_STATE_H__CB23162B_1D02_466C_863F_78B6696CBC60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// State.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStateDlg dialog

#include "ColorStatic.h"

class CStateDlg : public CDialog
{
// Construction
public:
	void SetSafeLevel();
	CStateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStateDlg)
	enum { IDD = IDD_STATE };
	CListCtrl		m_local_address;
	CListCtrl		m_gateway_address;
	CColorStatic	m_last_time;
	CColorStatic	m_last_pc;
	CColorStatic	m_last_mac;
	CColorStatic	m_last_ip;
	CColorStatic	m_internal;
	CColorStatic	m_extern;
	CStatic			m_level_slider;
	CColorStatic	m_level_low_text;
	CColorStatic	m_level_high_text;
	CStatic			m_safe_level;
	CColorStatic	m_state_text;
	CColorStatic	m_lasttime_text;
	CColorStatic	m_last_pc_text;
	CColorStatic	m_last_mac_text;
	CColorStatic	m_internal_text;
	CColorStatic	m_last_ip_text;
	CColorStatic	m_extern_text;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStateDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBRUSH		m_back_brush;

	//安全等级的各种位图
	CBitmap		m_safe_level_0_bitmap;
	CBitmap		m_safe_level_1_bitmap;
	CBitmap		m_safe_level_2_bitmap;
	CBitmap		m_safe_level_3_bitmap;

	CBitmap		m_slider_0_bitmap;
	CBitmap		m_slider_1_bitmap;
	CBitmap		m_slider_2_bitmap;
	CBitmap		m_slider_3_bitmap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATE_H__CB23162B_1D02_466C_863F_78B6696CBC60__INCLUDED_)
