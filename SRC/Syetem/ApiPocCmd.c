#include "ALLHead.h"

#define DrvMC8332_UseId_Len	        100//define UART Tx buffer length value
#define APIPOC_GroupName_Len            32//unicode只存前2位，00不存，32/2=16,屏幕最多显示16个字符
#define APIPOC_UserName_Len             38
#define APIPOC_Group_Num                40
#define APIPOC_User_Num                 20

const u8 *ucAtPocHead          = "AT+POC=";
const u8 *ucSetIPAndID         = "010000";
const u8 *ucPocOpenConfig      ="00000001010101";
u8 *ucStartPTT                  = "0B0000";
u8 *ucEndPTT                    = "0C0000";
u8 *ucGroupListInfo             = "0d0000";
u8 *ucUserListInfo              = "0e00000000";
u8 *ucSetGPS                  = "110000";
typedef struct{
  struct{
    union{
      struct{
        u16 bUserInfo	: 3;
        u16 bUserWrite	: 1;
        u16 bPocReset	: 1;
        u16 bPocOpen	: 1;
        u16 bModeChange	: 1;
        u16 bMode	: 3;
        u16 bNetStat	: 2;
        u16 bUnline	: 1;
        u16             : 1;
        u16             : 2;
      }Bits;
      u16 Byte;
    }Msg;
    struct{
      struct{
        u8 bSet	: 1;
        u8 Len	: 7;
      }Msg;
      u8 Buf[DrvMC8332_UseId_Len];
    }LoginInfo;
  }NetState;
  
  struct{
    working_status_type current_working_status;
    PocStatesType PocStatus;
    GroupStatsType GroupStats;
    u8 KeyPttState;
    bool ReceivedVoicePlayStates;
    bool ReceivedVoicePlayStates_Intermediate;//喇叭
    bool ReceivedVoicePlayStatesForLED;
    ReceivedVoicePlayStatesType ReceivedVoicePlayStatesForDisplay;
    bool ToneState;
    bool ToneState_Intermediate;
  }States;
  struct{
/*****组名**************/
    struct{
      u16 ID;
      u8  Name[APIPOC_GroupName_Len];
      u8  NameLen;
    }AllGroupName[APIPOC_Group_Num];//所有群组成员名
    struct{
      u16 ID;
      u8  Name[APIPOC_GroupName_Len];
      u8  NameLen;
    }NowWorkingGroupName;//当前工作群组成员名
/******人名**************/
    struct{
      u32 ID;
      u8  Name[APIPOC_UserName_Len];
      u8 NameLen;
    }AllGroupUserName[APIPOC_User_Num];//群组成员名
    struct{
      u8 Name[APIPOC_UserName_Len];
      u8 NameLen;
    }LocalUserName;//本机用户名
    struct{
      u8 Name[APIPOC_UserName_Len];
      u8 NameLen;
    }SpeakingUserName;//当前说话人的名字
/**************************/
  }NameInfo;
  u8 ReadBuffer[80];//存EEPROM读取的数据使用
  u8 NowWorkingGroupNameBuf[APIPOC_GroupName_Len];
  u8 AllGroupNameBuf[APIPOC_GroupName_Len];
  u8 AllUserNameBuf[APIPOC_UserName_Len];
  u8 SpeakingUserNameBuf[APIPOC_UserName_Len];
  u8 LocalUserNameBuf[APIPOC_UserName_Len];
  
  u8 NowWorkingGroupNameForVoiceBuf[APIPOC_GroupName_Len*2+10];
  u8 AllGroupNameForVoiceBuf[APIPOC_GroupName_Len*2+10];
  u8 AllUserNameForVoiceBuf[APIPOC_UserName_Len*2+10];
  u8 LocalUserNameForVoiceBuf[APIPOC_UserName_Len*2+10];
  u16 offline_user_count;
  u16 all_user_num;//所有成员（包括离线）
  u16 GroupXuhao;
  u16 UserXuhao;
  u8 GroupIdBuf[5];
  u8 UserIdBuf[8];
  u8 gps_info_report[45];
  struct{
    u32 longitude_integer ;//度
    u32 longitude_float;//小数点后的数
    u32 latitude_integer;//度
    u32 latitude_float;//小数点后的数
  }Position;
}PocCmdDrv;

static PocCmdDrv PocCmdDrvobj;
static bool no_online_user(void);
#if 1//CDMA 中兴
void ApiPocCmd_PowerOnInitial(void)
{
  PocCmdDrvobj.States.current_working_status = m_group_mode;
  PocCmdDrvobj.States.PocStatus = OffLine;
  PocCmdDrvobj.States.GroupStats = LeaveGroup;
  PocCmdDrvobj.States.KeyPttState = 0;
  PocCmdDrvobj.States.ReceivedVoicePlayStates = FALSE;
  PocCmdDrvobj.States.ReceivedVoicePlayStates_Intermediate = FALSE;//喇叭
  PocCmdDrvobj.States.ReceivedVoicePlayStatesForLED = FALSE;
  PocCmdDrvobj.States.ReceivedVoicePlayStatesForDisplay = ReceivedVoiceNone;
  PocCmdDrvobj.States.ToneState = FALSE;
  PocCmdDrvobj.States.ToneState_Intermediate = FALSE;
  
  PocCmdDrvobj.offline_user_count=0;
  PocCmdDrvobj.all_user_num=0;
  PocCmdDrvobj.GroupXuhao=0;
  PocCmdDrvobj.UserXuhao=0;
  
  PocCmdDrvobj.NetState.Msg.Byte = 0x00;
}
#endif

