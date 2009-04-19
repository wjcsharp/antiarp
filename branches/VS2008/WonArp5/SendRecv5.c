/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#include "precomp.h"
#pragma hdrstop

/*************************************************************************************

  当有些网络包的进程规则未知时

*************************************************************************************/

BOOLEAN
MiniportCheckForHang5(
    IN NDIS_HANDLE  MiniportAdapterContext
    )
{
    PADAPTER            pAdapt = (PADAPTER)MiniportAdapterContext;


//	KdPrint(("MiniportCheckForHang : 0x%08x",pAdapt));

	return FALSE;
}

INT
ProtocolReceivePacket5(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet
    )
{
    PADAPTER            pAdapt =(PADAPTER)ProtocolBindingContext;
    NDIS_STATUS         Status;
    PNDIS_PACKET        MyPacket;
    BOOLEAN             Remaining;

	RAS_OPT				RasOpt = OPT_PASS;

    //
    // Drop the packet silently if the upper miniport edge isn't initialized or
    // the miniport edge is in low power state
    //
    if ((!pAdapt->MiniportHandle) || (pAdapt->MiniportDeviceState > NdisDeviceStateD0))
    {
          return 0;
    }

	RasOpt = CheckPacketRecvOpt(pAdapt,Packet,pAdapt->bWanAdapter);
	if(RasOpt == OPT_DROP)
	{
        //
        // We Don't Want this packet to Indicate to Protocol , Silently drop it.
        //
        return(0);
	}

    //
    // Check if we can reuse the same packet for indicating up.
    // See also: PtReceive(). 
    //
    (VOID)MyNdisIMGetCurrentPacketStack(Packet, &Remaining);
    if (Remaining)
    {
        //
        // We can reuse "Packet". Indicate it up and be done with it.
        //
        Status = NDIS_GET_PACKET_STATUS(Packet);
        NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &Packet, 1);
        return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
    }

    //
    // Get a packet off the pool and indicate that up
    //
    NdisDprAllocatePacket(&Status,
                           &MyPacket,
                           pAdapt->RecvPacketPoolHandle);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        PRECV_RSVD            RecvRsvd;

        RecvRsvd = (PRECV_RSVD)(MyPacket->MiniportReserved);
        RecvRsvd->OriginalPkt = Packet;

        NDIS_PACKET_FIRST_NDIS_BUFFER(MyPacket) = NDIS_PACKET_FIRST_NDIS_BUFFER(Packet);
        NDIS_PACKET_LAST_NDIS_BUFFER(MyPacket) = NDIS_PACKET_LAST_NDIS_BUFFER(Packet);

        //
        // Get the original packet (it could be the same packet as the one
        // received or a different one based on the number of layered miniports
        // below) and set it on the indicated packet so the OOB data is visible
        // correctly to protocols above us.
        //
        NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));

        //
        // Set Packet Flags
        //
        NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

        Status = NDIS_GET_PACKET_STATUS(Packet);

        NDIS_SET_PACKET_STATUS(MyPacket, Status);
        NDIS_SET_PACKET_HEADER_SIZE(MyPacket, NDIS_GET_PACKET_HEADER_SIZE(Packet));

        if (pAdapt->MiniportHandle != NULL)
        {
            NdisMIndicateReceivePacket(pAdapt->MiniportHandle, &MyPacket, 1);
        }

        //
        // Check if we had indicated up the packet with NDIS_STATUS_RESOURCES
        // NOTE -- do not use NDIS_GET_PACKET_STATUS(MyPacket) for this since
        // it might have changed! Use the value saved in the local variable.
        //
        if (Status == NDIS_STATUS_RESOURCES)
        {
            //
            // Our ReturnPackets handler will not be called for this packet.
            // We should reclaim it right here.
            //
            NdisDprFreePacket(MyPacket);
        }

        return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
    }
    else
    {
        //
        // We are out of packets. Silently drop it.
        //
        return(0);
    }
}

