/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop

// 网关欺骗型攻击
BOOLEAN			g_EnableGatewayCheck		= FALSE;

// IP 冲突攻击
BOOLEAN			g_EnableSameIPCheck			= FALSE;

BOOLEAN			g_EnableSendCheck			= FALSE;

//以下链表需要在驱动退出的时候，检测并释放内存
GATEWAY_ITEM*	g_Gateway_List				= NULL;
LAN_ITEM*		g_Lan_List					= NULL;
WAN_ITEM*		g_Wan_List					= NULL;


static UCHAR	Empty_MacAddress[6] = {0,0,0,0,0,0};

/************************************************************************************



************************************************************************************/

BOOLEAN
InitArpCheck()
{
	return InitGatewayCheck();
}

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

/************************************************************************************
	// 接收报文 会受到两种攻击 

	// 1. 网关欺骗攻击(内网)

	// 2. IP地址冲突攻击
	//    a. 内网IP地址间冲突
	//    b. 内网与外网IP地址冲突
	//	  c. 外网IP地址间冲突

************************************************************************************/

RAS_OPT
CheckPacketRecvOpt(
	PADAPTER			pAdapt,
	PNDIS_PACKET		pPacket,
	BOOLEAN				bWanPacket
	)
{
	enum RAS_OPT		RetOpt	= OPT_PASS;

	UINT				PacketSize;

	ARP_PACKET*			ArpPacket = NULL;

	GATEWAY_ITEM*		Gateway	  = NULL;

	LAN_ITEM*			LanItem	  = NULL;

	WAN_ITEM*			WanItem	  = NULL;

	ULONG				i		  = 0;

	BOOLEAN				bSameRecord	= FALSE;

	enum ATTACH_TYPE	AttachType  = ATTACH_NONE;

	ArpPacket = ExAllocatePoolWithTag(NonPagedPool,MAX_ETH_PACKET_SIZE,TAG);

	if(!ArpPacket)
	{
		goto Exit;
	}

	if(ArpPacket)
	{
		CopyPacket2Buffer(pPacket,(PUCHAR)ArpPacket,&PacketSize);

		if( ArpPacket->EthType != ETHERNET_ARP || 
			PacketSize < sizeof(ARP_PACKET)		)
		{
			goto Exit;
		}

		if( ArpPacket->OperateCode != 0x100 &&
			ArpPacket->OperateCode != 0x200 &&
			ArpPacket->OperateCode != 0x300 &&
			ArpPacket->OperateCode != 0x400 )
		{
			KdPrint((" 错误ARP/RARP协议攻击"));
			AttachType = WRONG_PROTOCOL_ATTACH;
			RetOpt = OPT_DROP;
			goto Exit;
		}

		//进行 IP - Mac 对应查询表的建立
		if(g_ArpFw_ShareMem)
		{
			// 查询广播包
			if( ArpPacket->OperateCode == ARP_QUERY										&&
				NdisEqualMemory(ArpPacket->DestMacAddress,Empty_MacAddress,6)			&&
				!NdisEqualMemory(ArpPacket->SourceMacAddress,Empty_MacAddress,6)		&&
				g_ArpFw_ShareMem->ulItemCount < MAX_IP_MAC_ITEM_COUNT					)
			{

				bSameRecord = FALSE;
				for( i = 0 ; i< g_ArpFw_ShareMem->ulItemCount; i++)
				{
					if(NdisEqualMemory( g_ArpFw_ShareMem->Items[i].IPAddress,ArpPacket->SourceIPAddress,4))
					{
						bSameRecord = TRUE;
						break;
					}
				}

				//当前没有该IP地址的记录
				if(!bSameRecord)
				{
					NdisAcquireSpinLock(&GlobalLock);

					memcpy(g_ArpFw_ShareMem->Items[g_ArpFw_ShareMem->ulItemCount].IPAddress,
							ArpPacket->SourceIPAddress,4);
					memcpy(g_ArpFw_ShareMem->Items[g_ArpFw_ShareMem->ulItemCount].MacAddress,
							ArpPacket->SourceMacAddress,6);

					g_ArpFw_ShareMem->ulItemCount ++;

					NdisReleaseSpinLock(&GlobalLock);
				}
				
			}

		}


		// ARP Reply 报文记录
		if(	ArpPacket->OperateCode == ARP_REPLY									&& 
			g_bRecord_ARP_Reply													&&
			NdisEqualMemory(ArpPacket->SourceIPAddress,g_Want_ARP_Reply_IP,4)	)
		{
			bSameRecord = FALSE;

			NdisAcquireSpinLock(&GlobalLock);

			if(g_Reply_Record->ulItemCount < MAX_REPLY_RECORD)
			{
				do
				{
					if(g_Reply_Record->ulItemCount > 0)
					{
						for(i = 0 ; i < g_Reply_Record->ulItemCount; i ++)
						{
							if(NdisEqualMemory(ArpPacket->SourceMacAddress,
								g_Reply_Record->Items[i].MacAddress,6))
							{
								g_Reply_Record->Items[i].RecordCount ++;
								bSameRecord = TRUE;
								break;
							}
						}
					}

					if(!bSameRecord)
					{
						NdisMoveMemory(g_Reply_Record->Items[g_Reply_Record->ulItemCount].IPAddress,
											ArpPacket->SourceIPAddress,4);
						NdisMoveMemory(g_Reply_Record->Items[g_Reply_Record->ulItemCount].MacAddress,
											ArpPacket->SourceMacAddress,6);
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].WanAddress	= pAdapt->bWanAdapter;
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].Gateway		= TRUE;
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].Next			= (IP_ITEM*)pAdapt;
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].RecordCount	= 1;

						g_Reply_Record->ulItemCount ++;
					}
				}
				while(FALSE);
			}

			NdisReleaseSpinLock(&GlobalLock);
		}

		//检测伪造ARP/RARP Query攻击中的源Mac地址是否为正确的网关地址
		if( g_EnableGatewayCheck															&&
			(ArpPacket->OperateCode == ARP_QUERY || ArpPacket->OperateCode == RARP_QUERY )   )
		{
			// 网关地址检测,Query操作中的源地址和源MAC地址必须是正确的
			if(!bWanPacket)		// 局域网网关检测
			{
				NdisAcquireSpinLock(&GlobalLock);
				Gateway  = g_Gateway_List;
				while(Gateway)
				{
					if( NdisEqualMemory(ArpPacket->SourceIPAddress,Gateway->IPAddress,4)	&&
						!NdisEqualMemory(ArpPacket->SourceMacAddress,Gateway->MacAddress,6)  )
					{
						// IP地址相同,Mac地址不同 (禁止该包往上通行)
						KdPrint(("伪造网关Query攻击报文"));
						AttachType = GATEWAY_ARP_QUERY_ATTACH;
						RetOpt = OPT_DROP;
						NdisReleaseSpinLock(&GlobalLock);
						goto Exit;					
					}

					Gateway = Gateway->Next;
				}
				NdisReleaseSpinLock(&GlobalLock);
			}
		}

		//伪造的ARP/RARP Reply报文检测
		if(	g_EnableGatewayCheck														  &&
			(ArpPacket->OperateCode == ARP_REPLY || ArpPacket->OperateCode == RARP_REPLY) )
		{

			if(!bWanPacket)		// 局域网网关检测
			{
				NdisAcquireSpinLock(&GlobalLock);
				Gateway = g_Gateway_List;
				while(Gateway)
				{
					if(	NdisEqualMemory(Gateway->IPAddress,ArpPacket->SourceIPAddress,4)	&& // 是网关IP
						!NdisEqualMemory(Gateway->MacAddress,ArpPacket->SourceMacAddress,6)	)	// Mac 地址不相同,网关攻击
					{
						KdPrint(("伪造网关Reply攻击报文"));
						//禁止该包往上通行
						AttachType = GATEWAY_ARP_REPLY_ATTACH;
						RetOpt = OPT_DROP;
						NdisReleaseSpinLock(&GlobalLock);
						goto Exit;
					}
					else if(NdisEqualMemory(Gateway->IPAddress,ArpPacket->DestIPAddress,4)		&&
							!NdisEqualMemory(Gateway->MacAddress,ArpPacket->DestMacAddress,6)	)
					{
						KdPrint(("伪造网关Reply攻击报文"));
						//禁止该包往上通行
						RetOpt = OPT_DROP;
						AttachType = GATEWAY_ARP_REPLY_ATTACH;
						NdisReleaseSpinLock(&GlobalLock);
						goto Exit;
					}

					Gateway = Gateway->Next;
				}

				NdisReleaseSpinLock(&GlobalLock);
			}
		}
		
		//进行 IP 冲突攻击检测
		if(	g_EnableSameIPCheck														&&
			NdisEqualMemory(ArpPacket->SourceIPAddress,ArpPacket->DestIPAddress,4)	)
		{
			NdisAcquireSpinLock(&GlobalLock);
			if(!pAdapt->bWanAdapter) // 局域网检测
			{
				LanItem = g_Lan_List;
				while(LanItem)
				{
					// IP 地址相同 而 源Mac 地址不同
					if( NdisEqualMemory(ArpPacket->SourceIPAddress,LanItem->IPAddress,4) &&
						!NdisEqualMemory(ArpPacket->SourceMacAddress,LanItem->MacAddress,6) )
					{
						KdPrint(("伪造内网间IP冲突攻击报文"));
						RetOpt = OPT_DROP;
						AttachType = LAN_SAMEIP_ATTACH;
						NdisReleaseSpinLock(&GlobalLock);
						goto Exit;
					}

					LanItem = LanItem->Next;
				}
				// 局域网对外网的相同IP攻击
				WanItem = g_Wan_List;
				while(WanItem)
				{
					if(NdisEqualMemory(ArpPacket->SourceIPAddress,WanItem->IPAddress,4))
					{
						KdPrint(("伪造内外网间IP冲突攻击报文"));
						RetOpt = OPT_DROP;
						AttachType = WAN_SAMEIP_ATTACH;
						NdisReleaseSpinLock(&GlobalLock);
						goto Exit;
					}
					WanItem = WanItem->Next;
				}
			}
			NdisReleaseSpinLock(&GlobalLock);
		}

	}

