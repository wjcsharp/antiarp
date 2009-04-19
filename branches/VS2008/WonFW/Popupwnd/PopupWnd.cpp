// RaiseAlert.cpp : implementation file
//

#include "stdafx.h"
#include "PopupWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPopupWnd

CPopupWnd::CPopupWnd()
{

	m_bAllSizeWnd	= FALSE;
	m_bSetCaptured	= FALSE;

	m_bAutoClose = FALSE;
	RegisterWindowClass();

	DWORD dwStyle	= WS_POPUP; 
	DWORD dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

	CreateEx(dwExStyle, POPUPWND_CLASSNAME, NULL, dwStyle, 0, 0, 0, 0, NULL, NULL, NULL);
}

CPopupWnd::~CPopupWnd()
{
	//必须调用
	DestroyWindow();
}

BOOL CPopupWnd::RegisterWindowClass()
{
	// Register the window class if it has not already been registered.
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();
	if(!(::GetClassInfo(hInst, POPUPWND_CLASSNAME	, &wndcls)))
	{
		// otherwise we need to register a new class
		wndcls.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; //
		wndcls.lpfnWndProc		= ::DefWindowProc;
		wndcls.cbClsExtra		= wndcls.cbWndExtra = 0;
		wndcls.hInstance		= hInst;
		wndcls.hIcon			= NULL;
		wndcls.hCursor			= LoadCursor(hInst, IDC_ARROW);
		wndcls.hbrBackground	= NULL;
		wndcls.lpszMenuName		= NULL;
		wndcls.lpszClassName	= POPUPWND_CLASSNAME;
		
		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	} //if

	return TRUE;

}

BEGIN_MESSAGE_MAP(CPopupWnd, CWnd)
	//{{AFX_MSG_MAP(CPopupWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPopupWnd message handlers


void CPopupWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	int iSave = dc.SaveDC();
	
	dc.SetBkMode(TRANSPARENT);

	CRect rc;
	GetClientRect(&rc);

	TEXTMETRIC metric;
	dc.GetTextMetrics(&metric);

	CRect title_rc;
	title_rc.top	= 0;
	title_rc.left	= 0;
	title_rc.right	= rc.right;
	title_rc.bottom = metric.tmHeight * 2;

	CRect body_rc;
	body_rc.top		= metric.tmHeight * 2;
	body_rc.left	= 0;
	body_rc.right	= rc.right;
	body_rc.bottom  = rc.bottom;


	CRect textrc;
	textrc.left  = 0;
	textrc.right = rc.right;
	textrc.top   = title_rc.top + metric.tmHeight/2;
	textrc.bottom = textrc.top + metric.tmHeight;



		CBrush titlebrush(RGB(0xfc,0x71,0x4d));
		CBrush *pOldBrush = dc.SelectObject(&titlebrush);

		dc.Rectangle(&title_rc);

		dc.SetTextColor(RGB(255,255,255));
		
		CString str = _T("  用户禁止项目");

		dc.DrawText(str,&textrc, DT_VCENTER | DT_LEFT);

		dc.SelectObject(pOldBrush);
		titlebrush.DeleteObject();


	
	dc.Rectangle(&body_rc);


	dc.RestoreDC(iSave);

}