#if 1//WCDMA 卓智达
void ApiPocCmd_WritCommand(PocCommType id, u8 *buf, u16 len)
{
  u8 cBuf[4]={0,0,0,0};
  u8 primary_buf_len;
  u16 i,temp_value;
  u8 gps_info_buf[25];
  DrvMC8332_TxPort_SetValidable(ON);
  DrvGD83_UART_TxCommand((u8 *)ucAtPocHead,strlen((char const *)ucAtPocHead));
  switch(id)
  {
  case PocComm_OpenPOC://1
    DrvGD83_UART_TxCommand((u8*)ucPocOpenConfig, strlen((char const *)ucPocOpenConfig));
    break;
  case PocComm_SetParam://设置账号密码
    memset(PocCmdDrvobj.ReadBuffer,0,sizeof(PocCmdDrvobj.ReadBuffer));
    FILE_Read(0,80,PocCmdDrvobj.ReadBuffer);//80位
    primary_buf_len=strlen((char const*)PocCmdDrvobj.ReadBuffer);
    PocCmdDrvobj.ReadBuffer[primary_buf_len]='0';
    PocCmdDrvobj.ReadBuffer[primary_buf_len+1]='0';
    DrvGD83_UART_TxCommand((u8 *)ucSetIPAndID,strlen((char const *)ucSetIPAndID));
    DrvGD83_UART_TxCommand(PocCmdDrvobj.ReadBuffer, strlen((char const *)PocCmdDrvobj.ReadBuffer));
    break;
  case PocComm_SetURL:
      DrvGD83_UART_TxCommand((u8 *)"120000687474703a2f2f736925642e7265616c7074742e636f6d3a32393939392f00",strlen((char const *)"120000687474703a2f2f736925642e7265616c7074742e636f6d3a32393939392f00"));
  case PocComm_Login:
    break;
  case PocComm_Logout:
  case PocComm_Cancel:
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case PocComm_ModifyPwd:
    break;
  case PocComm_EnterGroup:
    DrvGD83_UART_TxCommand("0900000000", 10);
    COML_HexToAsc(PocCmdDrvobj.NameInfo.AllGroupName[GroupCallingNum].ID,PocCmdDrvobj.GroupIdBuf);
    COML_StringReverse(4,PocCmdDrvobj.GroupIdBuf);
    DrvGD83_UART_TxCommand(PocCmdDrvobj.GroupIdBuf, 4);
    break;
  case PocComm_Invite:
    DrvGD83_UART_TxCommand("0a0000", 6);
    memset(PocCmdDrvobj.UserIdBuf,0,sizeof(PocCmdDrvobj.UserIdBuf));
    COML_HexToAsc(PocCmdDrvobj.NameInfo.AllGroupUserName[PersonalCallingNum].ID,PocCmdDrvobj.UserIdBuf);
    temp_value=strlen((char const*)PocCmdDrvobj.UserIdBuf);
    if(temp_value<8)
    {
      for(i=temp_value;i<8;i++)
      {
        PocCmdDrvobj.UserIdBuf[i]=0x30;
      }
    }
    COML_StringReverse(8,PocCmdDrvobj.UserIdBuf);
    DrvGD83_UART_TxCommand(PocCmdDrvobj.UserIdBuf, 8);
    break;
  case PocComm_StartPTT://3
    DrvGD83_UART_TxCommand(ucStartPTT,strlen((char const *)ucStartPTT));
    ApiPocCmd_ToneStateSet(TRUE);
    break;
  case PocComm_EndPTT://4
    DrvGD83_UART_TxCommand(ucEndPTT,strlen((char const *)ucEndPTT));
    ApiPocCmd_ToneStateSet(TRUE);
    break;
  case PocComm_GroupListInfo://5
     DrvGD83_UART_TxCommand(ucGroupListInfo, strlen((char const *)ucGroupListInfo));
    break;
  case PocComm_UserListInfo://6
#if 1//Test OK
    PocCmdDrvobj.offline_user_count=0;//发射0E指令前清零
    DrvGD83_UART_TxCommand(ucUserListInfo, strlen((char const *)ucUserListInfo));
    i=GetNowWorkingGroupXuhao()+1;//
    COML_HexToAsc(i,cBuf);
    switch(strlen((char const *)cBuf))
    {
    case 1:
      cBuf[1]='0';
      cBuf[2]='0';
      cBuf[3]='0';
      break;
    case 2:
      cBuf[2]='0';
      cBuf[3]='0';
      break;
    case 3:
      cBuf[3]='0';
      break;
    default:
      break;
    }
    COML_StringReverse(4,cBuf);
    DrvGD83_UART_TxCommand(cBuf,4);
#endif
    break;
  case PocComm_SetGps:
    DrvGD83_UART_TxCommand(ucSetGPS,strlen((char const *)ucSetGPS));
#if 1 //经纬度小数位合并换算及参数传递
    PocCmdDrvobj.Position.longitude_integer=beidou_longitude_degree();
    PocCmdDrvobj.Position.longitude_float = ((beidou_longitude_minute()*10000+beidou_longitude_minute())*10/6);//小数点后的数
    PocCmdDrvobj.Position.latitude_integer = beidou_latitude_degree();//度
    PocCmdDrvobj.Position.latitude_float = (beidou_latitude_minute()*10000+beidou_latitude_second())*10/6;//小数位合并换算
#endif
    Digital_TO_CHAR(&gps_info_buf[0],PocCmdDrvobj.Position.latitude_integer,2);
    Digital_TO_CHAR(&gps_info_buf[3],PocCmdDrvobj.Position.latitude_float,6);//转换格式二合一
    gps_info_buf[9] = 0x2C;
    Digital_TO_CHAR(&gps_info_buf[10],PocCmdDrvobj.Position.longitude_integer,3);
    gps_info_buf[13] = 0x2E;
    Digital_TO_CHAR(&gps_info_buf[14],PocCmdDrvobj.Position.longitude_float,6);//经度Longitude换算+转换格式二合一
    
    CHAR_TO_DIV_CHAR(gps_info_buf, PocCmdDrvobj.gps_info_report, 20);//20
    PocCmdDrvobj.gps_info_report[41]='0';
    PocCmdDrvobj.gps_info_report[42]='0';
    DrvGD83_UART_TxCommand(PocCmdDrvobj.gps_info_report,strlen((char const *)PocCmdDrvobj.gps_info_report));
    break;
  case PocComm_Key://7
    DrvGD83_UART_TxCommand(buf, len);
    break;
  default:
    break;
  }
  DrvMc8332_UART_TxTail();
  DrvMC8332_TxPort_SetValidable(ON);
}
#else //CDMA中兴
void ApiPocCmd_WritCommand(PocCommType id, u8 *buf, u16 len)
{
  u8 NetStateBuf[5]={0,0,0,0,0};
  u8 testData=0;
  DrvMC8332_TxPort_SetValidable(ON);
  DrvGD83_UART_TxCommand((u8 *)ucAtPocHead,strlen((char const *)ucAtPocHead));
  switch(id)
  {
  case PocComm_OpenPOC://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case PocComm_SetParam://设置账号密码
    DrvGD83_UART_TxCommand((u8 *)ucSetIPAndID,strlen((char const *)ucSetIPAndID));
    //FILE_Read(0,80,ReadBuffer);//80位

   // FILE_Read(28,22,ActiveUserID);
    //FILE_Read(0x230,250,TestReadBuffer);//0x260-0x2cc
    DrvGD83_UART_TxCommand(PocCmdDrvobj.ReadBuffer, strlen((char const *)PocCmdDrvobj.ReadBuffer));
    break;
  case PocComm_GetParam:
  case PocComm_Login:
    break;
  case PocComm_Logout:
  case PocComm_Cancel:
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case PocComm_ModifyPwd:
    break;
  case PocComm_EnterGroup:
    DrvGD83_UART_TxCommand("090000000000", 12);
    COML_HexToAsc(GroupCallingNum, NetStateBuf);
    if(strlen((char const*)NetStateBuf)==1)
    {
      NetStateBuf[1]=NetStateBuf[0];
      NetStateBuf[0]=0x30;
    }
    else
    {
      testData          =NetStateBuf[0];
      NetStateBuf[0]    =NetStateBuf[1];
      NetStateBuf[1]    =testData;
    }
    DrvGD83_UART_TxCommand(NetStateBuf, 2);
    break;
  case PocComm_Invite:
    DrvGD83_UART_TxCommand("0A0000000000", 12);
    PocCmdDrvobj.NetState.Buf2[0] = (((PersonalCallingNum+0x64)&0xf0)>>4)+0x30;	// 0x03+0x30
    PocCmdDrvobj.NetState.Buf2[1] = ((PersonalCallingNum+0x64)&0x0f)+0x30;
    DrvGD83_UART_TxCommand(PocCmdDrvobj.NetState.Buf2, 2);
    break;
  case PocComm_StartPTT://3
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case PocComm_EndPTT://4
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case PocComm_GroupListInfo://5
    DrvGD83_UART_TxCommand(ucGroupListInfo, strlen((char const *)ucGroupListInfo));
    break;
  case PocComm_UserListInfo://6
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case PocComm_Key://7
    DrvGD83_UART_TxCommand(buf, len);
    break;
  default:
    break;
  }
  DrvMc8332_UART_TxTail();
  DrvMC8332_TxPort_SetValidable(ON);
}
#endif


