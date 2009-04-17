
/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/


#ifndef _MINIPORT_H
#define _MINIPORT_H
#if _MSC_VER > 1000
#pragma once
#endif


BOOLEAN
MiniportCheckForHang5(
    IN NDIS_HANDLE  MiniportAdapterContext
    );

NDIS_STATUS
MiniportInitialize5(
    OUT PNDIS_STATUS             OpenErrorStatus,
    OUT PUINT                    SelectedMediumIndex,
    IN  PNDIS_MEDIUM             MediumArray,
    IN  UINT                     MediumArraySize,
    IN  NDIS_HANDLE              MiniportAdapterHandle,
    IN  NDIS_HANDLE              WrapperConfigurationContext
    );

VOID
MiniportSendPackets5(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PPNDIS_PACKET           PacketArray,
    IN UINT                    NumberOfPackets
    );

NDIS_STATUS
MiniportQueryInformation5(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesWritten,
    OUT PULONG                    BytesNeeded
    );

VOID
MiniportQueryPNPCapabilities5(
    IN OUT PADAPTER            pAdapt,
    OUT PNDIS_STATUS         pStatus
    );

NDIS_STATUS
MiniportSetInformation5(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN NDIS_OID                Oid,
    IN PVOID                   InformationBuffer,
    IN ULONG                   InformationBufferLength,
    OUT PULONG                 BytesRead,
    OUT PULONG                 BytesNeeded
    );

VOID
MiniportProcessSetPowerOid5(
    IN OUT PNDIS_STATUS          pNdisStatus,
    IN PADAPTER                  pAdapt,
    IN PVOID                     InformationBuffer,
    IN ULONG                     InformationBufferLength,
    OUT PULONG                   BytesRead,
    OUT PULONG                   BytesNeeded
    );

VOID
MiniportReturnPacket5(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PNDIS_PACKET            Packet
    );

NDIS_STATUS
MiniportTransferData5(
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred,
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_HANDLE              MiniportReceiveContext,
    IN UINT                     ByteOffset,
    IN UINT                     BytesToTransfer
    );

VOID
MiniportHalt5(
    IN NDIS_HANDLE                MiniportAdapterContext
    );

VOID
MiniportCancelSendPackets5(
    IN NDIS_HANDLE            MiniportAdapterContext,
    IN PVOID                  CancelId
    );

VOID
MiniportDevicePnPEvent5(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_DEVICE_PNP_EVENT    DevicePnPEvent,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength
    );

VOID
MiniportAdapterShutdown5(
    IN NDIS_HANDLE                MiniportAdapterContext
    );

VOID
MiniportFreeAllPacketPools5(
    IN PADAPTER                    pAdapt
    );


#endif