void CPopupWnd::OnTimer(UINT nIDEvent) 
{

    switch (nIDEvent)
    {
        case IDT_POP_WINDOW_TIMER:    // When the window comes up
        {
            switch (m_nStatusBarPos)
            {
                case STP_BOTTOM:
                case STP_RIGHT:
                {
                    m_nWndSize+=30;
                    if (m_nWndSize > m_nWndHeight)
                    {
                        KillTimer(IDT_POP_WINDOW_TIMER);

						m_bAllSizeWnd = TRUE;	//窗体已经最大化

                        SetTimer(IDT_SHOW_WINDOW_TIMER, m_nMsgTimeOut, NULL);
                    }
                    else
                    {
                        // Keep sizing the window, show it
                        SetWindowPos(
                            &wndTopMost,
                            m_nWndLeft,
                            m_nWndBottom - m_nWndSize, 
                            m_nWndWidth - 1,
                            m_nWndSize,
                            SWP_SHOWWINDOW
                        );
                    }
                }
                break;

                case STP_TOP:
                {
                    m_nWndSize+=30;
                    if (m_nWndSize > m_nWndHeight)
                    {
                        KillTimer(IDT_POP_WINDOW_TIMER);

						m_bAllSizeWnd = TRUE;	//窗体已经最大化

                        SetTimer(IDT_SHOW_WINDOW_TIMER, m_nMsgTimeOut, NULL);
                    }
                    else
                    {
                        // Keep sizing the window, collapse it
                        SetWindowPos(
                            &wndTop,
                            m_nWndLeft,
                            m_nWndTop, 
                            m_nWndWidth - 1,
                            m_nWndSize,
                            SWP_SHOWWINDOW
                        );
                    }
                }
                break;

                case STP_LEFT:
                {
                    m_nWndSize+=30;
                    if (m_nWndSize > m_nWndHeight)
                    {
                        KillTimer(IDT_POP_WINDOW_TIMER);
						
						m_bAllSizeWnd = TRUE;

                        SetTimer(IDT_SHOW_WINDOW_TIMER, m_nMsgTimeOut, NULL);
                    }
                    else
                    {
                        // Keep sizing the window, collpase it
                        SetWindowPos(
                            &wndTopMost,
                            m_nWndLeft + 1,
                            m_nWndBottom - m_nWndSize, 
                            m_nWndWidth,
                            m_nWndSize,
                            SWP_SHOWWINDOW
                        );
                    }
                }
                break;
            }
        }
        break;

        case IDT_SHOW_WINDOW_TIMER:
        {
            KillTimer(IDT_SHOW_WINDOW_TIMER);

			if(m_bAutoClose)
			{

				OnClose();
			}

        }
        break;
		default:
			break;

    }
	
}

void CPopupWnd::Popup(BOOL bAutoClose,int iDelayMiscSeconds,int nWidth,int nHeight,BOOL bShowCenter)
{

		if(IsWindowVisible()) return;


    m_nWndWidth  = nWidth;
    m_nWndHeight = nHeight;

	m_nWndSize = 0;

	if(iDelayMiscSeconds != 0)
	{
		m_nMsgTimeOut          = iDelayMiscSeconds;
	}
	else
		m_nMsgTimeOut          = 5000;

    m_nMsgWndCreationDelay = 10;

	m_bAutoClose = bAutoClose;
	if(!bShowCenter)
	{

		if (CheckIfStatusBarBottom())  // Most frequent case is status bar at bottom
		{
			PopWndForBottomStatusBar();
		}
		else
		{
			if (CheckIfStatusBarTop())
			{
				PopWndForTopStatusBar();
			}
			else
			{
				if (CheckIfStatusBarLeft())
				{
					PopWndForLeftStatusBar();
				}
				else
				{
					m_nStatusBarPos = STP_RIGHT; // Force it, as no need for calling "CheckIfStatusBarRight()
					PopWndForRightStatusBar();
				}
			}
		}
	}
	else
	{
		//将窗体置中
		MoveChildCenter();
		ShowCenter();
		return;
	}
	//将窗体置中
	MoveChildCenter();

}


void CPopupWnd::PopWndForLeftStatusBar()
{
    
    CRect rectDesktopWithoutTaskbar;   // The desktop area 

    // Get the desktop area
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    
    // Calculate the actual width of the Window and its position
    m_nWndLeft   = rectDesktopWithoutTaskbar.left;
    m_nWndTop    = rectDesktopWithoutTaskbar.bottom - m_nWndHeight;
    m_nWndRight  = m_nWndLeft + m_nWndWidth;
    m_nWndBottom = m_nWndTop + m_nWndHeight;
    
    m_nWndSize = 0; // The height of window is zero before showing

    SetTimer(IDT_POP_WINDOW_TIMER, m_nMsgWndCreationDelay, NULL);

}


