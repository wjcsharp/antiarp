
#include "precomp.h"
#pragma hdrstop

USER_SHARE_EVENT		g_NotifyEvent = { 0, NULL, NULL};

NTSTATUS
InitUserShareEvent(PUSER_SHARE_EVENT UserEvent,HANDLE EventHandle)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if( UserEvent->Event	  != NULL )
	{
		return STATUS_SUCCESS;
	}

	Status = ObReferenceObjectByHandle(	EventHandle,
											EVENT_MODIFY_STATE,
											*ExEventObjectType,
											KernelMode,
											(PVOID*)&UserEvent->Event,
											0);
	if(Status == STATUS_SUCCESS)
	{
		UserEvent->EventHandle = EventHandle;
		UserEvent->ProcessId   = PsGetCurrentProcessId();
	}

	return Status;
}

NTSTATUS
UninitUserShareEvent(PUSER_SHARE_EVENT UserEvent)
{
	if(UserEvent->ProcessId != PsGetCurrentProcessId())
	{
		return STATUS_UNSUCCESSFUL;
	}

	if(UserEvent->Event != NULL)
	{
		ObDereferenceObject(UserEvent->Event);
		UserEvent->Event		= NULL;
		UserEvent->EventHandle  = NULL;
		UserEvent->ProcessId    = 0;
	}

	return STATUS_SUCCESS;
}

VOID
SetUserShareEvent(PUSER_SHARE_EVENT UserEvent)
{
	if(UserEvent->Event != NULL)
	{
		KeSetEvent(UserEvent->Event,IO_NO_INCREMENT, FALSE);
	}
}


