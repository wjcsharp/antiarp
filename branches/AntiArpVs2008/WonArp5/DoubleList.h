
#ifndef _DOUBLE_LIST_H
#define _DOUBLE_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

NTSTATUS
InitPacketList();

VOID
UnInitPacketList();

VOID
AddPacketToListTail(NOTIFY_PACKET*	Packet);

NOTIFY_PACKET*
GetPacketFromListHead();

VOID
FreePacket(NOTIFY_PACKET*	Packet);

NOTIFY_PACKET*
AllocatePacket();

#endif

