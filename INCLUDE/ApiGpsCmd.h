#ifndef  _API_GPS_Cmd_H
#define  _API_GPS_Cmd_H
#include "AllHead.h"

extern void ApiGpsCmd_PowerOnInitial(void);
extern void ApiGpsCmd_100msRenew(void);//决定什么时候发送什么数据;
extern void ApGpsCmd_10msRenew(void);//获取收到的指令，并解析

extern u8 *ApiGps_GetTcpIpAddress();
extern u8 *ApiGps_GetTcpPortAddress();
//extern void ApiGpsCmd_PowerReset(void);
//extern void ApiGpsCmd_1sRenew(void);
//extern u8 * ApiGpsCmd_GetIP(void);
//extern u8 * ApiGpsCmd_GetPort(void);
//extern BOOL ApiGpsCmd_GetCountDown(void);
//extern BOOL ApiGpsCmd_GetEnable(u8 Index);
//extern u8 ApiGpsCmd_GetServicer(void);
//extern u8 ApiGpsCmd_GetWorkMode(void);
//extern u8 * ApiGpsCmd_GetMapBuf(void);
//extern BOOL ApiGpsCmd_SetMapBuf(u8 ucData);
//extern void ApiGpsCmd_SetMapStart(void);
//extern u8 ApiGpsCmd_GetGpsProtocal(void);

#endif