/*************************************************************************************


*************************************************************************************/
VOID
MiniportSendPackets5(
    IN NDIS_HANDLE             MiniportAdapterContext,
    IN PPNDIS_PACKET           PacketArray,
    IN UINT                    NumberOfPackets
    )
{
    PADAPTER            pAdapt = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS         Status;
    UINT                i;
    PVOID               MediaSpecificInfo		= NULL;
    UINT                MediaSpecificInfoSize	= 0;

    for (i = 0; i < NumberOfPackets; i++)
    {
        PNDIS_PACKET    Packet, MyPacket;

        Packet = PacketArray[i];			

        //
        // The driver should fail the send if the virtual miniport is in low 
        // power state
        //
        if (pAdapt->MiniportDeviceState > NdisDeviceStateD0)
        {
            NdisMSendComplete(pAdapt->MiniportHandle,
                            Packet,
                            NDIS_STATUS_FAILURE);
            continue;
        }

		if(CheckPacketSendOpt(pAdapt,Packet,pAdapt->bWanAdapter) == OPT_DROP)
		{
            NdisMSendComplete(pAdapt->MiniportHandle,
                            Packet,
                            NDIS_STATUS_FAILURE);
            continue;
		}

        //
        // Use NDIS 5.1 packet stacking:
        //
        {
            PNDIS_PACKET_STACK        pStack;
            BOOLEAN                   Remaining;

            //
            // Packet stacks: Check if we can use the same packet for sending down.
            //
            pStack = (PNDIS_PACKET_STACK)MyNdisIMGetCurrentPacketStack(Packet, &Remaining);
            if (Remaining)
            {
                //
                // We can reuse "Packet".
                //
                // NOTE: if we needed to keep per-packet information in packets
                // sent down, we can use pStack->IMReserved[].
                //
                ASSERT(pStack);
                //
                // If the below miniport is going to low power state, stop sending down any packet.
                //
                NdisAcquireSpinLock(&pAdapt->Lock);
                if (pAdapt->PTDeviceState > NdisDeviceStateD0)
                {
                    NdisReleaseSpinLock(&pAdapt->Lock);
                    NdisMSendComplete(pAdapt->MiniportHandle,
                                        Packet,
                                        NDIS_STATUS_FAILURE);
                }
                else
                {
                    pAdapt->OutstandingSends++;
                    NdisReleaseSpinLock(&pAdapt->Lock);

                    NdisSend(&Status,
                              pAdapt->BindingHandle,
                              Packet);
        
                    if (Status != NDIS_STATUS_PENDING)
                    {
                        NdisMSendComplete(pAdapt->MiniportHandle,
                                            Packet,
                                            Status);
                   
                        ADAPT_DECR_PENDING_SENDS(pAdapt);

                    }
                }
                continue;
            }
        }
        do 
        {
            NdisAcquireSpinLock(&pAdapt->Lock);
            //
            // If the below miniport is going to low power state, stop sending down any packet.
            //
            if (pAdapt->PTDeviceState > NdisDeviceStateD0)
            {
                NdisReleaseSpinLock(&pAdapt->Lock);
                Status = NDIS_STATUS_FAILURE;
                break;
            }
            pAdapt->OutstandingSends++;
            NdisReleaseSpinLock(&pAdapt->Lock);

            NdisAllocatePacket(&Status,
                               &MyPacket,
                               pAdapt->SendPacketPoolHandle);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                PSEND_RSVD        SendRsvd;

                SendRsvd = (PSEND_RSVD)(MyPacket->ProtocolReserved);
                SendRsvd->OriginalPkt = Packet;
				SendRsvd->PacketMem   = NULL;

                NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

                NDIS_PACKET_FIRST_NDIS_BUFFER(MyPacket) = NDIS_PACKET_FIRST_NDIS_BUFFER(Packet);
                NDIS_PACKET_LAST_NDIS_BUFFER(MyPacket) = NDIS_PACKET_LAST_NDIS_BUFFER(Packet);

                //
                // Copy the OOB data from the original packet to the new
                // packet.
                //
                NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(MyPacket),
                            NDIS_OOB_DATA_FROM_PACKET(Packet),
                            sizeof(NDIS_PACKET_OOB_DATA));
                //
                // Copy relevant parts of the per packet info into the new packet
                //

                NdisIMCopySendPerPacketInfo(MyPacket, Packet);

                //
                // Copy the Media specific information
                //
                NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
                                                    &MediaSpecificInfo,
                                                    &MediaSpecificInfoSize);

                if (MediaSpecificInfo || MediaSpecificInfoSize)
                {
                    NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(MyPacket,
                                                        MediaSpecificInfo,
                                                        MediaSpecificInfoSize);
                }

                NdisSend(&Status,
                         pAdapt->BindingHandle,
                         MyPacket);

                if (Status != NDIS_STATUS_PENDING)
                {

                    NdisIMCopySendCompletePerPacketInfo (Packet, MyPacket);

                    NdisFreePacket(MyPacket);
                    ADAPT_DECR_PENDING_SENDS(pAdapt);
                }
            }
            else
            {
                //
                // The driver cannot allocate a packet.
                // 
                ADAPT_DECR_PENDING_SENDS(pAdapt);
            }
        }
        while (FALSE);

        if (Status != NDIS_STATUS_PENDING)
        {
            NdisMSendComplete(pAdapt->MiniportHandle,
                              Packet,
                              Status);
        }
    }
}
/************************************************************************************



************************************************************************************/

