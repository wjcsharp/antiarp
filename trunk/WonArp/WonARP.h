/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#ifndef _RASACSIM4_H
#define _RASACSIM4_H
#if _MSC_VER > 1000
#pragma once
#endif


#define MAX_BUNDLEID_LENGTH 50

#define TAG										'rMND'
#define WAIT_INFINITE							0

//
//  Receive packet pool bounds
//
#define MIN_RECV_PACKET_POOL_SIZE				4
#define MAX_RECV_PACKET_POOL_SIZE				40


#define MIN_SEND_PACKET_POOL_SIZE				20
#define MAX_SEND_PACKET_POOL_SIZE				400


typedef struct _ADAPTER ADAPTER, *PADAPTER;
typedef struct _NDIS_REQUEST40 NDIS_REQUEST40, *PNDIS_REQUEST40;
typedef struct _NDIS_REQUEST50 NDIS_REQUEST50, *PNDIS_REQUEST50;


extern
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT            DriverObject,
    IN PUNICODE_STRING           RegistryPath
    );
extern
VOID 
LoadDynamicFunction();


VOID
ProtocolUnload(
    IN PDRIVER_OBJECT        DriverObject
    );

NDIS_STATUS
ProtocolRegisterDevice(
    VOID
    );

NDIS_STATUS
ProtocolDeregisterDevice(
    VOID
    );


struct _MYQUERY_INFORMATION
{
  NDIS_OID    Oid;
  PVOID       InformationBuffer;
  UINT        InformationBufferLength;
  UINT        BytesWritten;
  UINT        BytesNeeded;
};

struct _MYSET_INFORMATION
{
   NDIS_OID    Oid;
   PVOID       InformationBuffer;
   UINT        InformationBufferLength;
   UINT        BytesRead;
   UINT        BytesNeeded;
};

typedef struct _NDIS_REQUEST40
{
    UCHAR               MacReserved[4*sizeof(PVOID)];
    NDIS_REQUEST_TYPE   RequestType;
    union
    {
        struct _MYQUERY_INFORMATION QUERY_INFORMATION;
        struct _MYSET_INFORMATION   SET_INFORMATION;
    } DATA;

} NDIS_REQUEST40, *PNDIS_REQUEST40;


typedef struct _NDIS_REQUEST50
{
    UCHAR               MacReserved[4*sizeof(PVOID)];
    NDIS_REQUEST_TYPE   RequestType;
    union
    {
        struct _MYQUERY_INFORMATION QUERY_INFORMATION;
        struct _MYSET_INFORMATION   SET_INFORMATION;
    } DATA;
	// NDIS 5.0 / NDIS5.1 ++
    UCHAR               NdisReserved[9*sizeof(PVOID)];
    union
    {
        UCHAR           CallMgrReserved[2*sizeof(PVOID)];
        UCHAR           ProtocolReserved[2*sizeof(PVOID)];
    };
    UCHAR               MiniportReserved[2*sizeof(PVOID)];
    // -- NDIS 5.0 / NDIS5.1

} NDIS_REQUEST50, *PNDIS_REQUEST50;