Exit:
	if(ArpPacket)
	{
		if(g_ArpFw_ShareMem && AttachType != ATTACH_NONE)	
		{
			g_ArpFw_ShareMem->NotifyPacket.AttachCount = 1;
			g_ArpFw_ShareMem->NotifyPacket.AttachType  = AttachType;
			g_ArpFw_ShareMem->NotifyPacket.SendPacket  = FALSE;
			g_ArpFw_ShareMem->NotifyPacket.WanPacket   = pAdapt->bWanAdapter;

			RtlCopyMemory((PVOID)&g_ArpFw_ShareMem->NotifyPacket.ArpPacket,
							ArpPacket,sizeof(ARP_PACKET));

			SetUserShareEvent(&g_NotifyEvent);

		}

		ExFreePool(ArpPacket);
		ArpPacket = NULL;
	}

	return RetOpt;
}

/************************************************************************************





************************************************************************************/

enum RAS_OPT
OldRecvPacketOpt(
	PADAPTER				pAdapt,
    IN  PVOID               HeaderBuffer,
    IN  UINT                HeaderBufferSize,
    IN  PVOID               LookAheadBuffer,
    IN  UINT                LookAheadBufferSize,
    IN  UINT                PacketSize)
{

	enum RAS_OPT RetOpt	= OPT_PASS;

	ARP_PACKET			*ArpPacket = NULL;

	GATEWAY_ITEM*		Gateway	  = NULL;

	LAN_ITEM*			LanItem	  = NULL;

	WAN_ITEM*			WanItem	  = NULL;

	ETH_HEADER*			EthHeader = HeaderBuffer;

	ULONG				i		  = 0;

	BOOLEAN				bSameRecord	= FALSE;

	enum ATTACH_TYPE	AttachType  = ATTACH_NONE;

	//检测是否是 ARP 报文
	if( EthHeader->EthType != ETHERNET_ARP)
	{
		goto OldRecvExit;
	}

	ArpPacket = ExAllocatePoolWithTag(NonPagedPool,MAX_ETH_PACKET_SIZE,TAG);

	if(ArpPacket)
	{
		memcpy(ArpPacket,HeaderBuffer,14);
		memcpy(((PUCHAR)ArpPacket) + 14 ,LookAheadBuffer,LookAheadBufferSize);

		if( ArpPacket->OperateCode != 0x100 &&
			ArpPacket->OperateCode != 0x200 &&
			ArpPacket->OperateCode != 0x300 &&
			ArpPacket->OperateCode != 0x400 )
		{
			KdPrint((" 错误ARP/RARP协议攻击"));
			AttachType = WRONG_PROTOCOL_ATTACH;
			RetOpt = OPT_DROP;
			goto OldRecvExit;
		}

		//进行 IP - Mac 对应查询表的建立
		if(g_ArpFw_ShareMem)
		{
			// 查询广播包
			if( ArpPacket->OperateCode == ARP_QUERY										&&
				NdisEqualMemory(ArpPacket->DestMacAddress,Empty_MacAddress,6)			&&
				!NdisEqualMemory(ArpPacket->SourceMacAddress,Empty_MacAddress,6)		&&
				g_ArpFw_ShareMem->ulItemCount < MAX_IP_MAC_ITEM_COUNT					)
			{

				bSameRecord = FALSE;
				for( i = 0 ; i< g_ArpFw_ShareMem->ulItemCount; i++)
				{
					if(NdisEqualMemory( g_ArpFw_ShareMem->Items[i].IPAddress,ArpPacket->SourceIPAddress,4))
					{
						bSameRecord = TRUE;
						break;
					}
				}
	
				//当前没有该IP地址的记录
				if(!bSameRecord)
				{
					NdisAcquireSpinLock(&GlobalLock);

					memcpy(g_ArpFw_ShareMem->Items[g_ArpFw_ShareMem->ulItemCount].IPAddress,
							ArpPacket->SourceIPAddress,4);
					memcpy(g_ArpFw_ShareMem->Items[g_ArpFw_ShareMem->ulItemCount].MacAddress,
							ArpPacket->SourceMacAddress,6);

					g_ArpFw_ShareMem->ulItemCount ++;

					NdisReleaseSpinLock(&GlobalLock);
				}
				
			}
		}

		// ARP Reply 报文记录
		if(	ArpPacket->OperateCode == ARP_REPLY									&& 
			g_bRecord_ARP_Reply													&&
			NdisEqualMemory(ArpPacket->SourceIPAddress,g_Want_ARP_Reply_IP,4)	)
		{
			bSameRecord = FALSE;

			NdisAcquireSpinLock(&GlobalLock);

			if(g_Reply_Record->ulItemCount < MAX_REPLY_RECORD)
			{
				do
				{
					if(g_Reply_Record->ulItemCount > 0)
					{
						for(i = 0 ; i < g_Reply_Record->ulItemCount; i ++)
						{
							if(NdisEqualMemory(ArpPacket->SourceMacAddress,
								g_Reply_Record->Items[i].MacAddress,6))
							{
								g_Reply_Record->Items[i].RecordCount ++;
								bSameRecord = TRUE;
								break;
							}
						}
					}

					if(!bSameRecord)
					{
						NdisMoveMemory(g_Reply_Record->Items[g_Reply_Record->ulItemCount].IPAddress,
											ArpPacket->SourceIPAddress,4);
						NdisMoveMemory(g_Reply_Record->Items[g_Reply_Record->ulItemCount].MacAddress,
											ArpPacket->SourceMacAddress,6);
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].WanAddress	= pAdapt->bWanAdapter;
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].Gateway		= TRUE;
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].Next			= (IP_ITEM*)pAdapt;
						g_Reply_Record->Items[g_Reply_Record->ulItemCount].RecordCount	= 1;

						g_Reply_Record->ulItemCount ++;
					}
				}
				while(FALSE);
			}

			NdisReleaseSpinLock(&GlobalLock);
		}

		//检测伪造ARP/RARP Query攻击中的源Mac地址是否为正确的网关地址
		if( g_EnableGatewayCheck															&&
			(ArpPacket->OperateCode == ARP_QUERY || ArpPacket->OperateCode == RARP_QUERY )   )
		{
			// 网关地址检测,Query操作中的源地址和源MAC地址必须是正确的
			if(!pAdapt->bWanAdapter)		// 局域网网关检测
			{
				NdisAcquireSpinLock(&GlobalLock);
				Gateway  = g_Gateway_List;
				while(Gateway)
				{
					if( NdisEqualMemory(ArpPacket->SourceIPAddress,Gateway->IPAddress,4)	&&
						!NdisEqualMemory(ArpPacket->SourceMacAddress,Gateway->MacAddress,6)  )
					{
						// IP地址相同,Mac地址不同 (禁止该包往上通行)
						KdPrint(("伪造网关Query攻击报文"));
						AttachType = GATEWAY_ARP_QUERY_ATTACH;
						RetOpt = OPT_DROP;
						NdisReleaseSpinLock(&GlobalLock);
						goto OldRecvExit;					
					}

					Gateway = Gateway->Next;
				}
				NdisReleaseSpinLock(&GlobalLock);
			}
		}

		//伪造的ARP/RARP Reply报文检测
		if(	g_EnableGatewayCheck														  &&
			(ArpPacket->OperateCode == ARP_REPLY || ArpPacket->OperateCode == RARP_REPLY) )
		{

			if(!pAdapt->bWanAdapter)		// 局域网网关检测
			{
				NdisAcquireSpinLock(&GlobalLock);
				Gateway = g_Gateway_List;
				while(Gateway)
				{
					if(	NdisEqualMemory(Gateway->IPAddress,ArpPacket->SourceIPAddress,4)	&& // 是网关IP
						!NdisEqualMemory(Gateway->MacAddress,ArpPacket->SourceMacAddress,6)	)	// Mac 地址不相同,网关攻击
					{
						KdPrint(("伪造网关Reply攻击报文"));
						//禁止该包往上通行
						AttachType = GATEWAY_ARP_REPLY_ATTACH;
						RetOpt = OPT_DROP;
						NdisReleaseSpinLock(&GlobalLock);
						goto OldRecvExit;
					}
					else if(NdisEqualMemory(Gateway->IPAddress,ArpPacket->DestIPAddress,4)		&&
							!NdisEqualMemory(Gateway->MacAddress,ArpPacket->DestMacAddress,6)	)
					{
						KdPrint(("伪造网关Reply攻击报文"));
						//禁止该包往上通行
						RetOpt = OPT_DROP;
						AttachType = GATEWAY_ARP_REPLY_ATTACH;
						NdisReleaseSpinLock(&GlobalLock);
						goto OldRecvExit;
					}

					Gateway = Gateway->Next;
				}

				NdisReleaseSpinLock(&GlobalLock);
			}
		}
		
		//进行 IP 冲突攻击检测
		if(	g_EnableSameIPCheck														&&
			NdisEqualMemory(ArpPacket->SourceIPAddress,ArpPacket->DestIPAddress,4)	)
		{
			NdisAcquireSpinLock(&GlobalLock);
			if(!pAdapt->bWanAdapter) // 局域网检测
			{
				LanItem = g_Lan_List;
				while(LanItem)
				{
					// IP 地址相同 而 源Mac 地址不同
					if( NdisEqualMemory(ArpPacket->SourceIPAddress,LanItem->IPAddress,4) &&
						!NdisEqualMemory(ArpPacket->SourceMacAddress,LanItem->MacAddress,6) )
					{
						KdPrint(("伪造内网间IP冲突攻击报文"));
						RetOpt = OPT_DROP;
						AttachType = LAN_SAMEIP_ATTACH;
						NdisReleaseSpinLock(&GlobalLock);
						goto OldRecvExit;
					}

					LanItem = LanItem->Next;
				}
				// 局域网对外网的相同IP攻击
				WanItem = g_Wan_List;
				while(WanItem)
				{
					if(NdisEqualMemory(ArpPacket->SourceIPAddress,WanItem->IPAddress,4))
					{
						KdPrint(("伪造内外网间IP冲突攻击报文"));
						RetOpt = OPT_DROP;
						AttachType = WAN_SAMEIP_ATTACH;
						NdisReleaseSpinLock(&GlobalLock);
						goto OldRecvExit;
					}
					WanItem = WanItem->Next;
				}
			}
			NdisReleaseSpinLock(&GlobalLock);
		}

	}