VOID
ProtocolSendComplete5(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  PNDIS_PACKET           Packet,
    IN  NDIS_STATUS            Status
    )
{
    PADAPTER          pAdapt = (PADAPTER)ProtocolBindingContext;
    PNDIS_PACKET      Pkt; 
    NDIS_HANDLE       PoolHandle;
	PSEND_RSVD        SendRsvd;
	PUCHAR			  PacketMem = NULL;

    PoolHandle = MyNdisGetPoolFromPacket(Packet);
    if (PoolHandle != pAdapt->SendPacketPoolHandle)
    {
        NdisMSendComplete(pAdapt->MiniportHandle,
                          Packet,
                          Status);
    }
    else
    {

        SendRsvd = (PSEND_RSVD)(Packet->ProtocolReserved);
        Pkt		  = SendRsvd->OriginalPkt;
		PacketMem = SendRsvd->PacketMem;

		if(Pkt)
		{
			NdisIMCopySendCompletePerPacketInfo (Pkt, Packet);

			NdisDprFreePacket(Packet);

			NdisMSendComplete(pAdapt->MiniportHandle,
										 Pkt,
										 Status);
		}
		else
		{
			if(PacketMem)
			{
				MyFreeNdisSendPacket(Packet);
				ExFreePool(PacketMem);
			}
			else
			{
				NdisDprFreePacket(Packet);
			}
		}
    }

    ADAPT_DECR_PENDING_SENDS(pAdapt);

}       

/************************************************************************************



************************************************************************************/
VOID
ProtocolTransferDataComplete5(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        Packet,
    IN  NDIS_STATUS         Status,
    IN  UINT                BytesTransferred
    )
{
    PADAPTER			pAdapt =(PADAPTER)ProtocolBindingContext;

	if(pAdapt->MiniportHandle)
	{
		NdisMTransferDataComplete(pAdapt->MiniportHandle,Packet,Status,BytesTransferred);
	}

}

/************************************************************************************



************************************************************************************/
VOID
ProtocolQueueReceivedPacket5(
    IN PADAPTER				pAdapt,
    IN PNDIS_PACKET			pRecvedPacket,
    IN BOOLEAN				DoIndicate,
	IN BOOLEAN				bReturnPacket
    )
{

    NdisDprAcquireSpinLock(&pAdapt->Lock);
    ASSERT(pAdapt->ReceivedPacketCount < MAX_RECV_PACKET_POOL_SIZE);

    pAdapt->ReceivedPackets[pAdapt->ReceivedPacketCount].pNdisPacket	= pRecvedPacket;
	pAdapt->ReceivedPackets[pAdapt->ReceivedPacketCount].bReturnPacket	= bReturnPacket;

    pAdapt->ReceivedPacketCount++;

    if ((pAdapt->ReceivedPacketCount == MAX_RECV_PACKET_POOL_SIZE) || DoIndicate)
    {
        NdisDprReleaseSpinLock(&pAdapt->Lock);

		ProtocolFlushReceiveQueue5(pAdapt);
    }
    else
    {
        NdisDprReleaseSpinLock(&pAdapt->Lock);
    }

}