void CPopupWnd::PopWndForRightStatusBar()
{
    PopWndForBottomStatusBar();
}

void CPopupWnd::PopWndForTopStatusBar()
{
    
    CRect rectDesktopWithoutTaskbar;   // The desktop area 

    // Get the desktop area
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    
    // Calculate the actual width of the Window and its position in screen co-ordinates
    m_nWndLeft   = rectDesktopWithoutTaskbar.right - m_nWndWidth;
    m_nWndTop    = rectDesktopWithoutTaskbar.top;
    m_nWndRight  = m_nWndLeft + m_nWndWidth;
    m_nWndBottom = m_nWndTop + m_nWndHeight;

    m_nWndSize = 0; // The height of window is zero before showing

    SetTimer(IDT_POP_WINDOW_TIMER, m_nMsgWndCreationDelay, NULL);
}


void CPopupWnd::PopWndForBottomStatusBar()
{
    
    CRect rectDesktopWithoutTaskbar;   // The desktop area 

    // Get the desktop area
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    
    // Calculate the actual width of the Window and its position in screen co-ordinates
    m_nWndLeft   = rectDesktopWithoutTaskbar.right - m_nWndWidth;
    m_nWndTop    = rectDesktopWithoutTaskbar.bottom - m_nWndHeight;
    m_nWndRight  = m_nWndLeft + m_nWndWidth;
    m_nWndBottom = m_nWndTop + m_nWndHeight;
    
    m_nWndSize = 0; // The height of window is zero before showing

    SetTimer(IDT_POP_WINDOW_TIMER, m_nMsgWndCreationDelay, NULL);
}



