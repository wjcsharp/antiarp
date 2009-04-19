
/******************************************************************************

	NDIS Filter 6.0

	用途 : ARP 防火墙

  	支持系统 : Vista / Server 2008

    作者  : 张聚长

*******************************************************************************/


#include <ndis.h>
#include "struct.h"
#include "Share60.h"
#include "UserShareMemory.h"
#include "UserShareEvent.h"
#include "GatewayCheck.h"

#include "Packet.h"
#include "flt_dbg.h" 
#include "filter.h"

#define TAG		'gdTF'