/************************************************************************************



************************************************************************************/
VOID
ProtocolFlushReceiveQueue5(
    IN PADAPTER         pAdapt
    ) 
{
    RECEIVED_PACKET  PacketArray[MAX_RECV_PACKET_POOL_SIZE];
    ULONG			 NumberOfPackets = 0, i;

	enum RAS_OPT	 RasOpt;

    do
    {
        NdisDprAcquireSpinLock(&pAdapt->Lock);

        if (pAdapt->ReceivedPacketCount > 0)
        {
            NdisMoveMemory(PacketArray,pAdapt->ReceivedPackets,
                            pAdapt->ReceivedPacketCount * sizeof(RECEIVED_PACKET));

            NumberOfPackets = pAdapt->ReceivedPacketCount;
            pAdapt->ReceivedPacketCount = 0;

            NdisDprReleaseSpinLock(&pAdapt->Lock);

			if ((pAdapt->MiniportHandle != NULL) 
					&& (pAdapt->MiniportDeviceState == NdisDeviceStateD0))
			{
				for(i=0; i< NumberOfPackets;i++)
				{
					//我们的包过滤操作
					RasOpt = CheckPacketRecvOpt(pAdapt,PacketArray[i].pNdisPacket,pAdapt->bWanAdapter);
					
					if( RasOpt == OPT_PASS)
					{
						NdisMIndicateReceivePacket(	pAdapt->MiniportHandle,
													&PacketArray[i].pNdisPacket, 1);
					}
					else
					{
						NDIS_SET_PACKET_STATUS(PacketArray[i].pNdisPacket, NDIS_STATUS_NOT_ACCEPTED);
					}

					if(PacketArray[i].bReturnPacket)
					{
						MiniportReturnPacket5(pAdapt, PacketArray[i].pNdisPacket);
					}
					
				}

				break;
			}
	
			for (i = 0; i < NumberOfPackets; i++)
			{
				if(PacketArray[i].bReturnPacket)
				{
					MiniportReturnPacket5(pAdapt, PacketArray[i].pNdisPacket);
				}
			}
			break;

        }

		NdisDprReleaseSpinLock(&pAdapt->Lock);

    } while (FALSE);

}

