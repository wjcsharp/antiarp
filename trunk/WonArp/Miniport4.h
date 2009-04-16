/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 / XP / 2003

    作者  : 张聚长

*******************************************************************************/
#ifndef _MINIPORT4_H
#define _MINIPORT4_H
#if _MSC_VER > 1000
#pragma once
#endif


//
// Miniport 4.0 proto-types
//
NDIS_STATUS
MiniportInitialize4(
    OUT PNDIS_STATUS             OpenErrorStatus,
    OUT PUINT                    SelectedMediumIndex,
    IN PNDIS_MEDIUM              MediumArray,
    IN UINT                      MediumArraySize,
    IN NDIS_HANDLE               MiniportAdapterHandle,
    IN NDIS_HANDLE               WrapperConfigurationContext
    );

VOID
MiniportSendPackets4(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN PPNDIS_PACKET              PacketArray,
    IN UINT                       NumberOfPackets
    );

NDIS_STATUS
MiniportQueryInformation4(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesWritten,
    OUT PULONG                    BytesNeeded
    );

NDIS_STATUS
MiniportSetInformation4(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesRead,
    OUT PULONG                    BytesNeeded
    );

VOID
MiniportReturnPacket4(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN PNDIS_PACKET               Packet
    );

NDIS_STATUS
MiniportTransferData4(
    OUT PNDIS_PACKET              Packet,
    OUT PUINT                     BytesTransferred,
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_HANDLE                MiniportReceiveContext,
    IN UINT                       ByteOffset,
    IN UINT                       BytesToTransfer
    );

VOID
MiniportHalt4(
    IN NDIS_HANDLE                MiniportAdapterContext
    );


VOID
MiniportQueryPNPCapabilities4(  
    IN PADAPTER                   MiniportProtocolContext, 
    OUT PNDIS_STATUS              Status
    );


NDIS_STATUS
MiniportSetMiniportSecondary4( 
    IN PADAPTER                    Secondary, 
    IN PADAPTER                    Primary
    );

BOOLEAN
MiniportCheckForHang4(
    IN NDIS_HANDLE  MiniportAdapterContext
    );

VOID
MiniportFreeAllPacketPools4(
    IN PADAPTER                    pAdapt
    );

NDIS_STATUS 
MiniportPromoteSecondary4( 
    IN PADAPTER                    pAdapt 
    );


NDIS_STATUS 
MiniportBundleSearchAndSetSecondary4(
    IN PADAPTER                    pAdapt 
    );

VOID
MiniportProcessSetPowerOid4(
    IN OUT PNDIS_STATUS          pNdisStatus,
    IN PADAPTER                  pAdapt,
    IN PVOID                     InformationBuffer,
    IN ULONG                     InformationBufferLength,
    OUT PULONG                   BytesRead,
    OUT PULONG                   BytesNeeded
    );

#endif

