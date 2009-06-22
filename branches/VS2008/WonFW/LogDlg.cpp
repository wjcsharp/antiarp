// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WonFW.h"
#include "LogDlg.h"
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogDlg dialog


CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_back_brush = ::CreateSolidBrush(RGB(255,255,255));

	m_record_count = 0;

	m_send_count   = 0;

	m_recv_count   = 0;

	InitializeCriticalSection(&m_known_section);
	InitializeCriticalSection(&m_unknown_section);

	m_known_names	= NULL;
	m_unknown_names = NULL;

	m_h_query_name_event = CreateEvent(NULL,TRUE,FALSE,NULL);

}

CLogDlg::~CLogDlg()
{
	PPC_NAME_ITEM			QueryItem = NULL;
	PVOID					pVoid   = NULL;

	CloseHandle(m_h_query_name_event);

	EnterCriticalSection(&m_known_section);
	{
		QueryItem		= m_known_names;
		m_known_names	= NULL;
	}
	while(QueryItem)
	{
		pVoid		= (PVOID)QueryItem;
		QueryItem	= QueryItem->Next;
		free(pVoid);
	}
	LeaveCriticalSection(&m_known_section);

	EnterCriticalSection(&m_unknown_section);
	{
		QueryItem		= m_unknown_names;
		m_unknown_names	= NULL;
	}
	while(QueryItem)
	{
		pVoid		= (PVOID)QueryItem;
		QueryItem	= QueryItem->Next;
		free(pVoid);
	}
	LeaveCriticalSection(&m_unknown_section);

	::DeleteCriticalSection(&m_known_section);
	::DeleteCriticalSection(&m_unknown_section);

}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogDlg)
	DDX_Control(pDX, IDC_LOGVIEW_COUNT, m_logview_count);
	DDX_Control(pDX, IDC_LOGVIEW_COUNT_TEXT, m_logview_count_text);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	//{{AFX_MSG_MAP(CLogDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CLEAR_LOGVIEW, OnClearLogview)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LOOK_LOG_FILE, &CLogDlg::OnBnClickedLookLogFile)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogDlg message handlers

HBRUSH CLogDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	return m_back_brush;
}