/************************************************************************************



************************************************************************************/
VOID
ProtocolReceiveComplete5(
    IN NDIS_HANDLE        ProtocolBindingContext
    )
{
    PADAPTER        pAdapt =(PADAPTER)ProtocolBindingContext;
        
    if ((pAdapt->MiniportHandle != NULL)
                && (pAdapt->MiniportDeviceState == NdisDeviceStateD0)
                && (pAdapt->IndicateRcvComplete == TRUE))
    {
        switch (pAdapt->Medium)
        {
            case NdisMedium802_3:
            case NdisMediumWan:
                NdisMEthIndicateReceiveComplete(pAdapt->MiniportHandle);
                break;

            case NdisMedium802_5:
                NdisMTrIndicateReceiveComplete(pAdapt->MiniportHandle);
                break;

            case NdisMediumFddi:
                NdisMFddiIndicateReceiveComplete(pAdapt->MiniportHandle);
                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }

    pAdapt->IndicateRcvComplete = FALSE;

}

/************************************************************************************



************************************************************************************/
NDIS_STATUS
ProtocolReceive5(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         MacReceiveContext,
    IN  PVOID               HeaderBuffer,
    IN  UINT                HeaderBufferSize,
    IN  PVOID               LookAheadBuffer,
    IN  UINT                LookAheadBufferSize,
    IN  UINT                PacketSize
    )
{
    PADAPTER			pAdapt = (PADAPTER)ProtocolBindingContext;
    PNDIS_PACKET		MyPacket, Packet = NULL;
    NDIS_STATUS			Status = NDIS_STATUS_SUCCESS;

	PNDIS_BUFFER		Buffer				= NULL;
	PUCHAR				pPacketMem			= NULL;
	UINT				byteTransfered		= 0;

	enum RAS_OPT		RasOpt;

    if ((!pAdapt->MiniportHandle) || (pAdapt->MiniportDeviceState > NdisDeviceStateD0))
    {
        Status = NDIS_STATUS_FAILURE;
    }
    else do
	{
        //
        // Get at the packet, if any, indicated up by the miniport below.
        //
        Packet = NdisGetReceivedPacket(pAdapt->BindingHandle, MacReceiveContext);
        if (Packet != NULL)
        {

            NdisDprAllocatePacket(&Status,
                                &MyPacket,
                                pAdapt->RecvPacketPoolHandle);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                NDIS_PACKET_FIRST_NDIS_BUFFER(MyPacket) = NDIS_PACKET_FIRST_NDIS_BUFFER(Packet);
                NDIS_PACKET_LAST_NDIS_BUFFER(MyPacket) = NDIS_PACKET_LAST_NDIS_BUFFER(Packet);

                NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));
                NDIS_SET_PACKET_HEADER_SIZE(MyPacket, HeaderBufferSize);

                NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

                NDIS_SET_PACKET_STATUS(MyPacket, NDIS_STATUS_RESOURCES);

				RasOpt = CheckPacketRecvOpt(pAdapt,MyPacket,pAdapt->bWanAdapter);
					
				if( RasOpt == OPT_PASS)
				{
					NdisMIndicateReceivePacket(	pAdapt->MiniportHandle,
													&MyPacket, 1);
				}
				else
				{
					Status = NDIS_STATUS_NOT_ACCEPTED;
				}

                NdisDprFreePacket(MyPacket);

                break;
            }
        }
        else
        {
            //
            // The miniport below us uses the old-style (not packet)
            // receive indication. Fall through.
            //
			if(PacketSize <= LookAheadBufferSize)
			{

				RasOpt = OldRecvPacketOpt(pAdapt,HeaderBuffer,HeaderBufferSize,
									LookAheadBuffer,LookAheadBufferSize,
									PacketSize);

				if( RasOpt == OPT_DROP)
				{
					Status =  NDIS_STATUS_NOT_ACCEPTED;
					break;
				}
			}
			else	//驱动没有接收完整个包
			{
				KdPrint((" 接收到不完整的包 "));
			}
        }

        if (Packet != NULL)
        {
            ProtocolFlushReceiveQueue5(pAdapt);
        }
  
        if ((pAdapt->MiniportHandle == NULL)
                || (pAdapt->MiniportDeviceState > NdisDeviceStateD0))
        {
            break;
        }
        
        pAdapt->IndicateRcvComplete = TRUE;
        switch (pAdapt->Medium)
        {
            case NdisMedium802_3:
            case NdisMediumWan:
                NdisMEthIndicateReceive(pAdapt->MiniportHandle,
                                             MacReceiveContext,
                                             HeaderBuffer,
                                             HeaderBufferSize,
                                             LookAheadBuffer,
                                             LookAheadBufferSize,
                                             PacketSize);
                break;

            case NdisMedium802_5:
                NdisMTrIndicateReceive(pAdapt->MiniportHandle,
                                            MacReceiveContext,
                                            HeaderBuffer,
                                            HeaderBufferSize,
                                            LookAheadBuffer,
                                            LookAheadBufferSize,
                                            PacketSize);
                break;

            case NdisMediumFddi:
                NdisMFddiIndicateReceive(pAdapt->MiniportHandle,
                                              MacReceiveContext,
                                              HeaderBuffer,
                                              HeaderBufferSize,
                                              LookAheadBuffer,
                                              LookAheadBufferSize,
                                              PacketSize);
                break;

            default:
                ASSERT(FALSE);
                break;
        }

    } while(FALSE);

    return Status;
}

