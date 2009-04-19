#ifndef CTRAYICON_H
#define CTRAYICON_H


class CTrayIcon  
{
public:
	CTrayIcon();
	CTrayIcon(CWnd* pWnd,UINT uCallbackMessage,LPCTSTR szTip,HICON icon,UINT nID);
	virtual ~CTrayIcon();
public:
	BOOL Create(CWnd* pParent,UINT uCallbackMessage,LPCTSTR szTip,HICON icon,UINT nID);
	BOOL SetIcon(UINT nIDResource);
	void HideIcon();
	void RemoveIcon();
	virtual LRESULT OnTrayNotification(WPARAM uID,LPARAM lEvent);
protected:
	void Initialize();
	BOOL m_bEnabled;
	BOOL m_bHidden;
	NOTIFYICONDATA m_tnd;
	UINT m_DefaultMenuItemID;
	BOOL m_DefaultMenuItemByPos;
};

#endif // !defined(AFX_TRAYICON_H__BA4D27DC_D552_41D2_95A4_480C345D2773__INCLUDED_)