BOOL CLogDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ListView_SetExtendedListViewStyle(m_log_list.m_hWnd,
				LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_FLATSB );

	m_log_list.InsertColumn(1,"攻击起始时间",LVCFMT_LEFT,120);
	m_log_list.InsertColumn(2,"攻击类型",LVCFMT_LEFT,60);
	m_log_list.InsertColumn(3,"可疑攻击源主机",LVCFMT_LEFT,120);
	m_log_list.InsertColumn(4,"可疑攻击源IP",LVCFMT_LEFT,110);
	m_log_list.InsertColumn(5,"可疑攻击源MAC",LVCFMT_LEFT,130);
	m_log_list.InsertColumn(6,"次数",LVCFMT_LEFT,60);

	CThread::CreateThread(QueryIPInfoThread,this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLogDlg::OnClearLogview() 
{

	m_record_count = 0;
	m_logview_count.SetWindowText(TEXT("0"));

	m_log_list.DeleteAllItems();

}

void CLogDlg::QueryIPInfoThread(void* pParam)
{
	CLogDlg * pThis = (CLogDlg*)pParam;
	pThis->QueryIPInfoWork();
}

#pragma warning( disable : 4101 )

void CLogDlg::QueryIPInfoWork()
{
	PPC_NAME_ITEM			NameItem = NULL;
	PPC_NAME_ITEM			TempItem = NULL,EnumItem = NULL,BeforeItem = NULL;
	BOOL					FirstItem = TRUE;
	PPC_NAME_ITEM			KnownItem = NULL;

	DWORD					dwCurrentTime;

	UCHAR					szAddress[4];
	struct hostent *		host = NULL;
	TCHAR					szHostName[MAX_PATH];

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,2);
	WSAStartup( wVersionRequested, &wsaData );

	while(true)
	{
		WaitForSingleObject (m_h_query_name_event,INFINITE );

		EnterCriticalSection(&m_unknown_section);
		{
			NameItem = m_unknown_names;
		}
		LeaveCriticalSection(&m_unknown_section);

		dwCurrentTime = GetTickCount();

		while(NameItem)
		{
			if(	NameItem->LastQueryTime != 0					||
				(dwCurrentTime - NameItem->LastQueryTime) < 500	)  // 0.5 秒查询间隔
			{
				NameItem = NameItem->Next;
				continue;
			}

			memcpy(szAddress,&NameItem->IPAddress,4);

			host = gethostbyaddr((char*)szAddress,4,AF_INET);
			
			if(host && host->h_name)
			{
				KnownItem = (PPC_NAME_ITEM)malloc(sizeof(PC_NAME_ITEM));
				KnownItem->IPAddress  = NameItem->IPAddress;

#ifdef _MBCS
				_tcscpy_s(KnownItem->szHostName,host->h_name);
#else
				MultiByteToWideChar(CP_ACP,0,host->h_name,-1,KnownItem->szHostName,MAX_PATH);
#endif
				
				//将成功查询完成的Item 插入到已知链表
				EnterCriticalSection(&m_known_section);
				{
					KnownItem->Next = m_known_names;
					m_known_names   = KnownItem;
				}
				LeaveCriticalSection(&m_known_section);

				TempItem  = NameItem;
				NameItem  = NameItem->Next;
				EnterCriticalSection(&m_unknown_section);
				{
					EnumItem  = m_unknown_names;
					FirstItem = TRUE;
					while(EnumItem)
					{
						if(EnumItem->IPAddress == TempItem->IPAddress)
						{
							break;
						}
						else
						{
							FirstItem  = FALSE;
							BeforeItem = EnumItem;
						}

						EnumItem = EnumItem->Next;
					}

					if(EnumItem)
					{
						if(FirstItem)
						{
							m_unknown_names = m_unknown_names->Next;
						}
						else
						{
							BeforeItem->Next = EnumItem->Next;
						}
						
						free(EnumItem);
						EnumItem = NULL;
					}
				}
				LeaveCriticalSection(&m_unknown_section);
			}
			else
			{
				NameItem->LastQueryTime = dwCurrentTime;
				NameItem = NameItem->Next;
			}
		}

		ResetEvent(m_h_query_name_event);
	}
}

#pragma warning( default : 4101 )