//从EEPROM中读取数据传给写频软件
u8 ApiPocCmd_user_info_get(u8 ** pBuf)
{
  u8 Len=0;
  Len = Combine2Hex(PocCmdDrvobj.ReadBuffer, strlen((char const *)PocCmdDrvobj.ReadBuffer), PocCmdDrvobj.ReadBuffer);
  *pBuf = PocCmdDrvobj.ReadBuffer;
  return (strlen((char const *)PocCmdDrvobj.ReadBuffer))/2;
}

//写频写入数据存入EEPROM
bool ApiPocCmd_user_info_set(u8 *pBuf, u8 len)//cTxBuf为存放ip账号密码的信息
{
	bool r;
	u8 i, uRet = 0;
	//ADRLEN_INF	adr;

	for(i = 0; i < len; i++)
	{
		if(pBuf[i] == ';')
		{
			uRet++;
		}
	}
	if(uRet >= 2)
	{
		Dec2Hex(pBuf, len, PocCmdDrvobj.NetState.LoginInfo.Buf);//将收到的数转化为字符串//LoginInfo.Buf为存放
		PocCmdDrvobj.NetState.LoginInfo.Msg.Len = len << 0x01;//为什么要len*2？
		PocCmdDrvobj.NetState.LoginInfo.Msg.bSet = ON;
		//adr = CFG_GetCurAdr(ADR_IDLocalUserInfo);
		//FILE_Write(adr.Adr,adr.Len,(u8*)(&PocCmdDrvobj.NetState.LoginInfo));
                //FILE_Write(0,PocCmdDrvobj.NetState.LoginInfo.Msg.Len,(u8*)(&PocCmdDrvobj.NetState.LoginInfo));
                FILE_Write(0,90,(u8*)(&PocCmdDrvobj.NetState.LoginInfo));
		for(i = 0; i < len; i++)
		{
			PocCmdDrvobj.NetState.LoginInfo.Buf[i] = pBuf[i];
		}
		PocCmdDrvobj.NetState.LoginInfo.Msg.Len = len;
		
		//SYS_BufReset();
#if 0//WCDMA 卓智达
		PocCmdDrvobj.WorkState.UseState.WorkUserName.NameLen = 0;
#endif
		PocCmdDrvobj.NetState.Msg.Bits.bUserInfo = 3;
		PocCmdDrvobj.NetState.Msg.Bits.bUserWrite = ON;
		r = TRUE;
	}
	else
	{
		r = FALSE;
	}
	return r;
}

