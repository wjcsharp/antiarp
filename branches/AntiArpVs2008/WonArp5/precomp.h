
/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#pragma warning(disable:4214)   // bit field types other than int

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4127)   // conditional expression is constant
#pragma warning(disable:4054)   // cast of function pointer to PVOID
#pragma warning(disable:4244)   // conversion from 'int' to 'BOOLEAN', possible loss of data

#define		NDIS_MINIPORT_DRIVER 
#define	    NDIS_WDM					1

#ifndef		DRIVER
#define		DRIVER
#endif

#ifndef		NDIS51
	#define		NDIS50_MINIPORT			1
	#define		NDIS50					1
#endif

#include <wdm.h>
#include <ndis.h>
#include "structs.h"
#include "share.h"
#include "WonArp.h"
//4.0
#include "protocol4.h"
#include "Miniport4.h"
//5.1
#include "protocol5.h"
#include "miniport51.h"

//包的处理模块
#include "Packet.h"
#include "UserShareEvent.h"
#include "UserShareMemory.h"
#include "GatewayCheck.h"


#define	PROTOCOL_NAME		L"WonArp"
#define LINKNAME_STRING     L"\\DosDevices\\WonArp"
#define NTDEVICE_STRING     L"\\Device\\WonArp"


HANDLE
  PsGetCurrentProcessId();

//
// There should be no DbgPrint's in the Free version of the driver
//
#if DBG

#define DBGPRINT(Fmt)                                       \
    {                                                       \
       DbgPrint("WonArp: ");                               \
       DbgPrint Fmt;                                        \
    }

#else // if DBG

#define DBGPRINT(Fmt)                                            

#endif // if DBG

#ifndef NDIS51

PKEVENT 
  IoCreateSynchronizationEvent(
  IN PUNICODE_STRING  EventName,
  OUT PHANDLE  EventHandle
  );

#endif


