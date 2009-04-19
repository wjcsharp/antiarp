/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#define BUFFER_POOL_SIZE					100


typedef struct _RECEIVED_PACKET
{
	PNDIS_PACKET					pNdisPacket;
	BOOLEAN							bReturnPacket;	//表示pNdisPacket是否需要执行ReturnPacket操作
} RECEIVED_PACKET,*PRECEIVED_PACKET;

typedef struct _RECV_RSVD
{
	PNDIS_PACKET	OriginalPkt;
	PUCHAR			PacketMem;
	ULONG			PacketMemLen;
} RECV_RSVD, *PRECV_RSVD;

typedef struct _SEND_RSVD
{
	PNDIS_PACKET		 OriginalPkt;
	PUCHAR				 PacketMem;
} SEND_RSVD, *PSEND_RSVD;

#define MAX_ETH_PACKET_SIZE		1600

#define		ETHERNET_ARP	0x608		// 本机序列

#define		ARP_QUERY		0x100		// 本机序列
#define		ARP_REPLY		0x200		// 本机序列

#define		RARP_QUERY		0x300		// 本机序列
#define		RARP_REPLY		0x400		// 本机序列

typedef struct _ETH_HEADER
{
    UCHAR       DstAddr[6];
    UCHAR       SrcAddr[6];
    USHORT      EthType;				// 以太网协议 ARP
} ETH_HEADER;


// 用于网关地址的守卫列表
typedef struct	_GATEWAY_ITEM
{
	struct	_GATEWAY_ITEM*		Before;
	struct	_GATEWAY_ITEM*		Next;

	UCHAR						IPAddress[4];
	UCHAR						MacAddress[6];

} GATEWAY_ITEM;


typedef struct	_LAN_ITEM
{
	struct	_LAN_ITEM*			Before;
	struct	_LAN_ITEM*			Next;

	UCHAR						IPAddress[4];
	UCHAR						MacAddress[6];

} LAN_ITEM;

typedef struct	_WAN_ITEM
{
	struct	_WAN_ITEM*			Before;
	struct	_WAN_ITEM*			Next;

	UCHAR						IPAddress[4];
	UCHAR						MacAddress[6];

} WAN_ITEM;