#if 1 //WCDMA 卓智达
void ApiPocCmd_10msRenew(void)
{
  u8 ucId, Len,i,temp_id;
  u8 * pBuf;
  u16 ucNameId;
  u32 ucUserId;
  while((Len = DrvMC8332_PocNotify_Queue_front(&pBuf)) != 0x00)
  {
    ucId = COML_AscToHex(pBuf, 0x02);
    switch(ucId)
    {
    case 0x02://获取POC参数
      break;
    case 0x09://进入一个群组
      ucUserId =  COML_AscToHex(pBuf+2, 0x06);
      if(ucUserId==0x000000)
      {
        PocCmdDrvobj.States.current_working_status=m_group_mode;
      }
      break;
    case 0x0A://单呼某用户
      ucUserId =  COML_AscToHex(pBuf+2, 0x06);
      if(ucUserId==0x000000)
      {
        PocCmdDrvobj.States.current_working_status=m_personal_mode;
      }
      break;
    case 0x0B://按下PTT
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        PocCmdDrvobj.States.KeyPttState=1;//KeyPttState 0：未按PTT 1:按下ptt瞬间  2：按住PTT状态 3：松开PTT瞬间
        PocCmdDrvobj.States.ToneState_Intermediate=TRUE;//-----------------------延迟0.5s关闭喇叭
      }
      break;
    case 0x0C://判断松开PTT
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        PocCmdDrvobj.States.KeyPttState=3;//KeyPttState 0：未按PTT 1:按下ptt瞬间  2：按住PTT状态 3：松开PTT瞬间
        PocCmdDrvobj.States.ToneState_Intermediate=TRUE;
      }
      break;
    case 0x0D://获取群组信息
      break;
    case 0x0e://获取组成员信息
      PocCmdDrvobj.all_user_num=COML_AscToHex(pBuf+10,0x02);
      if(no_online_user()==TRUE)
      {
        VOICE_Play(NoOnlineUser);//无在线成员
        TASK_PersonalKeyModeSet(FALSE);
      }
      break;
    case 0x11://上报经纬度
      break;
    case 0x12://设置URL参数
      break;
    case 0x13://获取组成员个数
      break;
    case 0x16://设置呼叫时间限制
      break;
    case 0x28://获取北京时间
      break;
    case 0x80://群组列表
      ucNameId=COML_AscToHex(pBuf+16,0x04);
      if(ucNameId==0xffff)
      {}
      else
      {
        PocCmdDrvobj.GroupXuhao=COML_AscToHex(pBuf+8,0x04);
        PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].ID=ucNameId;//保存群组ID，从[0]开始存
        if(Len >= 24)//如果群组id后面还有群组名
        {
          PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].NameLen= (Len-24)/2;//英文字符只存一半
          if(PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].NameLen > APIPOC_GroupName_Len)
          {
            PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].NameLen = APIPOC_GroupName_Len;
          }
        }
        else//无群组名
        {
          PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].NameLen = 0x00;
        }
        for(i = 0x00; 2*i<PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].NameLen; i++)
        {
          PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].Name[2*i] = pBuf[4*i+24];//存入获取的群组名
          PocCmdDrvobj.NameInfo.AllGroupName[PocCmdDrvobj.GroupXuhao-1].Name[2*i+1] = pBuf[4*i+1+24];
        }
      }
      break;
    case 0x81://组成员列表
      ucId=COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x01)//如果成员不在线则不获取群组名
      {
        PocCmdDrvobj.offline_user_count++;
      }
      else
      {
        ucUserId=COML_AscToHex(pBuf+12,0x08);
        PocCmdDrvobj.UserXuhao=COML_AscToHex(pBuf+8,0x04);
        PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].ID=ucUserId;//保存群组ID，从[0]开始存
        if(Len >= 20)//如果群组id后面还有群组名
        {
          PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].NameLen= (Len-20)/2;//英文字符只存一半
          if(PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].NameLen > APIPOC_UserName_Len)
          {
            PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].NameLen = APIPOC_UserName_Len;
          }
        }
        else//无群组名
        {
          PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].NameLen = 0x00;
        }
        for(i = 0x00; 2*i<PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].NameLen; i++)
        {
          PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].Name[2*i] = pBuf[4*i+20];//存入获取的群组名
          PocCmdDrvobj.NameInfo.AllGroupUserName[PocCmdDrvobj.UserXuhao].Name[2*i+1] = pBuf[4*i+1+20];
        }
      }
      break;
    case 0x82://登录状态及本机用户名
      ucId = COML_AscToHex(pBuf+2, 0x02);
      switch(ucId)
      {
      case 0x00://离线
        PocCmdDrvobj.States.PocStatus=OffLine;
        break;
      case 0x01://登陆中
        PocCmdDrvobj.States.PocStatus=Landing;
        break;
      case 0x02://登录成功
        PocCmdDrvobj.States.PocStatus=LandSuccess;
        if(Len >= 12)//保存本机用户名
        {
          PocCmdDrvobj.NameInfo.LocalUserName.NameLen= (Len-12)/2;//英文字符只存一半
          if(PocCmdDrvobj.NameInfo.LocalUserName.NameLen > APIPOC_UserName_Len)
          {
            PocCmdDrvobj.NameInfo.LocalUserName.NameLen = APIPOC_UserName_Len;
          }
        }
        else//无群组名
        {
          PocCmdDrvobj.NameInfo.LocalUserName.NameLen = 0x00;
        }
        for(i = 0x00; 2*i<PocCmdDrvobj.NameInfo.LocalUserName.NameLen; i++)
        {
          PocCmdDrvobj.NameInfo.LocalUserName.Name[2*i] = pBuf[4*i+12];//存入获取的群组名
          PocCmdDrvobj.NameInfo.LocalUserName.Name[2*i+1] = pBuf[4*i+1+12];
        }
        break;
      case 0x03://注销中
        PocCmdDrvobj.States.PocStatus=Logout;
        break;
      default:
        break;
      }
      break;
    case 0x83://返回讲话用户信息
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        //自己无法讲话
      }
      else
      {
        //自己可以中断讲话人讲话
      }
      
      ucUserId=COML_AscToHex(pBuf+12,0x08);
      if(ucUserId==0xffffffff)
      {
        
      }
      else
      {
        if(Len >= 12)//如果群组id后面还有群组名
        {
          PocCmdDrvobj.NameInfo.SpeakingUserName.NameLen= (Len-12)/2;//英文字符只存一半
          if(PocCmdDrvobj.NameInfo.SpeakingUserName.NameLen > APIPOC_UserName_Len)
          {
            PocCmdDrvobj.NameInfo.SpeakingUserName.NameLen = APIPOC_UserName_Len;
          }
        }
        else//无群组名
        {
          PocCmdDrvobj.NameInfo.SpeakingUserName.NameLen = 0x00;
        }
        for(i = 0x00; 2*i<PocCmdDrvobj.NameInfo.SpeakingUserName.NameLen; i++)
        {
          PocCmdDrvobj.NameInfo.SpeakingUserName.Name[2*i] = pBuf[4*i+12];//存入
          PocCmdDrvobj.NameInfo.SpeakingUserName.Name[2*i+1] = pBuf[4*i+1+12];
        }
      }

      break;
    case 0x84://返回提示信息
      break;
    case 0x85://通知更新组或成员信息
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        ApiPocCmd_WritCommand(PocComm_GroupListInfo,0,0);
      }
      if(ucId==0x01)
      {
        ApiPocCmd_WritCommand(PocComm_UserListInfo,0,0);
      }
      break;
    case 0x86://通知用户进入群组信息
      temp_id = COML_AscToHex(pBuf+2, 0x02);
      if(temp_id==0x01)
      {
        PocCmdDrvobj.States.current_working_status=m_personal_mode;
      }
      else
      {
        PocCmdDrvobj.States.current_working_status=m_group_mode;
        TASK_PersonalKeyModeSet(FALSE);//解决单呼被结束，机器不退出单呼模式的问题
        
      }
      
      ucId = COML_AscToHex(pBuf+4, 0x02);
      if(ucId==0xff)
      {
        PocCmdDrvobj.States.GroupStats=LeaveGroup;
      }
      else
      {
        PocCmdDrvobj.States.GroupStats=EnterGroup;
        if(PocCmdDrvobj.States.current_working_status==m_group_mode)
        {
          PocCmdDrvobj.NameInfo.NowWorkingGroupName.ID=COML_AscToHex(pBuf+8,0x04);;//保存群组ID，从[0]开始存
          if(Len >= 12)//如果群组id后面还有群组名
          {
            PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen= (Len-12)/2;//英文字符只存一半
            if(PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen > APIPOC_GroupName_Len)
            {
              PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen = APIPOC_GroupName_Len;
            }
          }
          else//无群组名
          {
            PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen = 0x00;
          }
          for(i = 0x00; 2*i<PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen; i++)
          {
            PocCmdDrvobj.NameInfo.NowWorkingGroupName.Name[2*i] = pBuf[4*i+12];//存入获取的群组名
            PocCmdDrvobj.NameInfo.NowWorkingGroupName.Name[2*i+1] = pBuf[4*i+1+12];
          }
        }
      }
      MenuDisplay(Menu_RefreshAllIco);
      break;
    case 0x87://通知用户名字信息
      break;
    case 0x88://通知监听群组信息
      break;
    case 0x8A://通知接收到信息
      break;
    case 0x8B://通知音频播放状态
      ucId = COML_AscToHex(pBuf+4, 0x02);
      if(ucId==0x01)
      {
        PocCmdDrvobj.States.ReceivedVoicePlayStates=TRUE;//指示灯使用
        PocCmdDrvobj.States.ReceivedVoicePlayStatesForLED=TRUE;//指示灯使用
        PocCmdDrvobj.States.ReceivedVoicePlayStatesForDisplay=ReceivedVoiceStart;//喇叭控制/接收图标/显示呼叫用户名/使用
      }
      else if(ucId==0x00)
      {
        
        PocCmdDrvobj.States.ReceivedVoicePlayStates_Intermediate=TRUE;//喇叭控制使用
        PocCmdDrvobj.States.ReceivedVoicePlayStatesForLED=FALSE;//指示灯使用
        PocCmdDrvobj.States.ReceivedVoicePlayStatesForDisplay=ReceivedVoiceEnd;//喇叭控制/接收图标/显示呼叫用户名/使用
      }
      else
      {}
      break;
    case 0x8C://通知接收其他终端发来的消息
      break;
    default:
      break;
    }
  }
}

