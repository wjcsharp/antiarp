
#ifndef _SHARE_STRUCT_H
#define _SHARE_STRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif

// 最多记录RARP/ARP回应包
#define		MAX_REPLY_RECORD					100

//用于记录局域网的所有机器对网关地址的查询操作的记录，用于检索是谁在攻击局域网
#define		MAX_IP_MAC_ITEM_COUNT				1024

enum ATTACH_TYPE
{
	ATTACH_NONE,			   // 无攻击行为
	GATEWAY_ARP_QUERY_ATTACH,  // 伪造网关查询包攻击
	GATEWAY_ARP_REPLY_ATTACH,  // 伪造网关回应包攻击
	LAN_SAMEIP_ATTACH,		   // 局域网IP冲突攻击
	WAN_SAMEIP_ATTACH,		   // 广域网IP冲突攻击
	WRONG_PROTOCOL_ATTACH,	   // 错误的协议攻击
	UNKNOWN_ATTACH,

};

//#pragma pack ( push , 1 )

typedef struct _ARP_PACKET
{
    UCHAR       DstAddr[6];
    UCHAR       SrcAddr[6];
    USHORT      EthType;				// 以太网协议 ARP

	//ARP Packet
	USHORT		HardwareType;			// 0x1				// 2
	USHORT		ProtocolType;			// ntohs(0x800)		// 4

	UCHAR		HardwareSize;			// 6 Mac 地址长度	// 5
	UCHAR		ProtocolSize;			// 4 IPV4长度		// 6

	USHORT		OperateCode;			// 1 Query  , 2 Repley	//	8

	UCHAR		SourceMacAddress[6];	// 发出包的物理地址		//	14
	UCHAR		SourceIPAddress[4];		// 发出包的源地址		//	18

	UCHAR		DestMacAddress[6];		// 目的地址(Query 操作时全为0,Repley时为请求的物理地址	// 24
	UCHAR		DestIPAddress[4];		// 目的IP地址			//  28

} ARP_PACKET;

typedef struct _NOTIFY_PACKET
{
	struct _NOTIFY_PACKET*	Next;				// 下一个包
	PVOID				pAdapt;					// 是哪个张网卡的包
	BOOLEAN				SendPacket;				// 是否是本机发送出去的包
	BOOLEAN				WanPacket;				// 是否是Wan口包
	enum ATTACH_TYPE	AttachType;				// 攻击类型
	ULONG				AttachCount;			// 被攻击的次数

	// ARP 包数据
    ARP_PACKET			ArpPacket;

} NOTIFY_PACKET;

typedef struct _IP_ITEM
{
	struct _IP_ITEM*	Next;
	ULONG				RecordCount;	// 在等待的时间内收到 Reply报文的个数
	ULONG				QueryCount;		// 在等待的时间内系统发送 Query报文的个数,用于伪造ARP网关检测用
	BOOLEAN				WanAddress;		// 是否是广域网地址
	BOOLEAN				Gateway;		// 是否是网关地址
	UCHAR				IPAddress[4];	// 网络序
	UCHAR				MacAddress[6];	// Mac Address

} IP_ITEM;

typedef struct _REPLY_RECORD
{
	ULONG		ulQueryCount;	//在等待的时间内，执行了几个查询包操作
	ULONG		ulItemCount;
	IP_ITEM		Items[MAX_REPLY_RECORD];
} REPLY_RECORD,*PREPLY_RECORD;

typedef struct _IP_MAC_ITEM
{
	UCHAR				IPAddress[4];	// 网络序
	UCHAR				MacAddress[6];	// Mac Address
} IP_MAC_ITEM;

typedef struct _ARPFW_SHARE_MEM
{
	HANDLE			NotifyEvent;
	ULONG			ulItemCount;
	IP_MAC_ITEM		Items[MAX_IP_MAC_ITEM_COUNT];
	NOTIFY_PACKET	NotifyPacket;
	//执行查询时我们从内核自动识别出来的网关MAC地址
	REPLY_RECORD	Replay;
} ARPFW_SHARE_MEM,*PARPFW_SHARE_MEM;

//#pragma pack ( pop , 1 )

#endif

