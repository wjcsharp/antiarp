// ColorStatic.cpp : implementation file
//

#include "stdafx.h"
#include "ColorStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorStatic

CColorStatic::CColorStatic() : m_Text_Color(RGB(0,79,219))
{
}

CColorStatic::~CColorStatic()
{
}


BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
	//{{AFX_MSG_MAP(CColorStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CColorStatic::SetWindowText( LPCTSTR lpszString )
{
	CStatic::SetWindowText(lpszString);
	if(IsWindow(m_hWnd))
	{
		RedrawWindow();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CColorStatic message handlers

void CColorStatic::SetColor(COLORREF TextColor)
{
	m_Text_Color = TextColor;
}

void CColorStatic::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	RECT		Clientrc;
	CFont		*pFont,*pOldFont;
	CString		szText;
	int			oldBkMode;
	RECT		textrc;
	TEXTMETRIC metric;

	pFont = GetFont();
	GetWindowText(szText);
	GetClientRect(&Clientrc);

	//选用字体
	pOldFont = dc.SelectObject(pFont);
	// 用透明背景填充设备文件
	oldBkMode = dc.SetBkMode(TRANSPARENT);

	dc.GetTextMetrics(&metric);

	if(!szText.IsEmpty())
	{
		dc.SetTextColor(m_Text_Color);				
		
		//获取字体的位置框
		textrc.left   = Clientrc.left;
		textrc.top    = (Clientrc.bottom + Clientrc.top - metric.tmHeight)/2;
		textrc.bottom = (Clientrc.bottom + Clientrc.top + metric.tmHeight)/2;
		textrc.right  = Clientrc.right;

		
		dc.DrawText(szText,&textrc,DT_LEFT);
	}

	dc.SelectObject(pOldFont);
	dc.SetBkMode(oldBkMode);

}

void CColorStatic::SetColorAndRedraw(COLORREF TextColor)
{
	m_Text_Color = TextColor;
	if(IsWindow(m_hWnd))
	{
		RedrawWindow();
	}
}
