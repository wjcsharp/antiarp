// NetInstall.cpp: implementation of the CNetInstall class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <stdlib.h>
#include <stdio.h>
#include "pch.h"
#include "snetcfg.h"
#include "NetInstall.h"
#pragma comment(lib,"setupapi.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void 
__stdcall
EnableDriverSigningXP(BOOL bEnable );

void 
__stdcall
EnableDriverSigning2K(BOOL bEnable );

HRESULT
GetPnpID (
	LPWSTR lpszInfFile,
    LPWSTR *lppszPnpID);


CNetInstall::CNetInstall()
{

}

CNetInstall::~CNetInstall()
{


}


STDMETHODIMP CNetInstall::InstallNetServiceDriver(char* szNetIMProtocolFile)
{
	HRESULT hr =	S_OK;

	WCHAR			*pwszInfFile = NULL;
	ULONG			nLenOfInfFile = 0;
    
	enum NetClass	nc	=	NC_NetService;

	LPWSTR			pwszPnpID;

	//Calculate the number of wide characters 
	nLenOfInfFile = MultiByteToWideChar( CP_ACP, 0, szNetIMProtocolFile, -1, NULL, 0 );
	pwszInfFile = (PWSTR)malloc( nLenOfInfFile * sizeof(WCHAR) );
	if( !pwszInfFile) return S_FALSE;
	MultiByteToWideChar( CP_ACP, 0, szNetIMProtocolFile, -1, pwszInfFile, nLenOfInfFile );

	hr = GetPnpID( pwszInfFile, &pwszPnpID );

    if ( hr == S_OK ) 
	{
#ifdef _DEBUG
		OutputDebugStringW(L"GetPnpID return : ");
		OutputDebugStringW(pwszPnpID);
#endif
		if(FindIfComponentInstalled(pwszPnpID) != S_OK)
		{

			hr = HrInstallNetComponent(pwszPnpID,nc,pwszInfFile);

		}
		
		CoTaskMemFree( pwszPnpID );
	}
#ifdef _DEBUG
	else
	{
		OutputDebugStringW(L"读取Inf文件的Install Section 出错");
	}
#endif

	free(pwszInfFile);

	return hr;
}

STDMETHODIMP CNetInstall::RemoveNetServiceDriver(char*  szComponentID)
{
	HRESULT			hr	=	S_OK;
	WCHAR			*swzComponentID = NULL;
	ULONG			nLenOfComID = 0;

	//Calculate the number of wide characters 
	nLenOfComID = MultiByteToWideChar( CP_ACP, 0, szComponentID, -1, NULL, 0 );
	swzComponentID = (PWSTR)malloc( nLenOfComID * sizeof(WCHAR) );
	if( !swzComponentID) return S_FALSE;
	MultiByteToWideChar( CP_ACP, 0, szComponentID, -1, swzComponentID, nLenOfComID );

	hr = HrUninstallNetComponent(swzComponentID);

	free(swzComponentID);

	return hr;
}


STDMETHODIMP CNetInstall::DriverSignCheck(BOOL Enable)
{
	OSVERSIONINFO VerInfo;
	VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &VerInfo );

	if(VerInfo.dwPlatformId==VER_PLATFORM_WIN32_NT)
	{
		if(	VerInfo.dwMajorVersion == 5    &&
			VerInfo.dwMinorVersion == 0    )
		{	//Windows 2000
			EnableDriverSigning2K(Enable);
		}
		else if(VerInfo.dwMajorVersion == 5	 &&
				VerInfo.dwMinorVersion >= 1    )
		{	//Windows XP Or Later

			EnableDriverSigningXP(Enable);
		}
	}

	return S_OK;
}


void 
__stdcall
EnableDriverSigning2K(BOOL bEnable )
{
	HKEY hKey;
	DWORD status = 0;
	DWORD size = 4;
	ULONG ucSingValue =0;
	TCHAR lpSubKey[] = TEXT("SOFTWARE\\Microsoft\\Driver Signing\\");
	
	if( bEnable )
		ucSingValue = 1;
	else
		ucSingValue = 0;
	 
	status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, lpSubKey, 0 , KEY_WRITE, &hKey );
	if( status != ERROR_SUCCESS )
		return ;
	
	status = RegSetValueEx( hKey, TEXT("Policy"), 0, REG_BINARY, (CONST BYTE *)&ucSingValue, size );
	
	RegCloseKey( hKey );
	
}

#define HP_HASHVALUE HP_HASHVAL

