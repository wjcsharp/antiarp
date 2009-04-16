
/******************************************************************************

	NDIS Filter 6.0

	用途 : ARP 防火墙

  	支持系统 : Vista / Server 2008

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop


NDIS_STATUS
FilterRegisterDevice(
    VOID
    )
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING         DeviceName;
    UNICODE_STRING         DeviceLinkUnicodeString;
    PDRIVER_DISPATCH       DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];
    NDIS_DEVICE_OBJECT_ATTRIBUTES   DeviceAttribute;
    PFILTER_DEVICE_EXTENSION        FilterDeviceExtension;
    PDRIVER_OBJECT                  DriverObject;
   
    DEBUGP(DL_TRACE, ("==>FilterRegisterDevice\n"));
   
    
    NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));
    
    DispatchTable[IRP_MJ_CREATE] = DispatchCreate;
    DispatchTable[IRP_MJ_CLEANUP] = DispatchCleanup;
    DispatchTable[IRP_MJ_CLOSE] = DispatchClose;
    DispatchTable[IRP_MJ_DEVICE_CONTROL] = FilterDeviceIoControl;
    
    
    NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
    NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);
    
    //
    // Create a device object and register our dispatch handlers
    //
    NdisZeroMemory(&DeviceAttribute, sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES));
    
    DeviceAttribute.Header.Type = NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
    DeviceAttribute.Header.Revision = NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
    DeviceAttribute.Header.Size = sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES);
    
    DeviceAttribute.DeviceName = &DeviceName;
    DeviceAttribute.SymbolicName = &DeviceLinkUnicodeString;
    DeviceAttribute.MajorFunctions = &DispatchTable[0];
    DeviceAttribute.ExtensionSize = sizeof(FILTER_DEVICE_EXTENSION);
    
    Status = NdisRegisterDeviceEx(
                FilterDriverHandle,
                &DeviceAttribute,
                &DeviceObject,
                &NdisFilterDeviceHandle
                );
   
   
    if (Status == NDIS_STATUS_SUCCESS)
    {
        FilterDeviceExtension = NdisGetDeviceReservedExtension(DeviceObject);
   
        FilterDeviceExtension->Signature = 'FTDR';
        FilterDeviceExtension->Handle = FilterDriverHandle;

        //
        // Workaround NDIS bug
        //
        DriverObject = (PDRIVER_OBJECT)FilterDriverObject;
    }
              
        
    DEBUGP(DL_TRACE, ("<==PtRegisterDevice: %x\n", Status));
        
    return (Status);
        
}

VOID
FilterDeregisterDevice(
    IN VOID
    )

{
    if (NdisFilterDeviceHandle != NULL)
    {
        NdisDeregisterDeviceEx(NdisFilterDeviceHandle);
    }

    NdisFilterDeviceHandle = NULL;

}


NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
{
	NTSTATUS			Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpStack;

	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	IrpStack->FileObject->FsContext = NULL;

	g_ulCurOpenCount ++;

	if(g_ulCurOpenCount == 1)
	{

	}

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information=0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;

}


NTSTATUS
DispatchClose(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
{
	PIO_STACK_LOCATION  IrpStack = IoGetCurrentIrpStackLocation(Irp);

	g_ulCurOpenCount --;

	if(g_ulCurOpenCount == 0)
	{
		
	}

	if(IsUserShareMemoryProcess(&g_Share_User_Mem) == STATUS_SUCCESS)
	{
		// PASSIVE_LEVEL
		UnInitArpCheck();

		NdisAcquireSpinLock(&GlobalLock);
		g_ArpFw_ShareMem = NULL;
		NdisReleaseSpinLock(&GlobalLock);

		UninitUserShareMemory(&g_Share_User_Mem);
	}

	UninitUserShareEvent(&g_NotifyEvent);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

}


NTSTATUS
DispatchCleanup(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
{
    PIO_STACK_LOCATION       IrpStack;
    NTSTATUS                 Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DeviceObject);

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    



    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}


NTSTATUS
DispatchNull(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
{
    PIO_STACK_LOCATION       IrpStack;
    NTSTATUS                 Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DeviceObject);

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}


PMS_FILTER    
filterFindFilterModule(
    IN PUCHAR                   Buffer,
    IN ULONG                    BufferLength
    )
{

   PMS_FILTER              pFilter;
   PLIST_ENTRY             Link;
   
   FILTER_ACQUIRE_LOCK(&FilterListLock, FALSE);
               
   Link = FilterModuleList.Flink;
               
   while (Link != &FilterModuleList)
   {
       pFilter = CONTAINING_RECORD(Link, MS_FILTER, FilterModuleLink);

       if (BufferLength >= pFilter->FilterModuleName.Length)
       {
           if (NdisEqualMemory(Buffer, pFilter->FilterModuleName.Buffer, pFilter->FilterModuleName.Length))
           {
               FILTER_RELEASE_LOCK(&FilterListLock, FALSE);
               return pFilter;
           }
       }
           
       Link = Link->Flink;
   }
   
   FILTER_RELEASE_LOCK(&FilterListLock, FALSE);
   return NULL;
}