//
// Structure used by both the miniport as well as the protocol part of the intermediate driver
// to represent an adapter and its corres. lower bindings
//
typedef struct _ADAPTER
{
    struct _ADAPTER *				Next;
   
    NDIS_SPIN_LOCK					Lock;
#ifdef _DEBUG
	PFILE_OBJECT					FileObject;
#endif
    NDIS_HANDLE						BindingHandle;    // To the lower miniport

    NDIS_HANDLE						MiniportHandle;    // NDIS Handle to for miniport up-calls

    NDIS_HANDLE						SendPacketPoolHandle;

    NDIS_HANDLE						RecvPacketPoolHandle;

	NDIS_HANDLE						RecvBufferPool;

	NDIS_HANDLE						SendBufferPool;

    NDIS_MEDIUM						Medium;

	BOOLEAN							bWanAdapter;			//指示本绑定是否是Wan口

    NDIS_DEVICE_POWER_STATE			MiniportDeviceState;    // Miniport's Device State 
    
	NDIS_DEVICE_POWER_STATE			PTDeviceState;          // Protocol's Device State 
    
	ULONG							MaxFrameSize;

    NDIS_STATUS						Status;					// Open Status

    NDIS_EVENT						Event;					// Used by bind/halt for Open/Close Adapter synch.

    PULONG							BytesNeeded;

    PULONG							BytesReadOrWritten;

    BOOLEAN							IndicateRcvComplete;

	ULONG							AcceptAccessRequestAttributesLen;

	UCHAR*							AcceptAccessRequestAttributes;
    
    BOOLEAN							OutstandingRequests;		// TRUE iff a request is pending
																// at the miniport below
    BOOLEAN							QueuedRequest;				// TRUE iff a request is queued at
																// this IM miniport

    BOOLEAN							StandingBy;					// True - When the miniport or protocol is transitioning from a D0 to Standby (>D0) State
    
	BOOLEAN							UnbindingInProcess;
																// False - At all other times, - Flag is cleared after a transition to D0

	NDIS_STRING						DeviceName;					// For initializing the miniport edge
    
	NDIS_STRING						UpperBindingDevice;

	NDIS_EVENT						MiniportInitEvent;			// For blocking UnbindAdapter while
																// an IM Init is in progress.
    BOOLEAN							MiniportInitPending;		// TRUE iff IMInit in progress
    
	NDIS_STATUS						LastIndicatedStatus;		// The last indicated media status
    
	NDIS_STATUS						LatestUnIndicateStatus;		// The latest suppressed media status
    
	ULONG							OutstandingSends;

	RECEIVED_PACKET					ReceivedPackets[MAX_RECV_PACKET_POOL_SIZE];

	ULONG							ReceivedPacketCount;
    
    NDIS_REQUEST40					Request4;        // This is used to wrap a request coming down
													// to us. This exploits the fact that requests
    NDIS_REQUEST50					Request5;        // This is used to wrap a request coming down
													// are serialized down to us.
} ADAPTER, *PADAPTER;



#ifndef NDIS_PACKET_FIRST_NDIS_BUFFER
#define NDIS_PACKET_FIRST_NDIS_BUFFER(_Packet)      ((_Packet)->Private.Head)
#endif

#ifndef NDIS_PACKET_LAST_NDIS_BUFFER
#define NDIS_PACKET_LAST_NDIS_BUFFER(_Packet)       ((_Packet)->Private.Tail)
#endif

#ifndef NDIS_PACKET_VALID_COUNTS
#define NDIS_PACKET_VALID_COUNTS(_Packet)           ((_Packet)->Private.ValidCounts)
#endif

#define ADAPT_DECR_PENDING_SENDS(_pAdapt) \
    {                                         \
        NdisAcquireSpinLock(&(_pAdapt)->Lock);   \
        (_pAdapt)->OutstandingSends--;           \
        NdisReleaseSpinLock(&(_pAdapt)->Lock);   \
    }

//
// Custom Macros to be used by the RasACS driver 
//
#define IsIMDeviceStateOn(_pP)        ((_pP)->MiniportDeviceState == NdisDeviceStateD0 && (_pP)->PTDeviceState == NdisDeviceStateD0 ) 

// 函数原型定义

#ifndef NDIS51

typedef struct _NDIS_PACKET_STACK
{
    ULONG_PTR   IMReserved[2];
    ULONG_PTR   NdisReserved[4];
} NDIS_PACKET_STACK, *PNDIS_PACKET_STACK;

typedef enum _NDIS_DEVICE_PNP_EVENT
{
    NdisDevicePnPEventQueryRemoved,
    NdisDevicePnPEventRemoved,
    NdisDevicePnPEventSurpriseRemoved,
    NdisDevicePnPEventQueryStopped,
    NdisDevicePnPEventStopped,
    NdisDevicePnPEventPowerProfileChanged,
    NdisDevicePnPEventMaximum
} NDIS_DEVICE_PNP_EVENT, *PNDIS_DEVICE_PNP_EVENT;

typedef VOID
(*W_CANCEL_SEND_PACKETS_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   CancelId
    );

typedef VOID
(*W_PNP_EVENT_NOTIFY_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   DevicePnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    );

typedef VOID
(*W_MINIPORT_SHUTDOWN_HANDLER) (
    IN  NDIS_HANDLE                     MiniportAdapterContext
    );