OldRecvExit:
	if(ArpPacket)
	{
		if(g_ArpFw_ShareMem && AttachType != ATTACH_NONE)	
		{
			g_ArpFw_ShareMem->NotifyPacket.AttachCount = 1;
			g_ArpFw_ShareMem->NotifyPacket.AttachType  = AttachType;
			g_ArpFw_ShareMem->NotifyPacket.SendPacket  = FALSE;
			g_ArpFw_ShareMem->NotifyPacket.WanPacket   = pAdapt->bWanAdapter;

			RtlCopyMemory((PVOID)&g_ArpFw_ShareMem->NotifyPacket.ArpPacket,
							ArpPacket,sizeof(ARP_PACKET));

			SetUserShareEvent(&g_NotifyEvent);

		}

		ExFreePool(ArpPacket);
		ArpPacket = NULL;
	}


	return RetOpt;

}


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
		g_Reply_Record->ulQueryCount ++;
		BeginCheckGateway();
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

/**************************************************************************************




**************************************************************************************/

VOID CopyPacket2Buffer(
		IN PNDIS_PACKET		pPacket,
		IN OUT PUCHAR		pBuff,
		IN OUT PUINT		pLength) 
{ 
	PNDIS_BUFFER BuffDT;
	PUCHAR BuffVA;
	UINT BuffLen;	
	*pLength = 0;
	BuffLen=0; 

	NdisQueryPacket(pPacket,NULL,NULL,&BuffDT,NULL); 

	while(BuffDT!=(PNDIS_BUFFER)NULL) 
	{ 
		NdisQueryBufferSafe(BuffDT,&BuffVA,&BuffLen,NormalPagePriority);   
		memcpy(pBuff,BuffVA,BuffLen);   
		pBuff		=	pBuff+BuffLen;   
		*pLength	+=	BuffLen;
		NdisGetNextBuffer(BuffDT,&BuffDT); 
	} 

	return; 
} 


/***********************************************************************************************


************************************************************************************************/

void MyFreeNdisSendPacket(PNDIS_PACKET p)
{
	PNDIS_BUFFER b;

	while(1) 
	{
		NdisUnchainBufferAtFront(p, &b);
		if(b != NULL) 
		{
			NdisFreeBuffer(b);
		} else 
		{
			break;
		}
	}
	NdisFreePacket(p);
}

UINT GetPacketSize(PNDIS_PACKET p)
{
	UINT	PacketSize  = 0;
	if(p)
	{
		NdisQueryPacket(p,NULL,NULL,NULL,&PacketSize);
	}
	return PacketSize;
}