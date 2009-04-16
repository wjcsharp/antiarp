
/******************************************************************************

	NDIS Filter 6.0

	用途 : ARP 防火墙

  	支持系统 : Vista / Server 2008

    作者  : 张聚长

*******************************************************************************/

#ifndef SHARE6_H
#define	SHARE6_H

#ifndef FILE_DEVICE_PHYSICAL_NETCARD
#define FILE_DEVICE_PHYSICAL_NETCARD		0x00000017
#endif

#include "../Share/ShareStruct.h"

#define _NDIS_CONTROL_CODE(request,method) \
            CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, request, method, FILE_ANY_ACCESS)

// 增加IP信息
#define IOCTL_ADDIPINFO_60					_NDIS_CONTROL_CODE(0x1, METHOD_BUFFERED)
// 移除本地IP信息	
#define	IOCTL_REMOVE_ALL_LAN_INFO_60		_NDIS_CONTROL_CODE(0x2, METHOD_BUFFERED)
// 移除网关保护信息
#define IOCTL_REMOVE_ALL_GATEWAY_INFO_60	_NDIS_CONTROL_CODE(0x3, METHOD_BUFFERED)
// 开始由IP地址查询对应的MAC地址
#define IOCTL_BEGIN_ARP_QUERY_60			_NDIS_CONTROL_CODE(0x4, METHOD_BUFFERED)
// 结束由IP地址查询对应的MAC地址
//#define IOCTL_END_ARP_QUERY_60				_NDIS_CONTROL_CODE(0x5, METHOD_BUFFERED)
// 获取记录到的网关回应包信息
//#define IOCTL_READ_REPLY_RECORD_60			_NDIS_CONTROL_CODE(0x6, METHOD_BUFFERED)
// 启用收包攻击检测
#define IOCTL_ENABLE_GATEWAY_CHECK_60		_NDIS_CONTROL_CODE(0x7, METHOD_BUFFERED)
// 启用IP冲突攻击检测
#define IOCTL_ENABLE_SAMEIP_CHECK_60		_NDIS_CONTROL_CODE(0x9, METHOD_BUFFERED)
// 禁用收包攻击检测
#define IOCTL_DISABLE_GATEWAY_CHECK_60		_NDIS_CONTROL_CODE(0xA, METHOD_BUFFERED)
// 禁用IP冲突攻击检测
#define IOCTL_DISABLE_SAMEIP_CHECK_60		_NDIS_CONTROL_CODE(0xC, METHOD_BUFFERED)
//设置共享对象
#define IOCTL_SET_SHARE_MEMORY_60 			_NDIS_CONTROL_CODE(0xF, METHOD_IN_DIRECT)
//清除共享对象
#define IOCTL_CLEAR_SHARE_MEMORY_60 		_NDIS_CONTROL_CODE(0x10, METHOD_IN_DIRECT)





#endif

