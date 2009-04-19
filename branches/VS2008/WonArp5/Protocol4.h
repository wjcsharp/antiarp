
/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#ifndef _PROTOCOL4_H
#define _PROTOCOL4_H
#if _MSC_VER > 1000
#pragma once
#endif

//
// Protocol 4.0 proto-types
//
extern
VOID
ProtocolOpenAdapterComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                Status,
    IN NDIS_STATUS                OpenErrorStatus
    );

extern
VOID
ProtocolCloseAdapterComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                Status
    );

extern
VOID
ProtocolResetComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                Status
    );

extern
VOID
ProtocolRequestComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PVOID		              NdisRequest, //PNDIS_REQUEST40  NdisRequest,
    IN NDIS_STATUS                Status
    );

extern
VOID
ProtocolStatus4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                GeneralStatus,
    IN PVOID                      StatusBuffer,
    IN UINT                       StatusBufferSize
    );

extern
VOID
ProtocolStatusComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext
    );

extern
VOID
ProtocolSendComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet,
    IN NDIS_STATUS                Status
    );

extern
VOID
ProtocolTransferDataComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet,
    IN NDIS_STATUS                Status,
    IN UINT                       BytesTransferred
    );

extern
NDIS_STATUS
ProtocolReceive4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_HANDLE                MacReceiveContext,
    IN PVOID                      HeaderBuffer,
    IN UINT                       HeaderBufferSize,
    IN PVOID                      LookAheadBuffer,
    IN UINT                       LookaheadBufferSize,
    IN UINT                       PacketSize
    );

extern
VOID
ProtocolReceiveComplete4(
    IN NDIS_HANDLE                ProtocolBindingContext
    );

extern
INT
ProtocolReceivePacket4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet
    );

extern
VOID
ProtocolBindAdapter4(
    OUT PNDIS_STATUS              Status,
    IN  NDIS_HANDLE               BindContext,
    IN  PNDIS_STRING              DeviceName,
    IN  PVOID                     SystemSpecific1,
    IN  PVOID                     SystemSpecific2
    );

extern
VOID
ProtocolUnbindAdapter4(
    OUT PNDIS_STATUS              Status,
    IN  NDIS_HANDLE               ProtocolBindingContext,
    IN  NDIS_HANDLE               UnbindContext
    );

extern 
NDIS_STATUS
ProtocolPNPHandler4(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNET_PNP_EVENT             pNetPnPEvent
    );


NDIS_STATUS
ProtocolPnPNetEventReconfigure4(
    IN PADAPTER          pAdapt,
    IN PNET_PNP_EVENT    pNetPnPEvent
    );    

NDIS_STATUS 
ProtocolPnPNetEventSetPower4(
    IN PADAPTER          		 pAdapt,
    IN PNET_PNP_EVENT            pNetPnPEvent
    );
    
VOID
ProtocolQueueReceivedPacket4(
    IN PADAPTER				pAdapt,
    IN PNDIS_PACKET			pRecvedPacket,
    IN BOOLEAN				DoIndicate,
	IN BOOLEAN				bReturnPacket
    );

VOID
ProtocolFlushReceiveQueue4(
    IN PADAPTER         pAdapt
    );

VOID
ProtocolUnloadProtocol4(
    VOID
    );

#endif //_PROTOCOL4_H
