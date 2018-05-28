#define DELLABEL
#include "AllHead.h"

#define DEL10MSLEN		0x0A
#define DEL_IDLE		0x00
#define DEL_RUN			0x01

typedef struct {
  union {
    struct {
      u16 b1ms  	: 1;
      u16 b10ms 	: 1;
      u16 b100ms	: 1;
      u16 b500ms	: 1;
      u16 b1S		: 1;
      u16 bTimeSet	: 1;
      u16 bTime0	: 1;
      u16 bTime1	: 1;
      u16 b500Alternate : 1;
      u16		: 7;
    }Bit;
    u16 Byte;
  }Msg;
  u8   c10msLen;
  u8   c100msLen;
  u8   c500msLen;
  u8   c1SLen;
  u8   c2SLen;
  u16  iTimer0;
  u16  iTimer1;
  struct{
    u8 TimeCount_Light;
    u8 CSQTimeCount;
    u8 WriteFreqTimeCount;
    u8 LobatteryTask_StartCount;
    u8 PrimaryLowPowerCount;
    u8 SignalPoorCount;
    u8 TimeCount2;
    u8 CIMICount;
    u8 NoCardCount;
    u8 PPPStatusOpenCount;
    u8 ToneStateCount;
    u8 ReceivedVoicePlayStatesCount;
    u8 beidou_valid_count;
    u8 poc_status_count;
    u8 choose_write_freq_or_gps_count;
    u8 receive_sos_statas_count;
    u8 ztts_states_intermediate_count;
    u8 alarm_count;
  }Count;
  u8 BacklightTimeBuf[1];//背光灯时间(需要设置进入eeprom)
  u8 KeylockTimeBuf[1];//键盘锁时间(需要设置进入eeprom)
}DEL_DRV;

static DEL_DRV	DelDrvObj;
static void DEL_100msProcess(void);
static void DEL_500msProcess(void);
static void DEL_1msProcess(void);
static void DEL_10msProcess(void);
static void DEL_TimerRenew(void);

void DEL_PowerOnInitial(void)//原瑞撒單片機多長時間進一次中斷
{
  Tim3_Timer_Init();
  C_TEST_OUT_SET();
  DelDrvObj.Msg.Byte 	= 0x00;
  DelDrvObj.c10msLen  = DEL10MSLEN;
  DelDrvObj.c100msLen = 0x0A;
  DelDrvObj.c500msLen = 0x05;
  DelDrvObj.c1SLen    = 0x01;
  DelDrvObj.c2SLen    = 0x02;
  
  DelDrvObj.Count.TimeCount_Light = 0;
  DelDrvObj.Count.CSQTimeCount = 0;
  DelDrvObj.Count.WriteFreqTimeCount = 0;
  DelDrvObj.Count.LobatteryTask_StartCount = 0;
  DelDrvObj.Count.PrimaryLowPowerCount = 0;
  DelDrvObj.Count.SignalPoorCount = 0;
  DelDrvObj.Count.TimeCount2 = 0;
  DelDrvObj.Count.CIMICount = 0;
  DelDrvObj.Count.NoCardCount = 0;
  DelDrvObj.Count.PPPStatusOpenCount = 0;
  DelDrvObj.Count.ToneStateCount = 0;
  DelDrvObj.Count.ReceivedVoicePlayStatesCount = 0;
  DelDrvObj.Count.beidou_valid_count = 0;
  DelDrvObj.Count.poc_status_count = 0;
  DelDrvObj.Count.choose_write_freq_or_gps_count = 0;
  DelDrvObj.Count.receive_sos_statas_count = 0;
  DelDrvObj.Count.ztts_states_intermediate_count = 0;
  return;
}

void DEL_Interrupt(void)
{
  DelDrvObj.c10msLen--;
  DelDrvObj.Msg.Bit.b1ms = DEL_RUN;
  //enableInterrupts();
  if (DelDrvObj.c10msLen == 0x00) //10ms interrupt process
  {
    DEL_TimerRenew();//delay timer renew process
    DelDrvObj.Msg.Bit.b10ms = DEL_RUN;
    DelDrvObj.c10msLen = DEL10MSLEN;
    DelDrvObj.c100msLen--;
    if (DelDrvObj.c100msLen == 0x00) //100ms interrupt process
    {
      DelDrvObj.Msg.Bit.b100ms = DEL_RUN;
      DelDrvObj.c100msLen = 0x0A;
      DelDrvObj.c500msLen--;
      if (DelDrvObj.c500msLen == 0x00) //500ms interrupt process
      {	
        DelDrvObj.Msg.Bit.b500ms = DEL_RUN;
        DelDrvObj.c500msLen = 0x05;
        DelDrvObj.c1SLen--;
        if (DelDrvObj.c1SLen == 0x00) //1s interrupt process
        {
          DelDrvObj.Msg.Bit.b1S = DEL_RUN;
          DelDrvObj.c1SLen = 0x02;	
          DelDrvObj.c2SLen--;
          if (DelDrvObj.c2SLen == 0x00)	//2s interrupt process
          {
            //DelDrvObj.Msg.Bit.b2S = DEL_RUN;
            DelDrvObj.c2SLen = 0x02;
          }
        }
      }
    }
  }
  return;
}

void DEL_Renew(void) 
{
  DEL_1msProcess();
  DEL_10msProcess();
  DEL_100msProcess();
  DEL_500msProcess();
  return;
}

void DEL_Soft1ms(u16 iLen) 
{
  u16 i;
  for(; iLen > 0; iLen--)
  {
    for( i = 0 ; i < 250; i++)
    {
      nop();
      nop();
      nop();
      nop();
      nop();
    }
  }
  return;
}

