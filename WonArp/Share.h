/******************************************************************************

	NDIS IM 4.0 / 5.1

	用途 : ARP 防火墙

  	支持系统 : 2000 sp4 / XP / 2003

    作者  : 张聚长

*******************************************************************************/

#ifndef SHARE_H
#define	SHARE_H

#include "../Share/ShareStruct.h"

// 增加IP信息
#define IOCTL_ADDIPINFO \
			CTL_CODE( FILE_DEVICE_NETWORK,0x201, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 移除本机IP信息
#define	IOCTL_REMOVE_ALL_LAN_INFO\
			CTL_CODE( FILE_DEVICE_NETWORK,0x202, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

// 移除网关地址信息
#define IOCTL_REMOVE_ALL_GATEWAY_INFO	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x203, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 开始由IP地址查询对应的MAC地址
#define IOCTL_BEGIN_ARP_QUERY	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x204, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 结束由IP地址查询对应的MAC地址
//#define IOCTL_END_ARP_QUERY	\
//			CTL_CODE( FILE_DEVICE_NETWORK,0x205, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 获取记录到的网关回应包信息
//#define IOCTL_READ_REPLY_RECORD	\
//			CTL_CODE( FILE_DEVICE_NETWORK,0x206, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 启用收包攻击检测
#define IOCTL_ENABLE_GATEWAY_CHECK	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x207, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 启用IP冲突攻击检测
#define IOCTL_ENABLE_SAMEIP_CHECK	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x209, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 禁用收包攻击检测
#define IOCTL_DISABLE_GATEWAY_CHECK	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x20A, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 禁用IP冲突攻击检测
#define IOCTL_DISABLE_SAMEIP_CHECK	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x20C, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OPENADAPTER 	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x20D, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLOSEADAPTER 	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x20E, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_SHARE_MEMORY 	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x20F, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_CLEAR_SHARE_MEMORY 	\
			CTL_CODE( FILE_DEVICE_NETWORK,0x210, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#endif

