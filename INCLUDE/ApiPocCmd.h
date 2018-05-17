#ifndef __APIPOCCMD_H
#define __APIPOCCMD_H

#include "AllHead.h"

typedef enum{
  PocComm_OpenPOC               = 0x00,
  PocComm_SetParam		= 0x01,
  PocComm_SetURL		= 0x02,
  PocComm_Login			= 0x03,
  PocComm_Logout		= 0x04,
  PocComm_Cancel		= 0x05,
  PocComm_ModifyPwd		= 0x06,
  PocComm_EnterGroup		= 0x09,
  PocComm_Invite                = 0x0A,
  PocComm_StartPTT		= 0x0B,
  PocComm_EndPTT		= 0x0C,
  PocComm_GroupListInfo	        = 0x0D,
  PocComm_UserListInfo          = 0x0E,
  PocComm_SetGps                = 0x0F,
  PocComm_Key			= 0x10
}PocCommType;

typedef enum{
  OffLine       =0x00,
  Landing       =0x01,
  LandSuccess   =0x02,
  Logout        =0x03
}PocStatesType;

typedef enum{
  LeaveGroup    =0x00,
  EnterGroup    =0x01,
  InGroup       =0x02
}GroupStatsType;

typedef enum{
  ReceivedVoiceNone     = 0x00,
  ReceivedVoiceStart    = 0x01,
  ReceivedVoiceBeing    = 0x02,
  ReceivedVoiceEnd      = 0x03
}ReceivedVoicePlayStatesType;

typedef enum{
  m_group_mode     =0x00,
  m_personal_mode  = 0x01
}working_status_type;

extern GroupStatsType ApiPocCmd_GroupStates(void);
extern void ApiPocCmd_GroupStatesSet(GroupStatsType a);

extern u8 ApiPocCmd_KeyPttState(void);
extern void ApiPocCmd_SetKeyPttState(u8 i);

extern bool ApiPocCmd_ReceivedVoicePlayStates(void);
extern void ApiPocCmd_ReceivedVoicePlayStatesSet(bool a);

extern ReceivedVoicePlayStatesType ApiPocCmd_ReceivedVoicePlayStatesForDisplay(void);
extern void ApiPocCmd_ReceivedVoicePlayStatesForDisplaySet(ReceivedVoicePlayStatesType a);


extern bool ApiPocCmd_ReceivedVoicePlayStatesIntermediate(void);//中间变量
extern void ApiPocCmd_ReceivedVoicePlayStatesIntermediateSet(bool a);//中间变量

extern bool ApiPocCmd_ToneStateIntermediate(void);
extern void ApiPocCmd_ToneStateIntermediateSet(bool a);

extern bool ApiPocCmd_ReceivedVoicePlayStatesForLED(void);

extern bool ApiPocCmd_ToneState(void);
extern void ApiPocCmd_ToneStateSet(bool a);

extern working_status_type get_current_working_status(void);
/*****群组用户名相关调用函数****************/
extern u8 *GetNowWorkingGroupNameForDisplay(void);
extern u8 GetNowWorkingGroupNameLenForDisplay(void);
extern u8 *GetAllGroupNameForDisplay(u8 a);
extern u8 *GetSpeakingUserNameForDisplay(void);//说话的用户：显示屏
extern u8 *GetAllUserNameForDisplay(u8 a);//所有用户：显示屏
extern u16 GetNowWorkingGroupXuhao(void);
extern u16 GetAllGroupNum(void);
extern u16 GetAllUserNum(void);
extern u8 *GetAllGroupNameForVoice(u8 a);
extern u8 *GetAllUserNameForVoice(u8 a);//所有用户：播报
extern void get_screen_display_group_name(void);
/*************/
extern void ApiPocCmd_PowerOnInitial(void);
extern void ApiPocCmd_WritCommand(PocCommType id, u8 *buf, u16 len);
extern bool ApiPocCmd_user_info_set(u8 *pBuf, u8 len);//cTxBuf为存放ip账号密码的信息
extern u8 ApiPocCmd_user_info_get(u8 ** pBuf);
extern void ApiPocCmd_10msRenew(void);
//extern void ApiPocCmd_83_1msRenew(void);


#endif