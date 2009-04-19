
#include "precomp.h"
#pragma hdrstop

SHARE_USER_MEM	g_Share_User_Mem = {NULL,NULL};

NTSTATUS
InitUserShareMemory(PSHARE_USER_MEM User_Mem,PMDL pUserMdl)
{
	PVOID	pSysAddress = NULL;
	ULONG	ulShareSize = MmGetMdlByteCount(pUserMdl);

	if(KeGetCurrentIrql() != PASSIVE_LEVEL) return STATUS_NOT_SUPPORTED;

	if(g_Share_User_Mem.Mdl) return STATUS_UNSUCCESSFUL;

	pSysAddress = MmGetSystemAddressForMdlSafe(pUserMdl,NormalPagePriority);

	if(!pSysAddress)
		return STATUS_NOT_SUPPORTED;

	User_Mem->Mdl = IoAllocateMdl(pSysAddress,ulShareSize,FALSE,FALSE,NULL);

	if(!User_Mem->Mdl)
	{
		return STATUS_NOT_SUPPORTED;
	}

	try
	{
		MmProbeAndLockPages(User_Mem->Mdl,KernelMode,IoModifyAccess);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		User_Mem->ShareMemory = NULL;

		IoFreeMdl(User_Mem->Mdl);
		User_Mem->Mdl = NULL;
		return STATUS_UNSUCCESSFUL;
	}

	User_Mem->ShareMemory = MmGetSystemAddressForMdlSafe(User_Mem->Mdl,NormalPagePriority);

	User_Mem->ProcessId = PsGetCurrentProcessId();

	return STATUS_SUCCESS;
}


NTSTATUS
UninitUserShareMemory(PSHARE_USER_MEM User_Mem)
{
	if(User_Mem->ShareMemory && User_Mem->Mdl)
	{
		MmUnlockPages(User_Mem->Mdl);

		IoFreeMdl(User_Mem->Mdl);
		User_Mem->Mdl = NULL;

	}

	return STATUS_SUCCESS;
}

NTSTATUS
IsUserShareMemoryProcess(PSHARE_USER_MEM User_Mem)
{
	if(PsGetCurrentProcessId() != User_Mem->ProcessId)
	{
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}