/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#ifndef _PROTOCOL5_H
#define _PROTOCOL5_H
#if _MSC_VER > 1000
#pragma once
#endif

VOID
ProtocolBindAdapter5(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    );

VOID
ProtocolSendComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PNDIS_PACKET           Packet,
    IN  NDIS_STATUS            Status
    );

VOID
ProtocolOpenAdapterComplete5(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    );

VOID
ProtocolUnbindAdapter5(
    OUT PNDIS_STATUS		   Status,
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_HANDLE            UnbindContext
    );

VOID
ProtocolUnloadProtocol5(
    VOID
	);

VOID
ProtocolCloseAdapterComplete5(
    IN    NDIS_HANDLE            ProtocolBindingContext,
    IN    NDIS_STATUS            Status
    );

VOID
ProtocolResetComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_STATUS            Status
    );

VOID
ProtocolRequestComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PVOID				   Request, //PNDIS_REQUEST50		   NdisRequest,
    IN  NDIS_STATUS            Status
    );

VOID
ProtocolStatus5(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  UINT                StatusBufferSize
    );

VOID
ProtocolStatusComplete5(
    IN NDIS_HANDLE            ProtocolBindingContext
    );

NDIS_STATUS
ProtocolPNPHandler5(
    IN NDIS_HANDLE        ProtocolBindingContext,
    IN PNET_PNP_EVENT     pNetPnPEvent
    );

NDIS_STATUS
ProtocolPnPNetEventReconfigure5(
    IN PADAPTER            pAdapt,
    IN PNET_PNP_EVENT      pNetPnPEvent
    );

NDIS_STATUS
ProtocolPnPNetEventSetPower5(
    IN PADAPTER            pAdapt,
    IN PNET_PNP_EVENT      pNetPnPEvent
    );

VOID
ProtocolSendComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PNDIS_PACKET           Packet,
    IN  NDIS_STATUS            Status
    );

VOID
ProtocolTransferDataComplete5(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        Packet,
    IN  NDIS_STATUS         Status,
    IN  UINT                BytesTransferred
    );

VOID
ProtocolQueueReceivedPacket5(
    IN PADAPTER				pAdapt,
    IN PNDIS_PACKET			pRecvedPacket,
    IN BOOLEAN				DoIndicate,
	IN BOOLEAN				bReturnPacket
    );

VOID
ProtocolFlushReceiveQueue5(
    IN PADAPTER         pAdapt
    );

VOID
ProtocolReceiveComplete5(
    IN NDIS_HANDLE        ProtocolBindingContext
    );

NDIS_STATUS
ProtocolReceive5(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         MacReceiveContext,
    IN  PVOID               HeaderBuffer,
    IN  UINT                HeaderBufferSize,
    IN  PVOID               LookAheadBuffer,
    IN  UINT                LookAheadBufferSize,
    IN  UINT                PacketSize
    );

INT
ProtocolReceivePacket5(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet
    );

#endif