typedef struct _NDIS51_MINIPORT_CHARACTERISTICS
{
#ifdef __cplusplus
    NDIS50_MINIPORT_CHARACTERISTICS Ndis50Chars;
#else
    NDIS50_MINIPORT_CHARACTERISTICS;
#endif
    //
    // Extensions for NDIS 5.1
    //
    W_CANCEL_SEND_PACKETS_HANDLER   CancelSendPacketsHandler;
    W_PNP_EVENT_NOTIFY_HANDLER      PnPEventNotifyHandler;
    W_MINIPORT_SHUTDOWN_HANDLER     AdapterShutdownHandler;
    PVOID                           Reserved1;
    PVOID                           Reserved2;
    PVOID                           Reserved3;
    PVOID                           Reserved4;
} NDIS51_MINIPORT_CHARACTERISTICS;


EXPORT
PVOID
NdisGetRoutineAddress(
    IN PNDIS_STRING     NdisRoutineName
    );

#endif

typedef VOID (*NDISCANCELSENDPACKETS)(
    IN NDIS_HANDLE  NdisBindingHandle,
    IN LONG* CancelId
    );

typedef NDIS_HANDLE (*NDISGETPOOLFROMPACKET)(
    IN PNDIS_PACKET  Packet
    );    

typedef PNDIS_PACKET_STACK (*NDISIMGETCURRENTPACKETSTACK)(
    IN PNDIS_PACKET  Packet,
    OUT BOOLEAN  *StacksRemaining
    );
    
typedef NDIS_STATUS (*NDISIMNOTIFYPNPEVENT)(
    IN  NDIS_HANDLE  MiniportHandle,
    IN  PNET_PNP_EVENT  NetPnPEvent
    );

typedef VOID (*NDISGETFIRSTBUFFERFROMPACKETSAFE)(
    IN	PNDIS_PACKET  Packet,
    OUT PNDIS_BUFFER  *FirstBuffer,
    OUT PVOID  *FirstBufferVA,
    OUT PUINT  FirstBufferLength,
    OUT PUINT  TotalBufferLength,
    IN	MM_PAGE_PRIORITY  Priority
    );


EXPORT
BOOLEAN
PsGetVersion(
  PULONG  MajorVersion  OPTIONAL,
  PULONG  MinorVersion  OPTIONAL,
  PULONG  BuildNumber  OPTIONAL,
  PUNICODE_STRING  CSDVersion  OPTIONAL
  );


//全局变量

extern NDIS_HANDLE							ProtHandle, DriverHandle;
extern NDIS_MEDIUM							MediumArray[4];
extern PADAPTER								g_AdaptList;
extern NDIS_SPIN_LOCK						GlobalLock;
extern PDRIVER_OBJECT						g_CurrentDriver;

extern NDISCANCELSENDPACKETS				MyNdisCancelSendPackets;
extern NDISGETPOOLFROMPACKET				MyNdisGetPoolFromPacket;
extern NDISIMGETCURRENTPACKETSTACK			MyNdisIMGetCurrentPacketStack;
extern NDISIMNOTIFYPNPEVENT					MyNdisIMNotifyPnPEvent;


extern ULONG gSfOsMajorVersion;
extern ULONG gSfOsMinorVersion;

#define IS_NDIS51() \
    (((gSfOsMajorVersion == 5) && (gSfOsMinorVersion >= 1)) || \
     (gSfOsMajorVersion > 5))

#define IS_WINDOWS2000() \
	( (gSfOsMajorVersion == 5) && (gSfOsMinorVersion == 0) )

#define IS_WINDOWSXP() \
	( (gSfOsMajorVersion == 5) && (gSfOsMinorVersion == 1) )

#define IS_WINDOWS2003() \
	( (gSfOsMajorVersion == 5) && (gSfOsMinorVersion == 2) )

#define PROCESS_NAME_OFFSET_2K						0x1FC

#define PROCESS_NAME_OFFSET_XP						0x174

#define PROCESS_NAME_OFFSET_2003					0x154

//自定义数据

extern ULONG		g_ulCurOpenCount;

extern PKEVENT		g_GlobalEvent;

extern PARPFW_SHARE_MEM	g_ArpFw_ShareMem;

#endif //_RASACSIM4_H