#else //CDMA 中兴

void ApiPocCmd_10msRenew(void)
{
  u8 ucId,i, Len;
  u8 * pBuf, ucRet;
  while((Len = DrvMC8332_PocNotify_Queue_front(&pBuf)) != 0x00)
  {
    ucId = COML_AscToHex(pBuf, 0x02);
    switch(ucId)
    {
    case 0x0A://判断讲话状态
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        if(TASK_Ptt_StartPersonCalling_Flag==TRUE)//如果按下PTT键单呼某用户
        {
          EnterPersonalCalling=TRUE;
        }
      }
      break;
    case 0x0B://判断按下PTT
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        KeyPttState=1;//KeyPttState 0：未按PTT 1:按下ptt瞬间  2：按住PTT状态 3：松开PTT瞬间
      }
      break;
    case 0x0C://判断松开PTT
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId==0x00)
      {
        KeyPttState=3;//KeyPttState 0：未按PTT 1:按下ptt瞬间  2：按住PTT状态 3：松开PTT瞬间
      }
      break;  
    case 0x0e://在线用户个数
      ucId = COML_AscToHex(pBuf+8, 0x04);
      //0e0000000007
      PocCmdDrvobj.WorkState.UseState.PttUserName.UserNum = ucId;
      if(Len==12)
      {
        if(PocCmdDrvobj.WorkState.UseState.PttUserName.UserNum==0)
        {
          PocNoOnlineMember_Flag=TRUE;
        }
        else
        { 
          PocCmdDrvobj.WorkState.UseState.PttUserName.UserNum = ucId;
        }
      }
      break;
    case 0x0d://群组个数
      ucId = COML_AscToHex(pBuf+10, 0x02);
      PocCmdDrvobj.WorkState.UseState.MainWorkGroup.GroupNum = ucId;
      break;
    case 0x80://获取工作组列表
      ucId = COML_AscToHex(pBuf+10, 0x02);
      if(Len >= 24)//如果群组id后面还有群组名
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = Len - 24;
      }
      else//无群组名
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = 0x00;
      }
      for(i = 0x00; i < PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen; i++)
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.Name[i] = pBuf[i+24];//存入获取的群组名
        PocCmdDrvobj.WorkState.UseState.Group[ucId].Name[i] = 
          PocCmdDrvobj.WorkState.UseState.WorkGroup.Name[i];
      }
      PocCmdDrvobj.WorkState.UseState.Group[ucId].NameLen = PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen;
      if(ucId==PocCmdDrvobj.WorkState.UseState.MainWorkGroup.GroupNum)
      {
      }
      POC_GetAllGroupNameStart_Flag=TRUE; 
      break;
    case 0x81://获取组内成员列表
      ucId = COML_AscToHex(pBuf+10, 0x02);//
      if(Len >= 20)//如果用户ID后面还有用户名
      {
        PocCmdDrvobj.WorkState.UseState.WorkUserName.NameLen = Len - 20;
      }
      else//无用户名
      {
        PocCmdDrvobj.WorkState.UseState.WorkUserName.NameLen = 0x00;
      }
      for(i = 0x00; i < PocCmdDrvobj.WorkState.UseState.WorkUserName.NameLen; i++)
      {
        PocCmdDrvobj.WorkState.UseState.WorkUserName.Name[i] = pBuf[i+20];//存入获取的群组名
        PocCmdDrvobj.WorkState.UseState.UserName[ucId].Name[i] = 
          PocCmdDrvobj.WorkState.UseState.WorkUserName.Name[i];
      }
      PocCmdDrvobj.WorkState.UseState.UserName[ucId].NameLen = PocCmdDrvobj.WorkState.UseState.WorkUserName.NameLen;
#if 1
          GettheOnlineMembersDone=TRUE;
#endif
      break;
    case 0x82://判断是否登录成功
      ucId = COML_AscToHex(pBuf+3, 0x01);
      if(ucId == 0x02)
      {
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.bLogin = 0x01;
        OffLineCount=0;
      }
      else if(ucId == 0x00)
      {
        OffLineCount++;
        if(OffLineCount>=5)
        {
          //ApiAtCmd_WritCommand(ATCOMM3_GD83Reset,(void*)0, 0);
          OffLineCount=0;
        }
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.bLogin = 0x00;
      }
      else
      {
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.bLogin = 0x00;
      }
      break;
/********判断是否是被呼状态******************************/
    case 0x83:
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId == 0x00)
      {
        //830000000001 07592875268df7533100f7530000
        //830000000001 310039003800300030003300300037003400370035000000
        //830000000001 310039003800300030003300300037003400370035000000
        if(Len >= 12)
        {
          PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen = Len - 12;
#if 1//尝试解决群组内被呼显示只有8位的问题
          if(InvalidCallCount==1)
          {
            InvalidCallCount=0;
          }
          else
          {
            if(PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen > APIPOC_CalledUserName_Len)
          {
            PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen = APIPOC_CalledUserName_Len;
            //解决切换群组出现话权下发指令，导致禁发 
            //PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen = 0;
          }
          POC_ReceivedVoice_Flag=TRUE;
          POC_ReceivedVoiceStart_Flag=2;//0:正常 1：收到语音 2：刚开始语音
          POC_ReceivedVoiceEnd_Flag=1;//0:正常 1：收到语音 2：刚结束语音
          POC_ReceivedVoiceEndForXTSF_Flag=1;
          }

#else
          if(PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen > APIPOC_CalledUserName_Len)
          {
            //PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen = APIPOC_CalledUserName_Len;
            //解决切换群组出现话权下发指令，导致禁发 
            PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen = 0;
          }
          else
          {
            POC_ReceivedVoice_Flag=TRUE;
            POC_ReceivedVoiceStart_Flag=2;//0:正常 1：收到语音 2：刚开始语音
            POC_ReceivedVoiceEnd_Flag=1;//0:正常 1：收到语音 2：刚结束语音
            POC_ReceivedVoiceEndForXTSF_Flag=1;
          }
#endif
        }
        for(i = 0x00; i < PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.NameLen; i++)
        {
          PocCmdDrvobj.WorkState.UseState.SpeakerRightnow.Name[i] = pBuf[i+12];//存入当前说话人的名字
        }
      }
      if(ucId == 0xff)
      {
        //Set_GreenLed(LED_ON);
        if(SwitchGroupBUG==TRUE)
        {
#if 0//解决进入个呼模式，按住PTT键不送，被呼方第一次亮绿灯能接收到语音，松开PTT绿灯常亮的问题
            POC_ReceivedVoice_Flag=FALSE;
            POC_ReceivedVoiceEnd_Flag=2;//0:正常 1：收到语音 2：刚结束语音
            POC_ReceivedVoiceEndForXTSF_Flag=2;
            POC_ReceivedVoiceStart_Flag=0;//0:正常 1：收到语音 2：刚开始语音
            api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//无发射无接收信号图标
            api_disp_all_screen_refresh();// 全屏统一刷新
            SwitchGroupBUG=FALSE;
#endif
        }
        else
        {
          POC_ReceivedVoice_Flag=FALSE;
          POC_ReceivedVoiceEnd_Flag=2;//0:正常 1：收到语音 2：刚结束语音
          POC_ReceivedVoiceEndForXTSF_Flag=2;
          //POC_ReceivedVoiceStart_Flag=0;//0:正常 1：收到语音 2：刚开始语音//尝试解决闪屏问题
        }
        //POC_ReceivedNoVoice_Flag=FALSE;
      }
      break;
