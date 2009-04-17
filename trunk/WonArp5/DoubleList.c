
#include "precomp.h"
#pragma hdrstop

#define				MAX_NOTIFY_PACKET			512

NOTIFY_PACKET*		g_PacketListHead			= NULL;

NOTIFY_PACKET*		g_PacketListTail			= NULL;

NOTIFY_PACKET*		g_PacketArray				= NULL;

NOTIFY_PACKET*		g_FreePacketList			= NULL;

NTSTATUS
InitPacketList()
{
	UINT i;

	g_PacketListHead = g_PacketListTail = NULL;

	g_PacketArray = (NOTIFY_PACKET*)ExAllocatePoolWithTag(NonPagedPool,MAX_NOTIFY_PACKET*sizeof(NOTIFY_PACKET),TAG);

	if(!g_PacketArray)
		return STATUS_UNSUCCESSFUL;

	RtlZeroMemory(g_PacketArray,MAX_NOTIFY_PACKET*sizeof(NOTIFY_PACKET));

	for( i = 0; i< MAX_NOTIFY_PACKET; i++ )
	{
		g_PacketArray[i].Next = g_FreePacketList;
		g_FreePacketList	  = &g_PacketArray[i];
	}

	return STATUS_SUCCESS;
}

VOID
UnInitPacketList()
{

	g_PacketListHead = g_PacketListTail = g_FreePacketList = NULL;

	ExFreePool(g_PacketArray);

	g_PacketArray = NULL;

}


VOID
AddPacketToListTail(NOTIFY_PACKET*	pPacket)
{
	if(pPacket == NULL) return;

	NdisAcquireSpinLock(&GlobalLock);

	if(g_PacketListHead)
	{
		pPacket->Next			= NULL;	   // Tail->Next = NULL;
		g_PacketListTail->Next	= pPacket; // Old->Next  = New;
		g_PacketListTail		= pPacket; // Tail       = New;
	}
	else
	{
		pPacket->Next		= NULL;		   // Tail->Next = NULL;
		g_PacketListTail	= pPacket;	   // Tail  = New;
		g_PacketListHead	= pPacket;	   // Head  = New;
	}

	NdisReleaseSpinLock(&GlobalLock);

}


NOTIFY_PACKET*
GetPacketFromListHead()
{

	NOTIFY_PACKET*	pPacket= NULL;

	NdisAcquireSpinLock(&GlobalLock);

	if(g_PacketListHead)
	{
		pPacket				= g_PacketListHead;
		g_PacketListHead	= pPacket->Next;
	}
	else
	{
		pPacket			= NULL;
	}

	NdisReleaseSpinLock(&GlobalLock);

	return pPacket;
}


VOID
FreePacket(NOTIFY_PACKET* pPacket)
{
	NdisAcquireSpinLock(&GlobalLock);

	if(pPacket)
	{
		pPacket->Next		= g_FreePacketList;
		g_FreePacketList	= pPacket;
	}

	NdisReleaseSpinLock(&GlobalLock);

}


NOTIFY_PACKET*
AllocatePacket()
{
	NOTIFY_PACKET*	Packet = NULL;

	NdisAcquireSpinLock(&GlobalLock);

	if(g_FreePacketList )
	{
		Packet				= g_FreePacketList;
		g_FreePacketList	= Packet->Next;
	}

	NdisReleaseSpinLock(&GlobalLock);

	return Packet;
}

