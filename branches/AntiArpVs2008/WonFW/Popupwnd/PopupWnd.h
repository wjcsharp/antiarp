#ifndef POPUPWND_H
#define POPUPWND_H


#define POPUPWND_CLASSNAME		_T("PopupWnd")

#define WM_EXECUTE_OK				WM_USER + 0x7FFE - 100
#define WM_EXECUTE_CANCEL			WM_USER + 0x7FFE - 101


#define WM_DELETE_ALERT				WM_USER + 0x7FFE - 201

const int IDT_POP_WINDOW_TIMER       = 100;
const int IDT_COLLAPSE_WINDOW_TIMER  = 101;
const int IDT_SHOW_WINDOW_TIMER      = 102;

const int STP_BOTTOM = 200;
const int STP_TOP    = 201;
const int STP_RIGHT  = 202;
const int STP_LEFT   = 203;

class CPopupWnd : public CWnd
{
// Construction
public:
	void ForceClose();
	CPopupWnd();
	~CPopupWnd();

	BOOL CreateChildWnd(LPCTSTR text = "");

	void Popup(BOOL bAutoClose=TRUE,int iDelayMiscSeconds=5000,int nWidth = 240,int nHeight = 340,BOOL bShowCenter = FALSE);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupWnd)
	//}}AFX_VIRTUAL
protected:
	void MoveChildCenter();
	static BOOL RegisterWindowClass();
	//{{AFX_MSG(CPopupWnd)
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void ShowCenter();
	void MoveWnd();

	BOOL CheckIfStatusBarBottom(); // Is Status bar at bottom ?
	BOOL CheckIfStatusBarTop();    // Is Status bar on top ?
	BOOL CheckIfStatusBarRight();  // Is Status bar at right side?
	BOOL CheckIfStatusBarLeft();   // Is Status bar at left side?
	
	void PopWndForBottomStatusBar(); // Pop window when status bar is at bottom
	void PopWndForTopStatusBar();    // Pop window when status bar on top
	void PopWndForRightStatusBar();  // Pop window when status bar is at right side
	void PopWndForLeftStatusBar();   // Pop window when status bar is at left side
	
    unsigned int m_nWndWidth;            // Message window width
	unsigned int m_nWndHeight;           // Mesaage window height
    unsigned int m_nMsgTimeOut;          // Seconds the window remains stationary
    unsigned int m_nMsgWndCreationDelay; // Seconds in which the window gets shown

    unsigned int m_nWndLeft;   // Message window left corner screen coordinates   
    unsigned int m_nWndTop;    // Message window top corner screen coordinates
    unsigned int m_nWndRight;  // Message window right corner screen coordinates
    unsigned int m_nWndBottom; // Message window bottom corner screen coordinates
    unsigned int m_nWndSize;    // Temp variable for storing window size for animation

    unsigned int m_nStatusBarPos; // const about where the status bar.

	BOOL m_bAutoClose;

public:
	//各种子窗体

protected:
	//鼠标按下的位置
	CPoint m_OldMousePoint;
	BOOL   m_bAllSizeWnd;
	BOOL   m_bSetCaptured;
};


#endif 