/**************************************/
    case 0x8B:
      ucId = COML_AscToHex(pBuf+4, 0x02);
      if(ucId==0x00)//语音通话结束
      {
        ApiPocCmd_PlayReceivedVoice_Flag=FALSE;
      }
      if(ucId==0x01)//语音通话开始
      {
        ApiPocCmd_PlayReceivedVoice_Flag=TRUE;
      }
      if(ucId==0x03)//tone音
      {
        ApiPocCmd_Tone_Flag=TRUE;
#if 1//当收到Tone音，将ZTTS至0
        ApiAtCmd_ZTTS_Flag=FALSE;
#endif
      }

      break;
    case 0x86:
      //InvalidCallCount=1;
      POC_Receive86_Flag=TRUE;
/****************判断接入单呼**************************************************************/
      ucId = COML_AscToHex(pBuf+4, 0x02);
      if(ucId==0x0a)//接入单呼
      { 
        POC_EnterPersonalCalling_Flag=2;//用于判断进入单呼,0:正常 2：正在进入单呼 1：单呼状态
        POC_QuitPersonalCalling_Flag=1;//用于判断退出单呼
      }
      else
      {
#if 0//test测试单呼后换组不正常的BUG
        if(ucId==0x00)//收到则退出单呼（退出单呼、进组状态）
        {
          POC_EnterPersonalCalling_Flag=0;
          POC_QuitPersonalCalling_Flag=2;
        }
#else
        if(POC_QuitPersonalCalling_Flag==1)//收到则退出单呼（退出单呼、进组状态）
        {
          POC_EnterPersonalCalling_Flag=0;
          POC_QuitPersonalCalling_Flag=2;
        }
#endif
      }
/****************群组状态判断及群组信息获取**************************************************************/
#if 1//将群组名与单呼名分开
      ucId = COML_AscToHex(pBuf+10, 0x02);
      if(ucId==0xff)
      {
        if(POC_EnterPersonalCalling_Flag==1)
        {
          POC_EnterPersonalCalling_Flag=0;
          POC_QuitPersonalCalling_Flag=2;
          POC_AtEnterPersonalCalling_Flag=0;
          POC_AtQuitPersonalCalling_Flag=0;
        }
        if(POC_AtEnterPersonalCalling_Flag==1)
        {
          POC_AtEnterPersonalCalling_Flag=0;
          POC_AtQuitPersonalCalling_Flag=2;
          POC_EnterPersonalCalling_Flag=0;
          POC_QuitPersonalCalling_Flag=0;
        }
        POC_EnterGroupCalling_Flag=0;//0默认 1在群组内 2正在进入
        POC_QuitGroupCalling_Flag=2;//0默认 1在群组内 2正在退出
      }
      else//正在进入群组或单呼
      {
          if(EnterPersonalCalling==TRUE)
          {
            POC_AtEnterPersonalCalling_Flag=2;
            POC_AtQuitPersonalCalling_Flag=1;
            EnterPersonalCalling=FALSE;
            TASK_Ptt_StartPersonCalling_Flag=FALSE;
          }
#if 1//解决被呼状态下换组后按PTT提示禁发，绿灯亮
          if(POC_AtEnterPersonalCalling_Flag==0)//解决单呼模式下显示个呼名前还显示一下群组名的BUG
          {
            POC_ReceivedVoice_Flag=FALSE;
            POC_ReceivedVoiceEnd_Flag=2;//0:正常 1：收到语音 2：刚结束语音
            POC_ReceivedVoiceEndForXTSF_Flag=2;
            POC_ReceivedVoiceStart_Flag=0;//0:正常 1：收到语音 2：刚开始语音
          }

#endif
        if(PocCmdDrvobj.WorkState.UseState.Msg.Bits.PersonalCallingMode == 0x01)//如果是进入单呼模式则86存入单呼名
        {
          POC_EnterGroupCalling_Flag=2;
          POC_QuitGroupCalling_Flag=1;
            if(Len >= 12)//如果群组id后面还有群组名
            {
              PocCmdDrvobj.WorkState.UseState.PttUserName.NameLen = Len - 12;
              if(PocCmdDrvobj.WorkState.UseState.PttUserName.NameLen > APIPOC_CalledUserName_Len)
              {
                PocCmdDrvobj.WorkState.UseState.PttUserName.NameLen = APIPOC_CalledUserName_Len;
              }
            }
            else//无群组名
            {
              PocCmdDrvobj.WorkState.UseState.PttUserName.NameLen = 0x00;
            }
            for(i = 0x00; i < PocCmdDrvobj.WorkState.UseState.PttUserName.NameLen; i++)
            {
              PocCmdDrvobj.WorkState.UseState.PttUserName.Name[i] = pBuf[i+12];//存入获取的群组名
            }
        }
        else//进组存组名
        {
          POC_EnterGroupCalling_Flag=2;
          POC_QuitGroupCalling_Flag=1;
          PocCmdDrvobj.WorkState.UseState.MainWorkGroup.PresentGroupId = ucId;
          ucId = 0x00;
          for(i = 0x00; i < 0x08; i++)
          {
            PocCmdDrvobj.WorkState.UseState.WorkGroup.Id[i] = pBuf[i+4];
            PocCmdDrvobj.WorkState.UseState.MainWorkGroup.Id[i] = PocCmdDrvobj.WorkState.UseState.WorkGroup.Id[i];
            if(PocCmdDrvobj.WorkState.UseState.WorkGroup.Id[i] != 'f') //=f为离开群组
              ucId++;
          }
          if(ucId==0x00)//如果为指令代表离开群组
          {}
          else//r如果为在群组内
          {
            if(Len >= 12)//如果群组id后面还有群组名
            {
              PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = Len - 12;
              if(PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen > APIPOC_UserName_Len)
              {
                PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = APIPOC_UserName_Len;
              }
            }
            else//无群组名
            {
              PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = 0x00;
            }
            for(i = 0x00; i < PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen; i++)
            {
              PocCmdDrvobj.WorkState.UseState.WorkGroup.Name[i] = pBuf[i+12];//存入获取的群组名
              PocCmdDrvobj.WorkState.UseState.MainWorkGroup.Name[i] = PocCmdDrvobj.WorkState.UseState.WorkGroup.Name[i];
            }
            PocCmdDrvobj.WorkState.UseState.MainWorkGroup.NameLen = PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen;
          }
        }
      }

