// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2F97958C_5345_49C5_B38E_B80AEB2964C6__INCLUDED_)
#define AFX_STDAFX_H__2F97958C_5345_49C5_B38E_B80AEB2964C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////

//#define WINVER 0x0500
#define DBGTestGUI 1

//////////////////////////////////////////////////////////////////////////

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <Winsock2.h>
#include "iphlpapi.h"
#include <windows.h>
#include <stdio.h>
#pragma comment(lib,"iphlpapi.lib")

#include "NewMenu\NewMenu.h"
#include "Thread.h"

#include "Object.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2F97958C_5345_49C5_B38E_B80AEB2964C6__INCLUDED_)
