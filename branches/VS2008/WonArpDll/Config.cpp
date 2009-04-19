// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include "Config.h"
#include "IniFile.h"

#define CONFIG_FILE		TEXT("WonArpFW.cfg")

CIniFile		g_ConfigFile;

GLOBAL_CONFIG	g_Config = {TRUE,TRUE,TRUE,TRUE,TRUE,TRUE};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	UINT i = 0xFFFFFFFF;
	i = g_ConfigFile.GetIntegerValue(TEXT("CONFIG"),TEXT("ANTIGATEWAY"),CONFIG_FILE);
	if(i == 0xFFFFFFFF)
	{
		//文件不存在
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTIGATEWAY"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		if( i != 0)
		{
			g_Config.ANTIGATEWAY = TRUE;
		}
		else
		{
			g_Config.ANTIGATEWAY = FALSE;
		}
	}

	i = g_ConfigFile.GetIntegerValue(TEXT("CONFIG"),TEXT("ANTISAMEIP"),CONFIG_FILE);
	if(i == 0xFFFFFFFF)
	{
		//文件不存在
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTISAMEIP"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		if( i != 0)
		{
			g_Config.ANTISAMEIP = TRUE;
		}
		else
		{
			g_Config.ANTISAMEIP = FALSE;
		}
	}

	i = g_ConfigFile.GetIntegerValue(TEXT("CONFIG"),TEXT("ANTISEND"),CONFIG_FILE);
	if(i == 0xFFFFFFFF)
	{
		//文件不存在
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTISEND"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		if( i != 0)
		{
			g_Config.ANTISEND = TRUE;
		}
		else
		{
			g_Config.ANTISEND = FALSE;
		}
	}
	
	i = g_ConfigFile.GetIntegerValue(TEXT("CONFIG"),TEXT("SYSTEM_BOOT_AUTOSTART"),CONFIG_FILE);
	if(i == 0xFFFFFFFF)
	{
		//文件不存在
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("SYSTEM_BOOT_AUTOSTART"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		if( i != 0)
		{
			g_Config.SYSTEM_BOOT_AUTOSTART = TRUE;
		}
		else
		{
			g_Config.SYSTEM_BOOT_AUTOSTART = FALSE;
		}
	}

	i = g_ConfigFile.GetIntegerValue(TEXT("CONFIG"),TEXT("POPUP_NOTIFY"),CONFIG_FILE);
	if(i == 0xFFFFFFFF)
	{
		//文件不存在
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("POPUP_NOTIFY"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		if( i != 0)
		{
			g_Config.POPUP_NOTIFY = TRUE;
		}
		else
		{
			g_Config.POPUP_NOTIFY = FALSE;
		}
	}

	g_Config.AUTO_REGCOGNIZE_GATEWAY_MAC  = TRUE;

}

CConfig::~CConfig()
{

}

STDMETHODIMP CConfig::GetCurrentConfig(PGLOBAL_CONFIG* Config)
{
	*Config = &g_Config;
	return S_OK;
}

STDMETHODIMP CConfig::SaveCurrentConfig(GLOBAL_CONFIG* Config)
{
	// 1
	if(g_Config.SYSTEM_BOOT_AUTOSTART)
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("SYSTEM_BOOT_AUTOSTART"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("SYSTEM_BOOT_AUTOSTART"),TEXT("0"),CONFIG_FILE);
	}

	// 2
	if(g_Config.POPUP_NOTIFY)
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("POPUP_NOTIFY"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("POPUP_NOTIFY"),TEXT("0"),CONFIG_FILE);
	}

	// 3
	if(g_Config.ANTIGATEWAY)
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTIGATEWAY"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTIGATEWAY"),TEXT("0"),CONFIG_FILE);
	}

	// 4
	if(g_Config.ANTISAMEIP)
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTISAMEIP"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTISAMEIP"),TEXT("0"),CONFIG_FILE);
	}

	//5
	if(g_Config.ANTISEND)
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTISEND"),TEXT("1"),CONFIG_FILE);
	}
	else
	{
		g_ConfigFile.SetKeyValue(TEXT("CONFIG"),TEXT("ANTISEND"),TEXT("0"),CONFIG_FILE);
	}

	return S_OK;
}