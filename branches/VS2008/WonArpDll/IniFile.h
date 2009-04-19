#ifndef INIFILE_H
#define INIFILE_H

class CIniFile
{
public:
	static BOOL     DelKeyValue(LPCTSTR lpSectionName,LPCTSTR lpKeyName,LPCTSTR lpFileName);
	static LPTSTR   GetSectionAllKeyValue(LPCTSTR lpSectionName,LPCTSTR lpFileName);
	static BOOL		SetKeyValue(LPCTSTR lpSectionName,LPCTSTR lpKeyName, LPCTSTR lpString,LPCTSTR lpFileName);
	static BOOL		AddSection(LPCTSTR lpSectionName, LPCTSTR lpFileName);
	static LPCTSTR  GetSectionNames(LPCTSTR lpFileName);
	static LPCTSTR	GetStringValue(LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpFileName);
	static UINT		GetIntegerValue(LPCTSTR SectionName,LPCTSTR lpKeyName,LPCTSTR lpFileName);
	CIniFile();
	~CIniFile();
};



#endif
