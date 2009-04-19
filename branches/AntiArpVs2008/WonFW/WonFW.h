// WonArpFW.h : main header file for the WONARPFW application
//

#if !defined(AFX_WONARPFW_H__CC1591C5_E57A_4244_9051_052463005C05__INCLUDED_)
#define AFX_WONARPFW_H__CC1591C5_E57A_4244_9051_052463005C05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

extern BOOL	g_StartupHide;

/////////////////////////////////////////////////////////////////////////////
// CWonArpFWApp:
// See WonArpFW.cpp for the implementation of this class
//

class CWonArpFWApp : public CWinApp
{
public:
	CWonArpFWApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWonArpFWApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWonArpFWApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WONARPFW_H__CC1591C5_E57A_4244_9051_052463005C05__INCLUDED_)
