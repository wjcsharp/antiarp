
#ifndef _USER_SHARE_EVENT_H
#define _USER_SHARE_EVENT_H

typedef struct _USER_SHARE_EVENT
{
	HANDLE		ProcessId;
	HANDLE		EventHandle;
	PKEVENT		Event;
} USER_SHARE_EVENT,*PUSER_SHARE_EVENT;

VOID
SetUserShareEvent(PUSER_SHARE_EVENT UserEvent);

NTSTATUS
UninitUserShareEvent(PUSER_SHARE_EVENT UserEvent);

NTSTATUS
InitUserShareEvent(PUSER_SHARE_EVENT UserEvent,HANDLE EventHandle);


extern USER_SHARE_EVENT		g_NotifyEvent;

#endif

