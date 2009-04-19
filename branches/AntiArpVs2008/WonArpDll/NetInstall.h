// NetInstall.h: interface for the CNetInstall class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETINSTALL_H__2D1BC0B4_C8B2_4676_90FC_9AA3077461F9__INCLUDED_)
#define AFX_NETINSTALL_H__2D1BC0B4_C8B2_4676_90FC_9AA3077461F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NETINSTALL_CLASS	"CNetInstall" 

class CNetInstall  
{
public:
	CNetInstall();
	virtual ~CNetInstall();
public:
	STDMETHOD(DriverSignCheck)(BOOL Enable);
	STDMETHOD(InstallNetServiceDriver)(char* szNetIMProtocolFile);
	STDMETHOD(RemoveNetServiceDriver)(char*  szProtocolComponentName);

};

#endif // !defined(AFX_NETINSTALL_H__2D1BC0B4_C8B2_4676_90FC_9AA3077461F9__INCLUDED_)
