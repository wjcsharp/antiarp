// IniFile.cpp: implementation of the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile()
{

}

CIniFile::~CIniFile()
{

}

UINT CIniFile::GetIntegerValue(LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpFileName)
{
	return ::GetPrivateProfileInt(lpSectionName,
									lpKeyName,
									0xFFFFFFFF,
									lpFileName
									);

}

LPCTSTR CIniFile::GetStringValue(LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpFileName)
{
	static TCHAR str[MAX_PATH];
	int strSize = MAX_PATH;
	::GetPrivateProfileString(lpSectionName,lpKeyName,_T(""),str,strSize,lpFileName);
	return str;

}
/***********************************************************************************************

lpszReturnBuffer 
	[out] Pointer to a buffer that receives the section names associated with the named file.
	The buffer is filled with one or more null-terminated strings; 
	the last string is followed by a second null character. 
nSize 
	[in] Size of the buffer pointed to by the lpszReturnBuffer parameter, in TCHARs. 
lpFileName 
	[in] Pointer to a null-terminated string that specifies the name of the initialization file.
	If this parameter is NULL, the function searches the Win.ini file.
	If this parameter does not contain a full path to the file, the system searches for the file 
	in the Windows directory. 


Return Values
	The return value specifies the number of characters copied to the specified buffer, 
	not including the terminating null character. If the buffer is not large enough to 
	contain all the section names associated with the specified initialization file, 
	the return value is equal to the size specified by nSize minus two.

***********************************************************************************************/

LPCTSTR CIniFile::GetSectionNames(LPCTSTR lpFileName)
{
	static TCHAR str[MAX_PATH];
	int nSize = MAX_PATH;
	::GetPrivateProfileSectionNames(str,nSize,lpFileName);
	return str;
}


BOOL CIniFile::AddSection(LPCTSTR lpSectionName,  LPCTSTR lpFileName)
{
	return ::WritePrivateProfileSection(lpSectionName,_T(""),lpFileName);
}

/*************************************************************************************************


Return Values
	If the function successfully copies the string to the initialization file, the return value is 
	nonzero.

	If the function fails, or if it flushes the cached version of the most recently accessed
	initialization
	 file, the return value is zero. To get extended error information, call GetLastError.

*************************************************************************************************/

BOOL CIniFile::SetKeyValue(LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	return ::WritePrivateProfileString(lpSectionName,lpKeyName,lpString,lpFileName);
}

LPTSTR CIniFile::GetSectionAllKeyValue(LPCTSTR lpSectionName, LPCTSTR lpFileName)
{
	static char allkeys[32760];
	DWORD nSize = 32760;
	nSize = GetPrivateProfileSection(lpSectionName,allkeys,nSize,lpFileName);
	return allkeys;
}

BOOL CIniFile::DelKeyValue(LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpFileName)
{
	return ::WritePrivateProfileString(lpSectionName,lpKeyName,NULL,lpFileName);
	
}
