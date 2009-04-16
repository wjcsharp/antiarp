#if !defined(AFX_COLORSTAIC_H__FADE4DB1_C16E_4732_B968_F9A7A3F66B0B__INCLUDED_)
#define AFX_COLORSTAIC_H__FADE4DB1_C16E_4732_B968_F9A7A3F66B0B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorStatic window

class CColorStatic : public CStatic
{
// Construction
public:
	CColorStatic(); // RGB(0,79,219)

// Attributes
public:

// Operations
public:
	void SetColor(COLORREF TextColor);
	void SetColorAndRedraw(COLORREF TextColor);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorStatic)
	void SetWindowText( LPCTSTR lpszString );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	COLORREF		m_Text_Color;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORSTAIC_H__FADE4DB1_C16E_4732_B968_F9A7A3F66B0B__INCLUDED_)
