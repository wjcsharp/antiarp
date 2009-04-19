/******************************************************************************

	NDIS Filter 6.0

	用途 : ARP 防火墙

  	支持系统 : Vista / Server 2008

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop


/************************************************************************************




************************************************************************************/

VOID RemoveAllGatewayInfo()
{
	PVOID			pVoid;
	GATEWAY_ITEM	*pGateway;

	NdisAcquireSpinLock(&GlobalLock);

	g_EnableGatewayCheck = FALSE;

	pGateway = g_Gateway_List;
	while(pGateway)
	{
		pVoid = pGateway;
		pGateway = pGateway->Next;
		ExFreePool(pVoid);
	}

	g_Gateway_List	= NULL;

	NdisReleaseSpinLock(&GlobalLock);

}

/************************************************************************************




************************************************************************************/

VOID RemoveAllLanInfo()
{
	PVOID			pVoid;
	LAN_ITEM*		pLan;

	NdisAcquireSpinLock(&GlobalLock);

	g_EnableSameIPCheck	 = FALSE;

	pLan = g_Lan_List;

	while(pLan)
	{
		pVoid	= pLan;
		pLan	= pLan->Next;
		ExFreePool(pVoid);
	}

	g_Lan_List = NULL;

	NdisReleaseSpinLock(&GlobalLock);

}

/************************************************************************************





************************************************************************************/

BOOLEAN
InitArpCheck()
{
	return InitGatewayCheck();
}

/************************************************************************************




************************************************************************************/

VOID
UnInitArpCheck()
{
	GATEWAY_ITEM	*pGateway;
	LAN_ITEM*		pLan;
	WAN_ITEM*		pWan;
	PVOID			pVoid;

	NdisAcquireSpinLock(&GlobalLock);

	//关闭所有的检测项目
	g_EnableGatewayCheck = FALSE;
	g_EnableSameIPCheck	 = FALSE;

	g_EnableSendCheck = FALSE;

	pGateway = g_Gateway_List;
	while(pGateway)
	{
		pVoid = pGateway;
		pGateway = pGateway->Next;
		ExFreePool(pVoid);
	}
	
	pLan = g_Lan_List;

	while(pLan)
	{
		pVoid	= pLan;
		pLan	= pLan->Next;
		ExFreePool(pVoid);
	}

	pWan = g_Wan_List;

	while(pWan)
	{
		pVoid	= pWan;
		pWan	= pWan->Next;
		ExFreePool(pVoid);
	}

	g_Gateway_List	= NULL;
	g_Lan_List		= NULL;
	g_Wan_List		= NULL;

	UninitGatewayCheck();

	//清理所有ARP记录包
	g_bRecord_ARP_Reply		= FALSE;

	if(g_Reply_Record)
	{
		g_Reply_Record = NULL;
	}

	NdisReleaseSpinLock(&GlobalLock);

}

/******************************************************************************************


RAS_OPT
CheckPacketSendOpt(
	PADAPTER			pAdapt,
	PNDIS_PACKET		pPacket,
	BOOLEAN				bWanPacket)
{

	enum RAS_OPT RetOpt	 = OPT_PASS;

	UINT				PacketSize;

	ARP_PACKET			*ArpPacket = NULL;

	//检测本机发送出的报文
//	if(!g_EnableSendCheck)	goto Exit;

	ArpPacket = ExAllocatePoolWithTag(NonPagedPool,MAX_ETH_PACKET_SIZE,TAG);

	if(!ArpPacket)
	{
		goto Exit;
	}

	// 检测是否是ARP Response报文,放行所有Query报文
	CopyPacket2Buffer(pPacket,(PUCHAR)ArpPacket,&PacketSize);

	if( ArpPacket->EthType != ETHERNET_ARP)
	{
		goto Exit;
	}

	if( g_bRecord_ARP_Reply						&&
		ArpPacket->OperateCode == ARP_QUERY		&&
		NdisEqualMemory(ArpPacket->DestIPAddress,g_Want_ARP_Reply_IP,4)	)
	{
		NdisAcquireSpinLock(&GlobalLock);
		g_Query_Count ++;
		NdisReleaseSpinLock(&GlobalLock);
	}

Exit:
	if(ArpPacket)
	{
		ExFreePool(ArpPacket);
		ArpPacket = NULL;
	}

	return RetOpt;
}

************************************************************************************/

