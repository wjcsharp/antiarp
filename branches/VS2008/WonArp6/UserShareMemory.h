
#ifndef SHARE_MEM2_H
#define SHARE_MEM2_H

typedef struct _SHARE_USER_MEM
{
	HANDLE		ProcessId;
	PMDL		Mdl;
	PVOID		ShareMemory;
} SHARE_USER_MEM,*PSHARE_USER_MEM;

NTSTATUS
InitUserShareMemory(PSHARE_USER_MEM User_Mem,PMDL pUserMdl);

NTSTATUS
UninitUserShareMemory(PSHARE_USER_MEM User_Mem);

NTSTATUS
IsUserShareMemoryProcess(PSHARE_USER_MEM User_Mem);

extern SHARE_USER_MEM	g_Share_User_Mem;

#endif

