#if !defined(AFX_BFLATBUTTON_H__B8A94FE8_1B0F_4EA5_A0D9_B518007F03DF__INCLUDED_)
#define AFX_BFLATBUTTON_H__B8A94FE8_1B0F_4EA5_A0D9_B518007F03DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyBitmapButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabButton window

class CTabButton : public CButton
{
// Construction
public:
	CTabButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabButton();

	BOOL LoadBitmaps(UINT nIDBitmapResourceNotFocus,
					UINT nIDBitmapResourceSel,
					UINT nIDBitmapResourceHover);

	VOID RedrawButton(BOOL Click);

	// Generated message map functions
protected:
	//{{AFX_MSG(CTabButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	COLORREF	m_SelectedTextColor;
	COLORREF	m_NotFocusTextColor;

	CBitmap		*m_pSelectedBackBitmap;
	CBitmap		*m_pNotFocusBackBitmap;
	CBitmap		*m_pMouseHoverBackBitmap;

	BOOL		m_bSelected;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BFLATBUTTON_H__B8A94FE8_1B0F_4EA5_A0D9_B518007F03DF__INCLUDED_)