BOOL CPopupWnd::CheckIfStatusBarLeft()
{
    unsigned int nAvailableScreenTop;
    unsigned int nAvailableScreenLeft;
    
    CRect rectDesktopWithoutTaskbar;   // The desktop area without status bar
    
    // Get the desktop area minus the status
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    nAvailableScreenLeft  = rectDesktopWithoutTaskbar.left;
    nAvailableScreenTop = rectDesktopWithoutTaskbar.top;

    if ((nAvailableScreenLeft > 0) && (nAvailableScreenTop == 0))
    {
        m_nStatusBarPos = STP_LEFT;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL CPopupWnd::CheckIfStatusBarRight()
{
    unsigned int nAvailableScreenWidth;
    unsigned int nAvailableScreenHeight;
    unsigned int nActualScreenWidth;
    unsigned int nActualScreenHeight;

    // Calculate the actual screen height and width
    nActualScreenWidth  = ::GetSystemMetrics(SM_CXFULLSCREEN);
    nActualScreenHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);

  
    CRect rectDesktopWithoutTaskbar;   // The desktop area without status bar
    
    // Get the desktop area minus the status
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    nAvailableScreenWidth  = rectDesktopWithoutTaskbar.Width();
    nAvailableScreenHeight = rectDesktopWithoutTaskbar.Height();

    if ((nAvailableScreenWidth != nActualScreenWidth) &&
        (nAvailableScreenHeight == nActualScreenHeight))
    {
        m_nStatusBarPos = STP_RIGHT;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL CPopupWnd::CheckIfStatusBarTop()
{
    unsigned int nAvailableScreenTop;
    unsigned int nAvailableScreenLeft;
    
    CRect rectDesktopWithoutTaskbar;   // The desktop area without status bar
    
    // Get the desktop area minus the status
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    nAvailableScreenLeft  = rectDesktopWithoutTaskbar.left;
    nAvailableScreenTop = rectDesktopWithoutTaskbar.top;

    if ((nAvailableScreenLeft == 0) && (nAvailableScreenTop > 0))
    {
        m_nStatusBarPos = STP_TOP;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL CPopupWnd::CheckIfStatusBarBottom()
{
    unsigned int nAvailableScreenWidth;
    unsigned int nAvailableScreenBottom;
    unsigned int nActualScreenWidth;
    unsigned int nActualScreenBottom;

    // Calculate the actual screen height and width
    nActualScreenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
    nActualScreenBottom = ::GetSystemMetrics(SM_CYSCREEN);

  
    CRect rectDesktopWithoutTaskbar;   // The desktop area without status bar
    
    // Get the desktop area minus the status
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    nAvailableScreenWidth  = rectDesktopWithoutTaskbar.Width();
    nAvailableScreenBottom = rectDesktopWithoutTaskbar.bottom;

    if ((nAvailableScreenWidth == nActualScreenWidth) &&
        (nAvailableScreenBottom < nActualScreenBottom))
    {
        m_nStatusBarPos = STP_BOTTOM;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CPopupWnd::MoveChildCenter()
{
	CWnd *pChild = NULL;

	CClientDC dc(this);
	CRect rc(0,0,0,0);
	TEXTMETRIC metric;
	dc.GetTextMetrics(&metric);
	
	rc.top = metric.tmHeight * 2;
	rc.bottom = m_nWndHeight;

	CRect childrc;
	pChild->GetClientRect(&childrc);
	
	CRect new_rc;

	new_rc.left		= (m_nWndWidth - childrc.right) /2;
	new_rc.right	= new_rc.left + childrc.right - childrc.left;
	new_rc.top		= metric.tmHeight + (m_nWndHeight - childrc.bottom)/2;
	new_rc.bottom   = new_rc.top + childrc.bottom;


	pChild->MoveWindow(&new_rc);
	
	pChild->ShowWindow(SW_SHOW);
}


BOOL CPopupWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL CPopupWnd::CreateChildWnd(LPCTSTR text )
{

	return FALSE;

}

void CPopupWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(m_bAllSizeWnd)
	{
		m_OldMousePoint = point;
		ClientToScreen(&m_OldMousePoint);
		
		SetCursor(LoadCursor(NULL,IDC_SIZEALL));
		
		m_bSetCaptured = TRUE;
		SetCapture();
	}

}

void CPopupWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_bAllSizeWnd && m_bSetCaptured)
	{
		ReleaseCapture();
		m_bSetCaptured = FALSE;
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		MoveWnd();
	}

}

void CPopupWnd::MoveWnd()
{
	CPoint pos;
	GetCursorPos(&pos);
	CRect clientrc;
	GetClientRect(&clientrc);
	ClientToScreen(&clientrc);
	CRect new_rc;
	
	new_rc.left  = clientrc.left + (pos.x - m_OldMousePoint.x);
	new_rc.right = clientrc.right + (pos.x - m_OldMousePoint.x);
	new_rc.top   = clientrc.top  + (pos.y - m_OldMousePoint.y);
	new_rc.bottom = clientrc.bottom + (pos.y - m_OldMousePoint.y);

	MoveWindow(new_rc.left,new_rc.top,clientrc.right - clientrc.left,clientrc.bottom - clientrc.top);

}

void CPopupWnd::ShowCenter()
{
	//高 m_nWndHeight
	//宽 m_nWndWidth;
	//左 m_nWndLeft
	//右 m_nWndRight
	//上 m_nWndTop
	//底 m_nWndBottom
	int iMaxScreenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	int iMaxScreenHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	m_nWndLeft	=	(iMaxScreenWidth - m_nWndWidth) /2;
	m_nWndTop	=	(iMaxScreenHeight - m_nWndHeight) /2;

    SetWindowPos( &wndTopMost,
                   m_nWndLeft,
                   m_nWndTop, 
                   m_nWndWidth,
                   m_nWndHeight,
                   SWP_SHOWWINDOW
                   );

	m_bAllSizeWnd = TRUE;	//窗体已经最大化
}

void CPopupWnd::OnClose() 
{

		ShowWindow(SW_HIDE);
		return;
	
	
		delete this;

}

void CPopupWnd::ForceClose()
{
	delete this;	
	
}