void CLogDlg::InsertLog(TCHAR*	LogTime,
						TCHAR*	AttachType,
						ULONG	IPAddress,
						TCHAR*	AttachSourceMac,
						ULONG	ulAttachCount,
						BOOL	bSend,
						BOOL	bKnownIP)
{
	TCHAR logcount[MAX_PATH];

	int  iItem      = -1;

	PPC_NAME_ITEM	NameItem = NULL;
	BOOL			FindName = FALSE;
	BOOL			InUnknownList = FALSE;
	TCHAR			AttachSourceIP[MAX_PATH];
	TCHAR			AttachPCName[MAX_PATH];
	UCHAR			szIPAddress[4];

	TCHAR			szEnumListMac[MAX_PATH];
	TCHAR			szEnumAttachType[MAX_PATH];
	TCHAR			szEnumIPAddress[MAX_PATH];
	TCHAR			szEnumHostName[MAX_PATH];
	TCHAR			szEnumAttachCount[MAX_PATH];
	int				i;
	int				iCurrentListCount = m_log_list.GetItemCount();
	int				iOldAttachCount;

	BOOL			bHaveSameRecord   = FALSE;
	BOOL			bOldRecordKnownIP = FALSE;
	BOOL			bOldRecordKnownHostName = FALSE;

	//检测当前的Mac地址是否已经在列表中

	if(iCurrentListCount)
	{
		for(i = 0 ; i< iCurrentListCount ; i++)
		{
			//先匹配Mac地址
			if(m_log_list.GetItemText(i,4,szEnumListMac,MAX_PATH))
			{
				if(_tcscmp(szEnumListMac,AttachSourceMac) == 0)
				{
					//攻击类型相同
					if(m_log_list.GetItemText(i,1,szEnumAttachType,MAX_PATH))
					{
						if(_tcscmp(szEnumAttachType,AttachType) == 0)
						{
							bHaveSameRecord = TRUE;

							if(m_log_list.GetItemText(i,3,szEnumIPAddress,MAX_PATH))
							{
								if(_tcscmp(szEnumIPAddress,TEXT("未知IP")))
								{
									bOldRecordKnownIP = TRUE;
								}
							}

							if(m_log_list.GetItemText(i,2,szEnumHostName,MAX_PATH))
							{
								if(_tcscmp(szEnumHostName,TEXT("未知主机")))
								{
									bOldRecordKnownHostName = TRUE;
								}
							}

							break;
						}
					}
				}
			}
		}
	}

	if(bHaveSameRecord)
	{
		if(bOldRecordKnownHostName)
		{
			if(m_log_list.GetItemText(i,5,szEnumAttachCount,MAX_PATH))
			{
#ifdef _MBCS
				iOldAttachCount = atoi(szEnumAttachCount);
#else
				iOldAttachCount = WstrToInt(szEnumAttachCount);
#endif

				iOldAttachCount ++;
				wsprintf(szEnumAttachCount,TEXT("%d"),iOldAttachCount);

				m_log_list.SetItemText(i,5,szEnumAttachCount);

				if(!bSend)
				{
					m_recv_count ++;
					wsprintf(logcount,TEXT("%d"),m_recv_count);
					g_state_dlg.m_extern.SetWindowText(logcount);
				}
				else 
				{
					m_send_count ++;
					wsprintf(logcount,TEXT("%d"),m_send_count);
					g_state_dlg.m_internal.SetWindowText(logcount);
				}

				//更新状态页
				g_state_dlg.m_last_time.SetWindowText(LogTime);

				g_state_dlg.m_last_ip.SetWindowText(szEnumIPAddress);
				g_state_dlg.m_last_pc.SetWindowText(szEnumHostName);
				g_state_dlg.m_last_mac.SetWindowText(szEnumListMac);

			}
		}
		else if(bOldRecordKnownIP && bKnownIP)
		{
			m_log_list.GetItemText(i,5,szEnumAttachCount,MAX_PATH);

#ifdef _MBCS
			iOldAttachCount = atoi(szEnumAttachCount);
#else
			iOldAttachCount = WstrToInt(szEnumAttachCount);
#endif
			wsprintf(szEnumAttachCount,TEXT("%d"),++iOldAttachCount);

			m_log_list.SetItemText(i,5,szEnumAttachCount);

			EnterCriticalSection(&m_known_section);
			{
				NameItem = m_known_names;
				while(NameItem)
				{
					if(NameItem->IPAddress == IPAddress)
					{
						_tcscpy_s(AttachPCName,NameItem->szHostName);
						FindName = TRUE;
						break;
					}
					NameItem = NameItem->Next;
				}
			}
			LeaveCriticalSection(&m_known_section);

			// 接收到攻击包
			if(!bSend)
			{
				m_recv_count ++;
				wsprintf(logcount,TEXT("%d"),m_recv_count);
				g_state_dlg.m_extern.SetWindowText(logcount);
			}
			else //发送攻击包
			{
				m_send_count ++;
				wsprintf(logcount,TEXT("%d"),m_send_count);
				g_state_dlg.m_internal.SetWindowText(logcount);
			}

			if(FindName)
			{
				m_log_list.SetItemText(i,2,AttachPCName); // 更新主机名
			}

			//更新状态页
			g_state_dlg.m_last_time.SetWindowText(LogTime);

			g_state_dlg.m_last_ip.SetWindowText(szEnumIPAddress);
			g_state_dlg.m_last_pc.SetWindowText(AttachPCName);
			g_state_dlg.m_last_mac.SetWindowText(szEnumListMac);

		}
		else if(bKnownIP)
		{
			m_log_list.GetItemText(i,5,szEnumAttachCount,MAX_PATH);

#ifdef _MBCS
			iOldAttachCount = atoi(szEnumAttachCount);
#else
			iOldAttachCount = WstrToInt(szEnumAttachCount);
#endif
			wsprintf(szEnumAttachCount,TEXT("%d"),++iOldAttachCount);

			m_log_list.SetItemText(i,5,szEnumAttachCount);
			
			memcpy(szIPAddress,&IPAddress,4);
			wsprintf(AttachSourceIP,TEXT("%d.%d.%d.%d"),szIPAddress[0],szIPAddress[1],szIPAddress[2],szIPAddress[3]);
			
			m_log_list.SetItemText(i,3,AttachSourceIP);

			//必须放入查询主机名称队列
			EnterCriticalSection(&m_unknown_section);
			{
				NameItem = m_unknown_names;
				while(NameItem)
				{
					if(NameItem->IPAddress == IPAddress)
					{
						InUnknownList = TRUE;
						break;
					}
					NameItem = NameItem->Next;
				}
				
				if(!InUnknownList)
				{
					NameItem = (PPC_NAME_ITEM)malloc(sizeof(PC_NAME_ITEM));

					memset(NameItem,0,sizeof(PC_NAME_ITEM));
					NameItem->IPAddress = IPAddress;
					NameItem->Next      = m_unknown_names;
					m_unknown_names     = NameItem;
				}
			}
			LeaveCriticalSection(&m_unknown_section);

			if(!InUnknownList)
			{
				SetEvent(m_h_query_name_event);
			}

			//更新状态页
			g_state_dlg.m_last_time.SetWindowText(LogTime);
			g_state_dlg.m_last_ip.SetWindowText(AttachSourceIP);
			g_state_dlg.m_last_pc.SetWindowText(TEXT("未知主机"));
			g_state_dlg.m_last_mac.SetWindowText(szEnumListMac);

		}
		else
		{
			m_log_list.GetItemText(i,5,szEnumAttachCount,MAX_PATH);

#ifdef _MBCS
			iOldAttachCount = atoi(szEnumAttachCount);
#else
			iOldAttachCount = WstrToInt(szEnumAttachCount);
#endif
			wsprintf(szEnumAttachCount,TEXT("%d"),++iOldAttachCount);

			m_log_list.SetItemText(i,5,szEnumAttachCount);

			// 接收到攻击包
			if(!bSend)
			{
				m_recv_count ++;
				wsprintf(logcount,TEXT("%d"),m_recv_count);
				g_state_dlg.m_extern.SetWindowText(logcount);
			}
			else //发送攻击包
			{
				m_send_count ++;
				wsprintf(logcount,TEXT("%d"),m_send_count);
				g_state_dlg.m_internal.SetWindowText(logcount);
			}
			
			//更新状态页
			g_state_dlg.m_last_time.SetWindowText(LogTime);

			g_state_dlg.m_last_ip.SetWindowText(TEXT("未知IP"));
			g_state_dlg.m_last_pc.SetWindowText(TEXT("未知主机"));
			g_state_dlg.m_last_mac.SetWindowText(szEnumListMac);
		}

		return ;
	}

	m_record_count ++;

	iItem = m_log_list.InsertItem(m_record_count,LogTime);

	if(iItem == -1) return;

	m_log_list.SetItemText(iItem,5,TEXT("1"));

	if(bKnownIP)
	{
		EnterCriticalSection(&m_known_section);
		{
			NameItem = m_known_names;
			while(NameItem)
			{
				if(NameItem->IPAddress == IPAddress)
				{
					_tcscpy_s(AttachPCName,NameItem->szHostName);
					FindName = TRUE;
					break;
				}
				NameItem = NameItem->Next;
			}
		}
		LeaveCriticalSection(&m_known_section);
		
		if(!FindName)
		{
			EnterCriticalSection(&m_unknown_section);
			{
				NameItem = m_unknown_names;
				while(NameItem)
				{
					if(NameItem->IPAddress == IPAddress)
					{
						InUnknownList = TRUE;
						break;
					}
					NameItem = NameItem->Next;
				}
				
				if(!InUnknownList)
				{
					NameItem = (PPC_NAME_ITEM)malloc(sizeof(PC_NAME_ITEM));

					memset(NameItem,0,sizeof(PC_NAME_ITEM));
					NameItem->IPAddress = IPAddress;
					NameItem->Next      = m_unknown_names;
					m_unknown_names     = NameItem;
				}
			}
			LeaveCriticalSection(&m_unknown_section);

			if(!InUnknownList)
			{
				SetEvent(m_h_query_name_event);
			}

			memcpy(szIPAddress,&IPAddress,4);
			wsprintf(AttachSourceIP,TEXT("%d.%d.%d.%d"),szIPAddress[0],szIPAddress[1],szIPAddress[2],szIPAddress[3]);
							
			m_log_list.SetItemText(iItem,1,AttachType);
			m_log_list.SetItemText(iItem,2,TEXT("未知主机"));
			m_log_list.SetItemText(iItem,3,AttachSourceIP);
			m_log_list.SetItemText(iItem,4,AttachSourceMac);

			//状态页内容更新
			g_state_dlg.m_last_time.SetWindowText(LogTime);
			g_state_dlg.m_last_mac.SetWindowText(AttachSourceMac);
			g_state_dlg.m_last_ip.SetWindowText(AttachSourceIP);
			g_state_dlg.m_last_pc.SetWindowText(TEXT("未知主机"));
		}
		else
		{
			memcpy(szIPAddress,&IPAddress,4);
			wsprintf(AttachSourceIP,TEXT("%d.%d.%d.%d"),szIPAddress[0],szIPAddress[1],szIPAddress[2],szIPAddress[3]);
							
			m_log_list.SetItemText(iItem,1,AttachType);
			m_log_list.SetItemText(iItem,2,AttachPCName);
			m_log_list.SetItemText(iItem,3,AttachSourceIP);
			m_log_list.SetItemText(iItem,4,AttachSourceMac);

			//状态页内容更新
			g_state_dlg.m_last_time.SetWindowText(LogTime);
			g_state_dlg.m_last_mac.SetWindowText(AttachSourceMac);
			g_state_dlg.m_last_ip.SetWindowText(AttachSourceIP);
			g_state_dlg.m_last_pc.SetWindowText(AttachPCName);
		}

		// 接收到攻击包
		if(!bSend)
		{
			m_recv_count ++;
			wsprintf(logcount,TEXT("%d"),m_recv_count);
			g_state_dlg.m_extern.SetWindowText(logcount);
		}
		else //发送攻击包
		{
			m_send_count ++;
			wsprintf(logcount,TEXT("%d"),m_send_count);
			g_state_dlg.m_internal.SetWindowText(logcount);
		}
	}
	else
	{
		m_log_list.SetItemText(iItem,1,AttachType);
		m_log_list.SetItemText(iItem,2,TEXT("未知主机"));
		m_log_list.SetItemText(iItem,3,TEXT("未知IP"));
		m_log_list.SetItemText(iItem,4,AttachSourceMac);

		//状态页内容更新
		g_state_dlg.m_last_time.SetWindowText(LogTime);
		g_state_dlg.m_last_mac.SetWindowText(AttachSourceMac);
		g_state_dlg.m_last_ip.SetWindowText(TEXT("未知IP"));
		g_state_dlg.m_last_pc.SetWindowText(TEXT("未知主机"));

		// 接收到攻击包
		if(!bSend)
		{
			m_recv_count ++;
			wsprintf(logcount,TEXT("%d"),m_recv_count);
			g_state_dlg.m_extern.SetWindowText(logcount);
		}
		else //发送攻击包
		{
			m_send_count ++;
			wsprintf(logcount,TEXT("%d"),m_send_count);
			g_state_dlg.m_internal.SetWindowText(logcount);
		}
	}

	_stprintf_s(logcount,MAX_PATH,TEXT("%d"),m_record_count);
	m_logview_count.SetWindowText(logcount);

}


void CLogDlg::OnBnClickedLookLogFile()
{
	// TODO: 在此添加控件通知处理程序代码
}
