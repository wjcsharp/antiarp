
#ifndef GATEWAY_CHECK_H
#define GATEWAY_CHECK_H


BOOLEAN
InitGatewayCheck();

VOID UninitGatewayCheck();

VOID
BeginCheckGateway();

//执行ARP查询时记录回应包的内容
extern BOOLEAN			g_bRecord_ARP_Reply;

extern PREPLY_RECORD	g_Reply_Record;

//extern ULONG			g_Reply_Count;
//extern ULONG			g_Query_Count;

extern UCHAR			g_Want_ARP_Reply_IP[4];

extern NDIS_TIMER		g_Check_Gateway_Mac_Timer;
extern BOOLEAN			g_Set_Gateway_Check_Timer;


#endif // GATEWAY_CHECK_H