void 
__stdcall
EnableDriverSigningXP(BOOL bEnable )
{
    HCRYPTPROV hCryptProv; 
    HCRYPTHASH hHash;
    BYTE data[16];
    DWORD len;
    DWORD seed;
    HKEY  hkey;
    char input[4];

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("System\\WPA\\PnP"),0,KEY_READ,&hkey) == ERROR_SUCCESS)
	{
		len=sizeof(seed);
		RegQueryValueEx(hkey,TEXT("seed"),NULL,NULL, (BYTE*)&seed,&len);
        RegCloseKey(hkey);
	}
	else
	{
		seed = 0;
	}

	//--------------------------------------------------------------------
    // Compute the cryptographic hash on the data.

    if(!CryptAcquireContext( &hCryptProv, NULL, NULL, PROV_RSA_FULL,0)) 
	{
		if(GetLastError() == NTE_BAD_KEYSET)
		{
			if(!CryptAcquireContext(&hCryptProv,NULL, NULL, PROV_RSA_FULL,CRYPT_NEWKEYSET))
			{
				CryptReleaseContext(hCryptProv,0);
				return;
			}
		}
    }

	//--------------------------------------------------------------------
    // Create a hash object.

    if(!CryptCreateHash(hCryptProv,CALG_MD5, 0, 0, &hHash)) 
    {
		CryptReleaseContext(hCryptProv,0);
		return;
    }

	if(bEnable)
	{
		input[0]=0;
		input[1]=1;	// 允许执行数字签名检查
		input[2]=0; 
		input[3]=0;
	}
	else
	{
		input[0]=0;
		input[1]=0;	// 关闭数字签名检查
		input[2]=0; 
		input[3]=0;
	}

    if(!CryptHashData(hHash,(unsigned char *)input,sizeof(input),0))
    {
		//出错
		CryptReleaseContext(hCryptProv,0);
		return;
    }

	if(!CryptHashData(hHash,(BYTE*)&seed, sizeof(seed),0))
    {
		//出错
		CryptReleaseContext(hCryptProv,0);
		return;
    }

	//-------------------------------------------------------------------- 
    len=sizeof(data);

    if(!CryptGetHashParam(hHash,HP_HASHVALUE,data,&len,0))
    {
		//出错
		CryptReleaseContext(hCryptProv,0);
		return;
    }

    //--------------------------------------------------------------------
    // HKLM\Software\Microsoft\Windows\CurrentVersion\Setup\PrivateHash 
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"),
					0,
					KEY_WRITE,
					&hkey
					)	==	ERROR_SUCCESS)
	{
		len=sizeof(seed);
		RegSetValueEx(hkey,TEXT("PrivateHash"), 0,REG_BINARY,data,sizeof(data));
		RegCloseKey(hkey);
    }
	else
	{
		goto Exit;
	}

	//还是要执行2000的相同的操作
	EnableDriverSigning2K(bEnable);

Exit:
    // Destroy the hash object.
    if(hHash) 
	{
        if(!CryptDestroyHash(hHash))
		{
			return;
		}
    }
    // Release the CSP.
    if(hCryptProv)
	{
        if(!CryptReleaseContext(hCryptProv,0))
		{
  			return;
		}
    }

}



//
// Function:  GetKeyValue
//
// Purpose:   Retrieve the value of a key from the inf file.
//
// Arguments:
//    hInf        [in]  Inf file handle.
//    lpszSection [in]  Section name.
//    lpszKey     [in]  Key name.
//    dwIndex     [in]  Key index.
//    lppszValue  [out] Key value.
//
// Returns:   S_OK on success, otherwise and error code.
//
// Notes:
//

HRESULT
GetKeyValue (
    HINF hInf,
	LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    DWORD  dwIndex,
    LPWSTR *lppszValue)
{
    INFCONTEXT  infCtx;
//    __range(0, 512) DWORD       dwSizeNeeded;
	DWORD       dwSizeNeeded;
    HRESULT     hr;

    *lppszValue = NULL;

    if ( SetupFindFirstLineW(hInf,
                             lpszSection,
                             lpszKey,
                             &infCtx) == FALSE )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if ( SetupGetStringFieldW(&infCtx,
                              dwIndex,
                              NULL,
                              0,
                              &dwSizeNeeded) )
    {
        *lppszValue = (LPWSTR)CoTaskMemAlloc( sizeof(WCHAR) * dwSizeNeeded );

        if ( !*lppszValue  )
        {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }

        if ( SetupGetStringFieldW(&infCtx,
                                dwIndex,
                                *lppszValue,
                                dwSizeNeeded,
                                NULL) == FALSE )
        {

            hr = HRESULT_FROM_WIN32(GetLastError());

            CoTaskMemFree( *lppszValue );
            *lppszValue = NULL;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}

//
// Function:  GetPnpID
//
// Purpose:   Retrieve PnpID from an inf file.
//
// Arguments:
//    lpszInfFile [in]  Inf file to search.
//    lppszPnpID  [out] PnpID found.
//
// Returns:   TRUE on success.
//
// Notes:
//

HRESULT
GetPnpID (
	LPWSTR lpszInfFile,
    LPWSTR *lppszPnpID)
{
    HINF    hInf;
    LPWSTR  lpszModelSection;
    HRESULT hr;

    *lppszPnpID = NULL;

    hInf = SetupOpenInfFileW( lpszInfFile,
                              NULL,
                              INF_STYLE_WIN4,
                              NULL );

    if ( hInf == INVALID_HANDLE_VALUE )
    {

        return HRESULT_FROM_WIN32(GetLastError());
    }

    //
    // Read the Model section name from Manufacturer section.
    //

    hr = GetKeyValue( hInf,
                      L"Manufacturer",
                      NULL,
                      1,
                      &lpszModelSection );

    if ( hr == S_OK )
    {

        //
        // Read PnpID from the Model section.
        //

        hr = GetKeyValue( hInf,
                          lpszModelSection,
                          NULL,
                          2,
                          lppszPnpID );

        CoTaskMemFree( lpszModelSection );
    }

    SetupCloseInfFile( hInf );

    return hr;
}