#else
      ucId = COML_AscToHex(pBuf+10, 0x02);
      if(ucId==0xff)
      {
        POC_EnterGroupCalling_Flag=0;//0默认 1在群组内 2正在进入
        POC_QuitGroupCalling_Flag=2;//0默认 1在群组内 2正在退出
      }
      else//正在进入群组
      {
        POC_EnterGroupCalling_Flag=2;
        POC_QuitGroupCalling_Flag=1;
      PocCmdDrvobj.WorkState.UseState.MainWorkGroup.PresentGroupId = ucId;

      ucId = 0x00;
      for(i = 0x00; i < 0x08; i++)
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.Id[i] = pBuf[i+4];
        PocCmdDrvobj.WorkState.UseState.MainWorkGroup.Id[i] = 
            PocCmdDrvobj.WorkState.UseState.WorkGroup.Id[i];
      if(PocCmdDrvobj.WorkState.UseState.WorkGroup.Id[i] != 'f') //=f为离开群组
        ucId++;
      }
      if(ucId==0x00)//如果为指令代表离开群组
      {
      }
      else//r如果为在群组内
      {
        if(Len >= 12)//如果群组id后面还有群组名
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = Len - 12;
        if(PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen > APIPOC_UserName_Len)
        {
          PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = APIPOC_UserName_Len;
        }
      }
      else//无群组名
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen = 0x00;
      }
      for(i = 0x00; i < PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen; i++)
      {
        PocCmdDrvobj.WorkState.UseState.WorkGroup.Name[i] = pBuf[i+12];//存入获取的群组名
        PocCmdDrvobj.WorkState.UseState.MainWorkGroup.Name[i] = 
            PocCmdDrvobj.WorkState.UseState.WorkGroup.Name[i];
      }
      PocCmdDrvobj.WorkState.UseState.MainWorkGroup.NameLen = PocCmdDrvobj.WorkState.UseState.WorkGroup.NameLen;
      }
      }
#endif
      break;
    case 0x91://通知进入某种模式（进入退出一键告警、单呼模式）
      ucId = COML_AscToHex(pBuf+2, 0x02);
      if(ucId == 0x00)
      {
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.AlarmMode = 0x00;//退出一键告警
      }
      if(ucId == 0x01)
      {
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.AlarmMode = 0x01;//进入一键告警
      }
      if(ucId == 0x02)
      {
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.PersonalCallingMode = 0x01;//进入单呼
      }
      if(ucId == 0x03)
      {
        PocCmdDrvobj.WorkState.UseState.Msg.Bits.PersonalCallingMode = 0x00;//退出单呼 
        //AUDIO_IOAFPOW(ON);//打开功放，解决'组呼模式'播报问题

      }
      break;
    default:
      break;
    }
  }
}
#endif


GroupStatsType ApiPocCmd_GroupStates(void)
{
  return PocCmdDrvobj.States.GroupStats;
}
void ApiPocCmd_GroupStatesSet(GroupStatsType a)
{
  PocCmdDrvobj.States.GroupStats=a;
}
u8 ApiPocCmd_KeyPttState(void)
{
  return PocCmdDrvobj.States.KeyPttState;
}
void ApiPocCmd_SetKeyPttState(u8 i)
{
  PocCmdDrvobj.States.KeyPttState=i;
}

bool ApiPocCmd_ReceivedVoicePlayStates(void)
{
  return PocCmdDrvobj.States.ReceivedVoicePlayStates;
}
void ApiPocCmd_ReceivedVoicePlayStatesSet(bool a)
{
  PocCmdDrvobj.States.ReceivedVoicePlayStates=a;
}

ReceivedVoicePlayStatesType ApiPocCmd_ReceivedVoicePlayStatesForDisplay(void)
{
  return PocCmdDrvobj.States.ReceivedVoicePlayStatesForDisplay;
}
void ApiPocCmd_ReceivedVoicePlayStatesForDisplaySet(ReceivedVoicePlayStatesType a)
{
  PocCmdDrvobj.States.ReceivedVoicePlayStatesForDisplay=a;
}

bool ApiPocCmd_ReceivedVoicePlayStatesIntermediate(void)//中间变量
{
  return PocCmdDrvobj.States.ReceivedVoicePlayStates_Intermediate;
}
void ApiPocCmd_ReceivedVoicePlayStatesIntermediateSet(bool a)//中间变量
{
  PocCmdDrvobj.States.ReceivedVoicePlayStates_Intermediate=a;
}

bool ApiPocCmd_ReceivedVoicePlayStatesForLED(void)
{
  return PocCmdDrvobj.States.ReceivedVoicePlayStatesForLED;
}

bool ApiPocCmd_ToneStateIntermediate(void)//中间变量
{
  return PocCmdDrvobj.States.ToneState_Intermediate;
}
void ApiPocCmd_ToneStateIntermediateSet(bool a)//中间变量
{
  PocCmdDrvobj.States.ToneState_Intermediate=a;
}
bool ApiPocCmd_ToneState(void)
{
  return PocCmdDrvobj.States.ToneState;
}
void ApiPocCmd_ToneStateSet(bool a)
{
  PocCmdDrvobj.States.ToneState=a;
}

/*****群组用户名相关调用函数****************/
u8 *GetNowWorkingGroupNameForDisplay(void)//当前群组名：显示屏
{
  u8 i;
  memset(PocCmdDrvobj.NowWorkingGroupNameBuf,0,sizeof(PocCmdDrvobj.NowWorkingGroupNameBuf));
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen;i++)
  {
    PocCmdDrvobj.NowWorkingGroupNameBuf[i]=COML_AscToHex(PocCmdDrvobj.NameInfo.NowWorkingGroupName.Name+(2*i), 0x02);
  }
  return PocCmdDrvobj.NowWorkingGroupNameBuf;
}
u8 GetNowWorkingGroupNameLenForDisplay(void)
{
  return PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen;
}

u8 *GetAllGroupNameForDisplay(u8 a)//所有群组：显示屏
{
  u8 i;
  memset(PocCmdDrvobj.AllGroupNameBuf,0,sizeof(PocCmdDrvobj.AllGroupNameBuf));
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.AllGroupName[a].NameLen;i++)
  {
    PocCmdDrvobj.AllGroupNameBuf[i]=COML_AscToHex(PocCmdDrvobj.NameInfo.AllGroupName[a].Name+(2*i), 0x02);
  }
  return PocCmdDrvobj.AllGroupNameBuf;
}
u8 *GetSpeakingUserNameForDisplay(void)//说话的用户：显示屏
{
  u8 i;
  memset(PocCmdDrvobj.SpeakingUserNameBuf,0,sizeof(PocCmdDrvobj.SpeakingUserNameBuf));
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.SpeakingUserName.NameLen;i++)
  {
    PocCmdDrvobj.SpeakingUserNameBuf[i]=COML_AscToHex(PocCmdDrvobj.NameInfo.SpeakingUserName.Name+(2*i), 0x02);
  }
  return PocCmdDrvobj.SpeakingUserNameBuf;
}
u8 *GetAllUserNameForDisplay(u8 a)//所有用户：显示屏
{
  u8 i;
  memset(PocCmdDrvobj.AllUserNameBuf,0,sizeof(PocCmdDrvobj.AllUserNameBuf));
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.AllGroupUserName[a].NameLen;i++)
  {
    PocCmdDrvobj.AllUserNameBuf[i]=COML_AscToHex(PocCmdDrvobj.NameInfo.AllGroupUserName[a].Name+(2*i), 0x02);
  }
  return PocCmdDrvobj.AllUserNameBuf;
}