bool DEL_SetTimer(u8 cId,u16 iLen)
{
  DelDrvObj.Msg.Bit.bTimeSet = 0x01;
  switch (cId)
  {
  case 0x00:
    if(iLen == 0x00)
    {
      DelDrvObj.Msg.Bit.bTime0 = DEL_IDLE;
    }
    else
    {
      DelDrvObj.Msg.Bit.bTime0 = DEL_RUN;
    }
    DelDrvObj.iTimer0 = iLen;
    break;
  case 0x01:
    if(iLen == 0x00)
    {
      DelDrvObj.Msg.Bit.bTime1 = DEL_IDLE;
    }
    else
    {
      DelDrvObj.Msg.Bit.bTime1 = DEL_RUN;
    }
    DelDrvObj.iTimer1 = iLen;
    break;
  default:
    return FALSE;
  }
  DelDrvObj.Msg.Bit.bTimeSet = 0x00;
  return TRUE;
}

bool DEL_GetTimer(u8 cId)
{
	bool r;

	r = FALSE;
	//1
        switch (cId)
	{
	case 0x00:
		if ((DelDrvObj.Msg.Bit.bTime0 == DEL_RUN) && (DelDrvObj.iTimer0 == 0x00))
		{
			r = TRUE;
			DelDrvObj.Msg.Bit.bTime0 = DEL_IDLE;
		}
		break;
	case 0x01:
		if ((DelDrvObj.Msg.Bit.bTime1 == DEL_RUN) && (DelDrvObj.iTimer1 == 0x00))
		{
			r = TRUE;
			DelDrvObj.Msg.Bit.bTime1 = DEL_IDLE;			
		}
		break;
	default:
		break;
	}
	return r;
}

static void DEL_TimerRenew(void)
{
  if(DelDrvObj.Msg.Bit.bTimeSet == 0x00)
  {
    if (DelDrvObj.iTimer0 != 0x00)
    {
      DelDrvObj.iTimer0--;
    }
    if (DelDrvObj.iTimer1 != 0x00)
    {
      DelDrvObj.iTimer1--;
    }
  }
  return;
}

