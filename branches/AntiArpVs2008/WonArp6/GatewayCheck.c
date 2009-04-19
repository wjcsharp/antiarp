
#include "precomp.h"
#pragma hdrstop

//执行RARP/ARP查询时记录回应包的内容
BOOLEAN			g_bRecord_ARP_Reply		= FALSE;

PREPLY_RECORD	g_Reply_Record			= NULL;

UCHAR			g_Want_ARP_Reply_IP[4];

NDIS_TIMER		g_Check_Gateway_Mac_Timer;
BOOLEAN			g_Set_Gateway_Check_Timer = FALSE;

/************************************************************************************



************************************************************************************/

VOID 
WaitGatewayResponseTimerCallback(
    IN PVOID  SystemSpecific1,
    IN PVOID  FunctionContext,
    IN PVOID  SystemSpecific2,
    IN PVOID  SystemSpecific3)
{

	NdisAcquireSpinLock(&GlobalLock);
	{
		if(g_Set_Gateway_Check_Timer)
		{
			g_bRecord_ARP_Reply = FALSE;
			memset(g_Want_ARP_Reply_IP,0,4);

			//通知Win32接收缓冲区数据
			SetUserShareEvent(&g_NotifyEvent);
			g_Set_Gateway_Check_Timer = FALSE;
		}
	}
	NdisReleaseSpinLock(&GlobalLock);
}

/************************************************************************************



************************************************************************************/

BOOLEAN
InitGatewayCheck()
{
	g_Set_Gateway_Check_Timer = FALSE;

	/*
	g_Reply_Record = ExAllocatePoolWithTag(NonPagedPool,
									MAX_REPLY_RECORD*sizeof(IP_ITEM),TAG);

	if(!g_Reply_Record)
	{
		return FALSE;
	}
	*/

	if(!g_ArpFw_ShareMem)
	{
		return FALSE;
	}

	g_Reply_Record = (PREPLY_RECORD)&g_ArpFw_ShareMem->Replay;

	NdisInitializeTimer(&g_Check_Gateway_Mac_Timer,
			WaitGatewayResponseTimerCallback,g_Reply_Record);

	return TRUE;
}

/************************************************************************************


************************************************************************************/

VOID
UninitGatewayCheck()
{
	BOOLEAN	  bCancel = FALSE;

	g_bRecord_ARP_Reply = FALSE;

	if(g_Set_Gateway_Check_Timer)
	{
		NdisCancelTimer(&g_Check_Gateway_Mac_Timer,&bCancel);
	}

}

VOID
BeginCheckGateway()
{
	if(!g_Set_Gateway_Check_Timer)
	{
		g_Set_Gateway_Check_Timer = TRUE;
		NdisSetTimer(&g_Check_Gateway_Mac_Timer,2000);
	}
}