u8 *GetLocalUserNameForDisplay(void)//本机用户：显示屏
{
  u8 i;
  memset(PocCmdDrvobj.LocalUserNameBuf,0,sizeof(PocCmdDrvobj.LocalUserNameBuf));
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.LocalUserName.NameLen;i++)
  {
    PocCmdDrvobj.LocalUserNameBuf[i]=COML_AscToHex(PocCmdDrvobj.NameInfo.LocalUserName.Name+(2*i), 0x02);
  }
  return PocCmdDrvobj.LocalUserNameBuf;
}

u8 *GetNowWorkingGroupNameForVoice(void)//当前群组：播报
{
  u8 i;
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.NowWorkingGroupName.NameLen;i++)
  {
    PocCmdDrvobj.NowWorkingGroupNameForVoiceBuf[4*i]    = PocCmdDrvobj.NameInfo.NowWorkingGroupName.Name[2*i];
    PocCmdDrvobj.NowWorkingGroupNameForVoiceBuf[4*i+1]  = PocCmdDrvobj.NameInfo.NowWorkingGroupName.Name[2*i+1];
    PocCmdDrvobj.NowWorkingGroupNameForVoiceBuf[4*i+2]  = 0x30;
    PocCmdDrvobj.NowWorkingGroupNameForVoiceBuf[4*i+3]  = 0x30;
  }
  return PocCmdDrvobj.NowWorkingGroupNameForVoiceBuf;
}

u8 *GetAllGroupNameForVoice(u8 a)//所有群组：播报
{
  u8 i;
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.AllGroupName[a].NameLen;i++)
  {
    PocCmdDrvobj.AllGroupNameForVoiceBuf[4*i]    = PocCmdDrvobj.NameInfo.AllGroupName[a].Name[2*i];
    PocCmdDrvobj.AllGroupNameForVoiceBuf[4*i+1]  = PocCmdDrvobj.NameInfo.AllGroupName[a].Name[2*i+1];
    PocCmdDrvobj.AllGroupNameForVoiceBuf[4*i+2]  = 0x30;
    PocCmdDrvobj.AllGroupNameForVoiceBuf[4*i+3]  = 0x30;
  }
  return PocCmdDrvobj.AllGroupNameForVoiceBuf;
}

u8 *GetAllUserNameForVoice(u8 a)//所有用户：播报
{
  u8 i;
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.AllGroupUserName[a].NameLen;i++)
  {
    PocCmdDrvobj.AllUserNameForVoiceBuf[4*i]    = PocCmdDrvobj.NameInfo.AllGroupUserName[a].Name[2*i];
    PocCmdDrvobj.AllUserNameForVoiceBuf[4*i+1]  = PocCmdDrvobj.NameInfo.AllGroupUserName[a].Name[2*i+1];
    PocCmdDrvobj.AllUserNameForVoiceBuf[4*i+2]  = 0x30;
    PocCmdDrvobj.AllUserNameForVoiceBuf[4*i+3]  = 0x30;
  }
  return PocCmdDrvobj.AllUserNameForVoiceBuf;
}

u8 *GetLocalUserNameForVoice(void)//本机用户：播报
{
  u8 i;
  for(i=0;2*i<=PocCmdDrvobj.NameInfo.LocalUserName.NameLen;i++)
  {
    PocCmdDrvobj.LocalUserNameForVoiceBuf[4*i]    = PocCmdDrvobj.NameInfo.LocalUserName.Name[2*i];
    PocCmdDrvobj.LocalUserNameForVoiceBuf[4*i+1]  = PocCmdDrvobj.NameInfo.LocalUserName.Name[2*i+1];
    PocCmdDrvobj.LocalUserNameForVoiceBuf[4*i+2]  = 0x30;
    PocCmdDrvobj.LocalUserNameForVoiceBuf[4*i+3]  = 0x30;
  }
  return PocCmdDrvobj.LocalUserNameForVoiceBuf;
}

//根据群组ID获取当前组的组索引
u16 GetNowWorkingGroupXuhao(void)
{
  u8 i;
  
  for(i=0;i<=PocCmdDrvobj.GroupXuhao;i++)
  {
    if(PocCmdDrvobj.NameInfo.NowWorkingGroupName.ID==PocCmdDrvobj.NameInfo.AllGroupName[i].ID)
    {
      return i;
    }
  }
  return 0;
}
//获取群组的最大组索引，群组个数
u16 GetAllGroupNum(void)
{
  return PocCmdDrvobj.GroupXuhao;
}

//获取用户的最大组索引，用户个数
u16 GetAllUserNum(void)
{
  return PocCmdDrvobj.UserXuhao+1;
}

//判断是否群组成员在线
static bool no_online_user(void)
{
  if(PocCmdDrvobj.all_user_num!=0)
  {
    if(PocCmdDrvobj.all_user_num==PocCmdDrvobj.offline_user_count)
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
  return FALSE;
}

//判断当前工作模式（单呼或组呼）
/************************/
working_status_type get_current_working_status(void)
{
  return PocCmdDrvobj.States.current_working_status;
}

//选择当前显示的是组呼名还是单呼临时群组名
void get_screen_display_group_name(void)
{
 if(PocCmdDrvobj.States.current_working_status==m_group_mode)//组呼模式
  {
    api_lcd_pwr_on_hint(0,2,"                ");
    api_lcd_pwr_on_hint(0,2,GetNowWorkingGroupNameForDisplay());
  }
  else //单呼模式
  {
    api_lcd_pwr_on_hint(0,2,"                ");
    api_lcd_pwr_on_hint(0,2,"Temporary groups");//Temporary groups临时群组
  }
}

PocStatesType poccmd_states_poc_status(void)
{
  return PocCmdDrvobj.States.PocStatus;
}

u32 poc_longitude_integer(void)
{
  return PocCmdDrvobj.Position.longitude_integer;
}

u32 poc_longitude_float(void)
{
  return PocCmdDrvobj.Position.longitude_float;
}

u32 poc_latitude_integer(void)
{
  return PocCmdDrvobj.Position.latitude_integer;
}

u32 poc_latitude_float(void)
{
  return PocCmdDrvobj.Position.latitude_float;
}