static void DEL_100msProcess(void)
{
  if (DelDrvObj.Msg.Bit.b100ms == DEL_RUN)
  {
    DelDrvObj.Msg.Bit.b100ms = DEL_IDLE;
    LED_IntOutputRenew();//LED output renew process
#ifdef BEIDOU
    ApiBeidou_Get_location_Information();
#else
    //ApiAtCmd_Get_location_Information();
#endif
    //ApiAtCmd_Get_DateTime_Information();
    ApiGpsCmd_100msRenew();
    if(DelDrvObj.Msg.Bit.b500Alternate == DEL_IDLE)
    {
      DelDrvObj.Msg.Bit.b500Alternate = DEL_RUN;
      ApiAtCmd_100msRenew();
    }
    else
    {
      DelDrvObj.Msg.Bit.b500Alternate = DEL_IDLE;
      //ApiPocCmd_1sRenew();
    }
  }
  return;
}
#if 1//WCDMA 卓智达
static void DEL_500msProcess(void)			//delay 500ms process server
{
  if (DelDrvObj.Msg.Bit.b500ms == DEL_RUN) 
  {
    DelDrvObj.Msg.Bit.b500ms = DEL_IDLE;
    VOICE_1sProcess();
    DelDrvObj.Count.TimeCount_Light++;
    DelDrvObj.Count.CSQTimeCount++;
/*********开机检测SIM卡*************/
    if(ApiAtCmd_bStartingUp()==1&&ApiAtCmd_bCardIn()==0)
    {
      DelDrvObj.Count.CIMICount++;
      if(DelDrvObj.Count.CIMICount>2*3)
      {
        ApiAtCmd_WritCommand(ATCOMM_CIMI,0,0);
        DelDrvObj.Count.CIMICount=0;
      }
      //检测到未插卡
      if(ApiAtCmd_bNoCard()==1)
      {
        DelDrvObj.Count.NoCardCount++;
        if(DelDrvObj.Count.NoCardCount>2*8)
        {
          DelDrvObj.Count.NoCardCount=0;
          VOICE_Play(NoSimCard);
          api_lcd_pwr_on_hint(0,2,"No SIM Card     ");
        }
      } 
    }
/*********定时5s发一次[AT+CSQ?]*************************************************/
    if(DelDrvObj.Count.CSQTimeCount>=2*5)
    {
      DelDrvObj.Count.CSQTimeCount=0;
      ApiAtCmd_WritCommand(ATCOMM_CSQ, (void*)0, 0);
      
      if(GetTaskId()==Task_Start&&task_status_account_config()==FALSE&&ApiAtCmd_CSQValue()<25)//如果处于开机状态、未写入账号状态、网络信号小于25状态
      {
        VOICE_Play(NetworkSearching);
        api_lcd_pwr_on_hint(0,2,"Network Search  ");
      }

      HDRCSQSignalIcons();
    }
/*******检测PPP链接**********/
    if(ApiAtCmd_bStartingUp()==1&&ApiAtCmd_bCardIn()==1)
    {
      if(ApiAtCmd_bPPPStatusOpen()==0&&ApiAtCmd_CSQValue()>=25)
      {
        DelDrvObj.Count.PPPStatusOpenCount++;
        if(DelDrvObj.Count.PPPStatusOpenCount>2*2)
        {
          ApiAtCmd_WritCommand(ATCOMM_ZPPPOPEN,0,0);
          DelDrvObj.Count.PPPStatusOpenCount=0;
        }
      }
      else
      {
        DelDrvObj.Count.PPPStatusOpenCount=0;
      }
    }
/******喇叭控制相关函数************************/
    if(ApiAtCmd_bZTTSStates_Intermediate()==1)//语音播报喇叭延迟2秒关闭
    {
      DelDrvObj.Count.ztts_states_intermediate_count++;
      if(DelDrvObj.Count.ztts_states_intermediate_count>2*2)
      {
        set_ApiAtCmd_bZTTSStates_Intermediate(0);
        set_ApiAtCmd_bZTTSStates(0);
        DelDrvObj.Count.ztts_states_intermediate_count = 0;
      }
    }
    
    if(ApiPocCmd_ReceivedVoicePlayStatesIntermediate()==TRUE)//对讲语音
    {
      DelDrvObj.Count.ReceivedVoicePlayStatesCount++;  
      if(DelDrvObj.Count.ReceivedVoicePlayStatesCount>1)
      {
        ApiPocCmd_ReceivedVoicePlayStatesIntermediateSet(FALSE);
        ApiPocCmd_ReceivedVoicePlayStatesSet(FALSE);
        DelDrvObj.Count.ReceivedVoicePlayStatesCount=0;
      }
    }
    
    if(ApiPocCmd_ToneStateIntermediate()==TRUE)//bb音
    {
      DelDrvObj.Count.ToneStateCount++;
      if(DelDrvObj.Count.ToneStateCount>1)
      {
        ApiPocCmd_ToneStateSet(FALSE);
        ApiPocCmd_ToneStateIntermediateSet(FALSE);
        DelDrvObj.Count.ToneStateCount=0;
      }
    }
/*****低于设定值播报网络信号弱*************************************************************************/
    
/******登录状态下的低电报警**********************************************/
    if(LobatteryTask_StartFlag==TRUE)
    {
      DelDrvObj.Count.LobatteryTask_StartCount++;
      if(DelDrvObj.Count.LobatteryTask_StartCount==1)
      {
        VOICE_Play(PowerLowPleaseCharge);
      }
      if(DelDrvObj.Count.LobatteryTask_StartCount>2*5)
      {
        DelDrvObj.Count.LobatteryTask_StartCount=0;
        LobatteryTask_StartFlag=FALSE;
      }
    }
/*****进入写频状态5s后将写频标志位清零****************/
    if(WriteFreq_Flag==TRUE)
    {
      if(DelDrvObj.Count.WriteFreqTimeCount>=2*6)
      {
        WriteFreq_Flag=FALSE;
        DelDrvObj.Count.WriteFreqTimeCount=0;
      }
      DelDrvObj.Count.WriteFreqTimeCount++;
    }  
/*********初级电量报警30s播报一次********************************/
    if(PrimaryLowPower_Flag==TRUE)
    {
      DelDrvObj.Count.PrimaryLowPowerCount++;
      if(DelDrvObj.Count.PrimaryLowPowerCount>=2*30)
      {
        DelDrvObj.Count.PrimaryLowPowerCount=0;
        VOICE_Play(LowBattery);
        PrimaryLowPower_Flag=FALSE;
      }
    }
/********收到0x8a则进入一键报警********/
    if(poc_receive_sos_statas()==TRUE)
    {
      BEEP_SetOutput(BEEP_IDPowerOff,ON,0x00,TRUE);
      ApiPocCmd_ToneStateSet(TRUE);
      AUDIO_IOAFPOW(ON);
      DelDrvObj.Count.receive_sos_statas_count++;
      if(DelDrvObj.Count.receive_sos_statas_count>=2*5)
      {
        set_poc_receive_sos_statas(FALSE);
        DelDrvObj.Count.receive_sos_statas_count=0;
        BEEP_SetOutput(BEEP_IDPowerOff,OFF,0x00,TRUE);
        ApiPocCmd_ToneStateSet(FALSE);
        AUDIO_IOAFPOW(OFF);
      }
    }
    else
    {
      DelDrvObj.Count.receive_sos_statas_count=0;
    }
    
/*******收到离线指令过1分钟未登陆重启*******/
    if(poccmd_states_poc_status()==OffLine)
    {
      DelDrvObj.Count.poc_status_count++;
      if(DelDrvObj.Count.poc_status_count>2*60)
      {
        DelDrvObj.Count.poc_status_count=0;
        main_init();//重启
      }
    }
    else
    {
      DelDrvObj.Count.poc_status_count=0;
    }
/****登陆成功一分钟后禁用写频功能、开启外部定位上报模式*********/
    if(GetTaskId()==Task_NormalOperation)
    {
      DelDrvObj.Count.choose_write_freq_or_gps_count++;
      if(DelDrvObj.Count.choose_write_freq_or_gps_count>2*60)
      {
#if 1//测试读写频功能暂时屏蔽
        GPIO_WriteLow(GPIOB,GPIO_PIN_3);//NFC
        GPIO_WriteHigh(GPIOB,GPIO_PIN_4);//北斗
#endif
        DelDrvObj.Count.choose_write_freq_or_gps_count = 2*60+1;
      }
    }
    else
    {
      DelDrvObj.Count.choose_write_freq_or_gps_count = 0;
    }
/****定位成功后5s上报一次定位*****/
    if(beidou_valid()==TRUE)
    {
      DelDrvObj.Count.beidou_valid_count++;
      if(DelDrvObj.Count.beidou_valid_count>2*5)
      {
        ApiPocCmd_WritCommand(PocComm_SetGps,0,0);
        DelDrvObj.Count.beidou_valid_count=0;
      }
    }
    else
    {
      DelDrvObj.Count.beidou_valid_count=0;
    }
/***********************************************************/
    FILE_Read(0x236,1,DelDrvObj.BacklightTimeBuf);//背光时间【秒】
    FILE_Read(0x247,1,DelDrvObj.KeylockTimeBuf);//键盘锁时间【秒】
    BacklightTimeCount=5*DelDrvObj.BacklightTimeBuf[0];
    if(DelDrvObj.KeylockTimeBuf[0]==0)
      KeylockTimeCount=200*2;//如果=200则永远不锁屏
    else
      KeylockTimeCount=5*DelDrvObj.KeylockTimeBuf[0];
    if(DelDrvObj.Count.TimeCount_Light>=2*BacklightTimeCount)//10s
    {
      MCU_LCD_BACKLIGTH(OFF);//关闭背光灯
      DelDrvObj.Count.TimeCount_Light=2*BacklightTimeCount;
    }
    else
    {
      MCU_LCD_BACKLIGTH(ON);//打开背光灯
    }
    if(NumberKeyboardPressDown_flag==TRUE||LockingState_EnterOK_Flag==TRUE)
    {
      DelDrvObj.Count.TimeCount_Light=0;//背光灯计数器清零
      //NumberKeyboardPressDown_flag=FALSE;
    }
    
    if(GetTaskId()==Task_NormalOperation)
    {
      if(KeylockTimeCount==200*2)
      {
        TimeCount=0;
      }
      else
      {
        TimeCount++;
        if(TimeCount>=KeylockTimeCount*2) //超时则锁屏
        {
          if(TimeCount==KeylockTimeCount*2)
          {
            LockingState_Flag=TRUE;//超时锁定标志位
            MenuDisplay(Menu_RefreshAllIco);
            get_screen_display_group_name();//选择显示当前群组昵称（群组或单呼临时群组）
            api_disp_all_screen_refresh();// 全屏统一刷新

            MenuModeCount=1;
            TheMenuLayer_Flag=0;
            MenuMode_Flag=0;
            ApiMenu_SwitchGroup_Flag=0;
            ApiMenu_SwitchCallUser_Flag=0;
            ApiMenu_SwitchOnlineUser_Flag=0;
            ApiMenu_GpsInfo_Flag=0;
            ApiMenu_BacklightTimeSet_Flag=0;
            ApiMenu_KeylockTimeSet_Flag=0;
            ApiMenu_NativeInfo_Flag=0;
            ApiMenu_BeiDouOrWritingFrequency_Flag=0;
          }
          TimeCount=KeylockTimeCount*2+1;
        }
        else
        {
        }
        if(NumberKeyboardPressDown_flag==TRUE&&TimeCount>=KeylockTimeCount*2)//超过10秒后再按按键提示“按OK键再按*键”
        {
          DelDrvObj.Count.TimeCount2++;
          api_lcd_pwr_on_hint(0,2,"Press OK then # ");//
          if(DelDrvObj.Count.TimeCount2>=2)//0.5s
          {
            DelDrvObj.Count.TimeCount2=0;
            NumberKeyboardPressDown_flag=FALSE;
            MenuDisplay(Menu_Locking_NoOperation);
          }
        }
        if(LockingState_EnterOK_Flag==TRUE)//锁定界面按下OK键
        {                                                           
          TimeCount3++;//解锁成功也应该至零，
          //MCU_LCD_BACKLIGTH(ON);//打开背光灯
          if(TimeCount3>=4*2)//3s
          {
            TimeCount3=0;
            //MCU_LCD_BACKLIGTH(OFF);//关闭背光灯
            LockingState_EnterOK_Flag=FALSE;
            MenuDisplay(Menu_Locking_NoOperation);
          }
        }
      }
      if(NumberKeyboardPressDown_flag==TRUE&&TimeCount<KeylockTimeCount*2)//当数字数字键盘按下
      {
        TimeCount=0;//当有按键按下，计数器清零
        NumberKeyboardPressDown_flag=FALSE;
      }
      }
      
        
       
   //   break;
   // default:
   //   break;
   // }
  }
  return;
}
#else
static void DEL_500msProcess(void)			//delay 500ms process server
{
  u8 i;
  u8 ShowTimeBuf1[6]={0,0,0,0,0,0};
  if (DelDrvObj.Msg.Bit.b500ms == DEL_RUN) 
  {
    DelDrvObj.Msg.Bit.b500ms = DEL_IDLE;
    VOICE_1sProcess();
    DEL_500ms_Count++;
    DEL_500ms_Count2++;
    TimeCount_Light++;
    DelDrvObj.Count.CSQTimeCount++;
    GetAllGroupMemberNameCount++;
/******报警键标志位，时间显示使用**********/
    if(KEY_4_Flag==TRUE)
    {
      KEY_4Count++;
      if(KEY_4Count>2*2)
      {
        KEY_4Count=0;
        KEY_4_Flag=FALSE;
      }
    }
    else
    {
      KEY_4Count=0;
    }
/******进入群组模式5秒显示时间*******************/
    if(POC_GetAllGroupNameDone_Flag==TRUE&&
       MenuMode_Flag==0&&
       POC_EnterPersonalCalling_Flag==0&&
       POC_QuitPersonalCalling_Flag==0&&
       POC_AtEnterPersonalCalling_Flag==0&&
       POC_AtQuitPersonalCalling_Flag==0&&
       KEY_4_Flag==FALSE&&
       KeyDownUpChoose_GroupOrUser_Flag==0)
    {
      ShowTimeCount++;
      if(ShowTimeCount>2*5)
      {
        ShowTime_Flag=TRUE;
        ShowTimeCount=11;
        if(Data_Time0()<=0x09&&Data_Time1()<=0x09)
        {
          ShowTimeBuf1[0]='0';
          COML_HexToAsc(Data_Time0(),ShowTimeBuf1+1);
          ShowTimeBuf1[2]=':';
          ShowTimeBuf1[3]='0';
          COML_HexToAsc(Data_Time1(),ShowTimeBuf1+4);
        }
        else if(Data_Time0()<=0x09&&Data_Time1()>0x09)
        {
          ShowTimeBuf1[0]='0';
          COML_HexToAsc(Data_Time0(),ShowTimeBuf1+1);
          ShowTimeBuf1[2]=':';
          COML_HexToAsc(Data_Time1(),ShowTimeBuf1+3);
          COML_StringReverse(2,ShowTimeBuf1+3);
        }
        else if(Data_Time0()>0x09&&Data_Time1()<=0x09)
        {
          COML_HexToAsc(Data_Time0(),ShowTimeBuf1);
          COML_StringReverse(2,ShowTimeBuf1);
          ShowTimeBuf1[2]=':';
          ShowTimeBuf1[3]='0';
          COML_HexToAsc(Data_Time1(),ShowTimeBuf1+4);
        }
        else//
        {
          COML_HexToAsc(Data_Time0(),ShowTimeBuf1);
          COML_StringReverse(2,ShowTimeBuf1);
          ShowTimeBuf1[2]=':';
          COML_HexToAsc(Data_Time1(),ShowTimeBuf1+3);
          COML_StringReverse(2,ShowTimeBuf1+3);
        }
        ShowTimeBuf1[5]='\0';
        api_lcd_pwr_on_hint7(ShowTimeBuf1);
      }
    }
    else
    {
      ShowTime_Flag=FALSE;
      ShowTimeCount=0;
      if(MenuMode_Flag!=1)
      {
        if(NetworkType_2Gor3G_Flag==3)
          api_disp_icoid_output( eICO_IDEmergency, TRUE, TRUE);//3G图标
        else
          api_disp_icoid_output( eICO_IDPOWERL, TRUE, TRUE);//图标：2G
        if(VoiceType_FreehandOrHandset_Flag==0)
          api_disp_icoid_output( eICO_IDTemper, TRUE, TRUE);//免提模式
        else
          api_disp_icoid_output( eICO_IDMONITER, TRUE, TRUE);//听筒模式图标
        if(PersonCallIco_Flag==0)
          api_disp_icoid_output( eICO_IDPOWERM, TRUE, TRUE);//显示组呼图标
        else
          api_disp_icoid_output( eICO_IDPOWERH, TRUE, TRUE);//显示个呼图标
        if(KeyDownUpChoose_GroupOrUser_Flag==0)
          api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//空图标-与选对应
        else
          api_disp_icoid_output( eICO_IDLOCKED, TRUE, TRUE);//选
      }

    }
    
/*****5秒喇叭开启则关闭喇叭**************/
    if(ApiAtCmd_ZTTS_Flag==TRUE)
    {
      ApiAtCmd_ZTTSCount++;
      if(ApiAtCmd_ZTTSCount>2*15)
      {
        ApiAtCmd_ZTTS_Flag=FALSE;
        ApiAtCmd_ZTTSCount=0;
      }
    }
    else
    {
      ApiAtCmd_ZTTSCount=0;
    }
/*****解决进入单呼模式但未按PTT的异常状态问题，进入单呼计时30s，则退出***************/
    /*if(POC_AtEnterPersonalCalling_Flag==1&&POC_AtQuitPersonalCalling_Flag==1)
    {
      PersonalCallingCount++;
      if(PersonalCallingCount>30*2)
      {
        PersonalCallingCount=0;
      }
    }*/
/******开机获取群组信息后2s按键生效***************/
    if(POC_GetAllGroupNameStart_Flag==TRUE)
    {
      POC_GetAllGroupNameDoneCount++;
      if(POC_GetAllGroupNameDoneCount>4)
      {
        POC_GetAllGroupNameStart_Flag=FALSE;
        POC_GetAllGroupNameDoneCount=0;
        POC_GetAllGroupNameDone_Flag=TRUE;
      }
    }
/*******1分钟获取一次群组成员**********************************************/
    if(GetAllGroupMemberNameCount>2*60)
    {
      ApiPocCmd_WritCommand(PocComm_UserListInfo,ucRequestUserListInfo,strlen((char const *)ucRequestUserListInfo));
      GetAllGroupMemberNameCount=0;
    }
/**********若指令发出无回音则处于升级状态**********************************/
   /* if(UpgradeNoATReturn_Flag==TRUE)
    {
      UpgradeNoATReturn_Count++;
      if(UpgradeNoATReturn_Count>=4)
      {
        UpgradeNoATReturn_Count=0;
        UpgradeNoATReturn_Flag2=TRUE;
      }
    }
    else
    {
      UpgradeNoATReturn_Count=0;
    }*/
/*********按个呼键未获取到在线成员，超时退回组呼模式*************************/
    if(Key_PersonalCalling_Flag==1&&GettheOnlineMembersDone==FALSE)
    {
      GetNoOnlineMembersCount++;
      if(GetNoOnlineMembersCount>2*3)
      {
        GetNoOnlineMembersCount=0;
        Key_PersonalCalling_Flag=0;//进入组呼标志位
        api_lcd_pwr_on_hint("                ");//清屏
        //api_lcd_pwr_on_hint(HexToChar_MainGroupId());//显示当前群组ID
        api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
        MenuMode_Flag=0;
        //用于PTT键及上下键返回默认状态
        KeyUpDownCount=0;
        KeyDownUpChoose_GroupOrUser_Flag=0;//解决（个呼键→返回键→OK或PTT）屏幕显示错误的BUG
      }
    }
    else
    {
      if(GettheOnlineMembersDone==TRUE)
      {
        GetNoOnlineMembersCount=0;
      }
    }
/*******无在线成员处理*******************/
    if(PocNoOnlineMember_Flag==TRUE)
    {
      PocNoOnlineMemberCount++;
      if(PocNoOnlineMemberCount>2*1)
      {
        PocNoOnlineMemberCount=0;
        PocNoOnlineMember_Flag=FALSE;
        PocNoOnlineMember_Flag2=TRUE;
      }
    }
/******登录状态下的低电报警**********************************************/
    if(LobatteryTask_StartFlag==TRUE)
    {
      DelDrvObj.Count.LobatteryTask_StartCount++;
      if(DelDrvObj.Count.LobatteryTask_StartCount==1)
      {
        VOICE_SetOutput(ATVOICE_FreePlay,"f78b45513575",12);//电量低请充电
      }
      if(DelDrvObj.Count.LobatteryTask_StartCount>2*5)
      {
        DelDrvObj.Count.LobatteryTask_StartCount=0;
        LobatteryTask_StartFlag=FALSE;
      }
    }
/**********防呆，解决异常禁发问题，常亮绿灯****************************/
    if(POC_ReceivedVoice_Flag==TRUE)
    {
      POC_ReceivedVoiceCount++;
      if(POC_ReceivedVoiceCount>2*30)
      {
        POC_ReceivedVoiceCount=0;
        POC_ReceivedVoice_Flag=FALSE;
        POC_ReceivedVoiceEnd_Flag=2;//0:正常 1：收到语音 2：刚结束语音
        POC_ReceivedVoiceEndForXTSF_Flag=2;
        POC_ReceivedVoiceStart_Flag=0;//0:正常 1：收到语音 2：刚开始语音
      }
    }
    else
    {
      POC_ReceivedVoiceCount=0;
    }
/*********受到关喇叭指令延迟两秒关闭******************************************/
#if 0
    //if(GetTaskId()==Task_NormalOperation)
    {
      if(ApiAtCmd_TrumpetVoicePlay_Flag==2)
      {
        ApiAtCmd_TrumpetVoicePlayCount++;
        if(ApiAtCmd_TrumpetVoicePlayCount>2*2)//原来2s，现在改为0.5s
        {
          ApiAtCmd_TrumpetVoicePlay_Flag=0;
          ApiAtCmd_TrumpetVoicePlayCount=0;
          AUDIO_IOAFPOW(OFF);
        }
      }
    }
#endif
/*******初始化去延时用定时**************************/
    if(TaskStartDeleteDelay1==2)//中兴易洽广域对讲
    {
      TaskStartDeleteDelay1Count++;
      if(TaskStartDeleteDelay1Count>=6)
      {
        TaskStartDeleteDelay1Count=0;
        TaskStartDeleteDelay2=1;
      }
    }
    if(TaskStartDeleteDelay3==2)//检不到卡
    {
      TaskStartDeleteDelay3Count++;
      if(TaskStartDeleteDelay3Count>=2*10)
      {
        TaskStartDeleteDelay3Count=0;
        TaskStartDeleteDelay3=1;
      }
    }
    if(TaskStartDeleteDelay4==2)//播报账号信息
    {
      TaskStartDeleteDelay4Count++;
      if(TaskStartDeleteDelay4Count>=2*4)
      {
        TaskStartDeleteDelay4Count=0;
        TaskStartDeleteDelay5=1;
      }
    }
    if(TaskStartDeleteDelay6==0)
    {
      TaskStartDeleteDelay6Count++;
      if(TaskStartDeleteDelay6Count>=2*6)//6秒拨一次搜索网络
      {
        TaskStartDeleteDelay6Count=0;
        TaskStartDeleteDelay6=1;
      }
    }
/*******解决呼叫方第一次呼叫，被呼方不亮绿灯的问题**********************************************/
#if 0//去除
    if(POC_ReceivedNoVoice_Flag==TRUE)
    {
      POC_ReceivedNoVoiceCount++;
      if(POC_ReceivedNoVoiceCount>=2)
      {
        POC_ReceivedNoVoiceCount=2;
        Set_GreenLed(LED_ON);
        api_disp_icoid_output( eICO_IDVOX, TRUE, TRUE);//接收信号图标
        api_disp_all_screen_refresh();// 全屏统一刷新
      }
    }
    else
    {
      POC_ReceivedNoVoiceCount=0;
    }
#endif

/*****************************************************/
#if 0
    if(UpDownSwitching_Flag==TRUE)
    {
      UpDownSwitchingCount++;
      if(UpDownSwitchingCount>1)
      {
        UpDownSwitchingCount=0;
        UpDownSwitching_Flag=FALSE;
        AUDIO_IOAFPOW(OFF);
      }
    }
#endif
/*****进入写频状态5s后将写频标志位清零****************/
    if(WriteFreq_Flag==TRUE)
    {
      if(DelDrvObj.Count.WriteFreqTimeCount>=10)
      {
        WriteFreq_Flag=FALSE;
        DelDrvObj.Count.WriteFreqTimeCount=0;
      }
      DelDrvObj.Count.WriteFreqTimeCount++;
    }
/***************/
    if(KeyDownUpChoose_GroupOrUser_Flag==3)
    {
      if(EnterKeyTimeCount>=4)
      {
        EnterKeyTimeCount=0;
        EnterKeyTime_2s_Flag=TRUE;
      }
      EnterKeyTimeCount++;
    }
    
/*********初级电量报警30s播报一次********************************/
    if(PrimaryLowPower_Flag==TRUE)
    {
      DelDrvObj.Count.PrimaryLowPowerCount++;
      if(DelDrvObj.Count.PrimaryLowPowerCount>=2*30)
      {
        DelDrvObj.Count.PrimaryLowPowerCount=0;
        VOICE_SetOutput(ATVOICE_FreePlay,"3575606c3575cf914e4f0cfff78b45513575",36);//播报电池电量低请充电
        PrimaryLowPower_Flag=FALSE;
      }
    }
/*********登录超过60s重启*********************************/
    if(Task_Landing_Flag==TRUE)
    {
      LandingTimeCount++;
      if(LandingTimeCount>=2*60)
      {
        LandingTimeCount=0;
        Task_Landing_Flag=FALSE;
        //ApiAtCmd_WritCommand(ATCOMM3_GD83Reset,(void*)0, 0);
      }
    }
    else
    {
      LandingTimeCount=0;
    }
/*********定时5s发一次[AT+CSQ?]*************************************************/
    //if(KaiJi_Flag==TRUE)
    //{
      if(DelDrvObj.Count.CSQTimeCount>=2*2)
      {
          DelDrvObj.Count.CSQTimeCount=0;
          if(NetworkType_2Gor3G_Flag==3)//如果是3G发送HDRCSQ，2G发送CSQ
          {
            if(BootProcess_SIMST_Flag!=2)
            {
              //ApiAtCmd_WritCommand(ATCOMM15_HDRCSQ, (void*)0, 0);
            }  
          }
          else
          {
            if(NetworkType_2Gor3G_Flag==2)
            {
              ApiAtCmd_WritCommand(ATCOMM6_CSQ, (void*)0, 0);
            }
          }
          HDRCSQSignalIcons();
      }
   // }

/******************************************************************************/
    if(GetTaskId()==Task_NormalOperation)
    {
      if(HDRCSQValue<=30)
      {
        SignalPoorCount++;
        if(SignalPoorCount==20*2||SignalPoorCount==40*2)
        {
          //播报网络信号弱
          VOICE_SetOutput(ATVOICE_FreePlay,"517fdc7ee14ff753315f",20);
        }
        if(SignalPoorCount>=60*2)//无信号六十秒，60s重启一次
        {
          ApiAtCmd_WritCommand(ATCOMM3_GD83Reset,(void*)0, 0);
          SignalPoorCount=0;
        }
      }
      else
      {
        SignalPoorCount=0;
      }
    }

/***********************************************************/
    if(ApiAtCmd_GetLoginState()==TRUE)//登录成功
      {
        GpsReconnectionTimeCount++;
#ifdef BEIDOU
        PowerOnCount++;
        if(PowerOnCount>=2*60)//开机定时超过1min，处于北斗模式
        {
          GPIO_WriteLow(GPIOB,GPIO_PIN_3);//NFC
          GPIO_WriteHigh(GPIOB,GPIO_PIN_4);//北斗
          PowerOnCount=2*60;
        }
#endif
        if(GpsReconnectionTimeCount==2*10)
        {
          ApiPocCmd_WritCommand(PocComm_UserListInfo,ucRequestUserListInfo,strlen((char const *)ucRequestUserListInfo));
          switch(ApiGpsServerType)
          {
          case GpsServerType_BuBiao:
            NoUseNum=ApiAtCmd_WritCommand(ATCOMM5_CODECCTL,(u8 *)ucGPSUploadTime_5s,strlen((char const *)ucGPSUploadTime_5s));//设置GPS定位信息5s发送一次
            break;
          case GpsServerType_ZTE:
            break;
          case GpsServerType_BuBiaoAndZTE:
            break;
          }
          GpsReconnectionTimeCount=21;
        }
        if(GpsReconnectionTimeCount>=25)
        {GpsReconnectionTimeCount=21;}
      }
    if(ApiPocCmd_Tone_Flag==TRUE)
    {
      ToneTimeCount++;
      if(ToneTimeCount>1)
      {
        ApiPocCmd_Tone_Flag=FALSE;
        ToneTimeCount=0;
      }
    }
    else
    {
      ToneTimeCount=0;
    }
    
    FILE_Read(0x236,1,DelDrvObj.BacklightTimeBuf);//背光时间【秒】
    FILE_Read(0x247,1,DelDrvObj.KeylockTimeBuf);//键盘锁时间【秒】
    BacklightTimeCount=5*DelDrvObj.BacklightTimeBuf[0];
    if(DelDrvObj.KeylockTimeBuf[0]==0)
      KeylockTimeCount=200*2;//如果=200则永远不锁屏
    else
      KeylockTimeCount=5*DelDrvObj.KeylockTimeBuf[0];
    if(TimeCount_Light>=2*BacklightTimeCount)//10s
    {
      MCU_LCD_BACKLIGTH(OFF);//关闭背光灯
      TimeCount_Light=2*BacklightTimeCount;
    }
    else
    {
      MCU_LCD_BACKLIGTH(ON);//打开背光灯
    }
    if(NumberKeyboardPressDown_flag==TRUE||LockingState_EnterOK_Flag==TRUE)
    {
      TimeCount_Light=0;//背光灯计数器清零
      //NumberKeyboardPressDown_flag=FALSE;
    }
    
    //if(DEL_500ms_Count>1) DEL_500ms_Count=0;
    //switch(DEL_500ms_Count)
    //{
   // case 1://1s
      if(GetTaskId()==Task_NormalOperation)
      {
        if(KeylockTimeCount==200*2)
        {
          TimeCount=0;
          //NumberKeyboardPressDown_flag=TRUE;
        }
        else
        {
        TimeCount++;

        if(TimeCount>=KeylockTimeCount*2) //超时则锁屏
        {
          if(TimeCount==KeylockTimeCount*2)
          {
            LockingState_Flag=TRUE;//超时锁定标志位
            //解决BUG：锁屏后会影响一级二级菜单显示，现处理办法为锁屏就退回默认群组状态,所有菜单标志位初始化
            //api_lcd_pwr_on_hint3("                ");//清屏
            MenuDisplay(Menu_RefreshAllIco);
            if(PersonCallIco_Flag==0)
            {
              api_lcd_pwr_on_hint("                ");//清屏
              
          TASK_PersonalKeyModeSet(FALSE);
          MenuMode_Flag=0;
          api_lcd_pwr_on_hint(0,2,"                ");//清屏
          api_lcd_pwr_on_hint(0,2,GetNowWorkingGroupNameForDisplay());//显示当前群组昵称
          KeyDownUpChoose_GroupOrUser_Flag=0;
          KeyUpDownCount=0;
              
              api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
              api_disp_all_screen_refresh();// 全屏统一刷新
            }
            else
            {
              api_lcd_pwr_on_hint("                ");//清屏
              //api_lcd_pwr_on_hint(HexToChar_MainUserId());//显示当前用户ID
              //api_lcd_pwr_on_hint(HexToChar_PersonalCallingNum());//显示当前用户ID
              api_lcd_pwr_on_hint4(UnicodeForGbk_MainUserName());//显示当前用户昵称
              api_disp_all_screen_refresh();// 全屏统一刷新
            }

            MenuModeCount=1;
            TheMenuLayer_Flag=0;
            MenuMode_Flag=0;
            ApiMenu_SwitchGroup_Flag=0;
            ApiMenu_SwitchCallUser_Flag=0;
            ApiMenu_SwitchOnlineUser_Flag=0;
            ApiMenu_GpsInfo_Flag=0;
            ApiMenu_BacklightTimeSet_Flag=0;
            ApiMenu_KeylockTimeSet_Flag=0;
            ApiMenu_NativeInfo_Flag=0;
            ApiMenu_BeiDouOrWritingFrequency_Flag=0;
          }
          TimeCount=KeylockTimeCount*2+1;
        }
        else
        {
          //MCU_LCD_BACKLIGTH(ON);//打开背光灯
        }
        if(NumberKeyboardPressDown_flag==TRUE&&TimeCount>=KeylockTimeCount*2)//超过10秒后再按按键提示“按OK键再按*键”
        {
          DelDrvObj.Count.TimeCount2++;
          api_lcd_pwr_on_hint("按OK键,再按#键  ");//
          if(DelDrvObj.Count.TimeCount2>=2)//0.5s
          {
            DelDrvObj.Count.TimeCount2=0;
            NumberKeyboardPressDown_flag=FALSE;
            MenuDisplay(Menu_Locking_NoOperation);
          }
        }
        if(LockingState_EnterOK_Flag==TRUE)//锁定界面按下OK键
        {                                                           
          TimeCount3++;//解锁成功也应该至零，
          //MCU_LCD_BACKLIGTH(ON);//打开背光灯
          if(TimeCount3>=4*2)//3s
          {
            TimeCount3=0;
            //MCU_LCD_BACKLIGTH(OFF);//关闭背光灯
            LockingState_EnterOK_Flag=FALSE;
            MenuDisplay(Menu_Locking_NoOperation);
          }
        }
      }
      if(NumberKeyboardPressDown_flag==TRUE&&TimeCount<KeylockTimeCount*2)//当数字数字键盘按下
      {
        TimeCount=0;//当有按键按下，计数器清零
        NumberKeyboardPressDown_flag=FALSE;
      }
      }
      
        
       
   //   break;
   // default:
   //   break;
   // }
  }
  return;
}
#endif

static void DEL_1msProcess(void)
{
  //if (DelDrvObj.Msg.Bit.b1ms == DEL_RUN)
  {
    //DelDrvObj.Msg.Bit.b1ms = DEL_IDLE;
    //ApiPocCmd_83_1msRenew();
    ApiPocCmd_10msRenew();
    ApiCaretCmd_10msRenew();
    ApiAtCmd_10msRenew();
    
  }
  return;
}

static void DEL_10msProcess(void)
{
  if (DelDrvObj.Msg.Bit.b10ms == DEL_RUN) 
  {
    DelDrvObj.Msg.Bit.b10ms = DEL_IDLE;
    ApGpsCmd_10msRenew();

      DelDrvObj.Count.alarm_count++;
      if(poc_receive_sos_statas()==TRUE)
      {
        if(DelDrvObj.Count.alarm_count>=2)
      {
        Test_PWM_LED();//报警音30ms进一次
        DelDrvObj.Count.alarm_count = 0;
      }
      }

  }
  return;
}
