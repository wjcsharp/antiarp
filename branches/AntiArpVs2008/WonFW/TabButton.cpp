// BFlatButton.cpp : implementation file
//

#include "stdafx.h"
#include "TabButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabButton

CTabButton::CTabButton() : m_SelectedTextColor(RGB(255,255,255)) ,
									 m_NotFocusTextColor(RGB(0,85,138))    ,
									 m_pSelectedBackBitmap(NULL),
									 m_pNotFocusBackBitmap(NULL),
									 m_pMouseHoverBackBitmap(NULL),
									 m_bSelected(FALSE)


{
}

CTabButton::~CTabButton()
{
	if(m_pNotFocusBackBitmap)
	{
		delete m_pNotFocusBackBitmap;
		m_pNotFocusBackBitmap = NULL;
	}
	
	if(m_pSelectedBackBitmap)
	{
		delete m_pSelectedBackBitmap;
		m_pSelectedBackBitmap = NULL;
	}

	if(m_pMouseHoverBackBitmap)
	{
		delete m_pMouseHoverBackBitmap;
		m_pMouseHoverBackBitmap = NULL;
	}
}


BEGIN_MESSAGE_MAP(CTabButton, CButton)
	//{{AFX_MSG_MAP(CTabButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabButton message handlers

void CTabButton::PreSubclassWindow() 
{
	CButton::PreSubclassWindow();
	ModifyStyle(0,BS_OWNERDRAW);
}

BOOL CTabButton::LoadBitmaps(UINT nIDBitmapResourceNotFocus,
								  UINT nIDBitmapResourceSel,
								  UINT nIDBitmapResourceHover )
{
	
	m_pNotFocusBackBitmap = new CBitmap;
	m_pNotFocusBackBitmap->LoadBitmap(nIDBitmapResourceNotFocus);

	m_pSelectedBackBitmap = new CBitmap;
	m_pSelectedBackBitmap->LoadBitmap(nIDBitmapResourceSel);

	m_pMouseHoverBackBitmap = new CBitmap;
	m_pMouseHoverBackBitmap->LoadBitmap(nIDBitmapResourceHover);

	return TRUE;
}

void CTabButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC		*pDC;
	CDC		MemDC;
	CString strText;
	CRect	rcBtn;

	TEXTMETRIC metric;
	int		oldBkMode;
	CFont	*pFont,*pOldFont;
	RECT	Textrc;

	BITMAP	BackBitmap;
	CBitmap *pOldBitmap;

	ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

	GetWindowText(strText);

	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	oldBkMode  = pDC->SetBkMode(TRANSPARENT);
	
	pFont = GetFont();

	pOldFont = pDC->SelectObject(pFont);

	//获取字体尺寸
	pDC->GetTextMetrics(&metric);
	
	//获取字体的位置框
	Textrc.left   = lpDrawItemStruct->rcItem.left;
	Textrc.top    = (lpDrawItemStruct->rcItem.bottom + lpDrawItemStruct->rcItem.top - metric.tmHeight)/2;
	Textrc.bottom = (lpDrawItemStruct->rcItem.bottom + lpDrawItemStruct->rcItem.top + metric.tmHeight)/2;
	Textrc.right  = lpDrawItemStruct->rcItem.right;

	MemDC.CreateCompatibleDC(pDC);

	rcBtn.CopyRect(&lpDrawItemStruct->rcItem);

	if(m_bSelected) //lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		if(m_pSelectedBackBitmap)
		{
			pOldBitmap = MemDC.SelectObject(m_pSelectedBackBitmap);

			m_pSelectedBackBitmap->GetBitmap(&BackBitmap);

			pDC->StretchBlt(rcBtn.left,rcBtn.top,rcBtn.Width(),rcBtn.Height(),
                      &MemDC,0,0,BackBitmap.bmWidth,BackBitmap.bmHeight,SRCCOPY); 

			MemDC.SelectObject(pOldBitmap);

		}

		pDC->SetTextColor(m_SelectedTextColor); //SelTextColor

		pDC->DrawText(strText,&Textrc,DT_CENTER);

	}
	else
	{
		if(m_pNotFocusBackBitmap)
		{
			pOldBitmap = MemDC.SelectObject(m_pNotFocusBackBitmap);

			m_pNotFocusBackBitmap->GetBitmap(&BackBitmap);

			pDC->StretchBlt(rcBtn.left,rcBtn.top,rcBtn.Width(),rcBtn.Height(),
                      &MemDC,0,0,BackBitmap.bmWidth,BackBitmap.bmHeight,SRCCOPY); 

			MemDC.SelectObject(pOldBitmap);

		}

		pDC->SetTextColor(m_NotFocusTextColor); //SelTextColor

		pDC->DrawText(strText,&Textrc,DT_CENTER);

	}

	pDC->SelectObject(pOldFont);
	//还原模式
	pDC->SetBkMode(oldBkMode);

}

VOID CTabButton::RedrawButton(BOOL Click)
{
	if(m_bSelected != Click)
	{
		m_bSelected = Click;
		Invalidate(TRUE);
	}
}
