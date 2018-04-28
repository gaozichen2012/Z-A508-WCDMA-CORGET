#define TASKABLE
#include "AllHead.h"

#if 1//WCDMA 卓智达
typedef struct{
  struct{
    bool AccountConfig;
    bool BootPrompt;
  }status;
}TaskDrv;
static TaskDrv TaskDrvobj;

bool EnterPttMoment_Flag=FALSE;
bool LoosenPttMoment_Flag=FALSE;
u8 EnterPttMomentCount=0;
u8 LoosenPttMomentCount=0;
#else //CDMA 中兴
u8 SSWLCount=0;
u8 StartingUpStep=0;
Key3_OptionType Key3Option;
bool NoUseNum=FALSE;
u8 AlarmCount=4;//2G3G切换计数,默认为3G模式
u8 NetworkType_2Gor3G_Flag=3;
u8 PersonCallIco_Flag=0;//根据显示组呼个呼图标判断状态
u8 Key_PersonalCalling_Flag=0;
bool TASK_Ptt_StartPersonCalling_Flag=FALSE;
bool Task_Landing_Flag=FALSE;


bool EnterKeyTime_2s_Flag=FALSE;
bool KEY_4_Flag=FALSE;
u8 TaskStartDeleteDelay1=0;
u8 TaskStartDeleteDelay2=0;
u8 TaskStartDeleteDelay3=0;
u8 TaskStartDeleteDelay4=1;
u8 TaskStartDeleteDelay5=0;
u8 TaskStartDeleteDelay6=0;

u8 *ucCLVL                       = "AT+CLVL=7";//音量增益7
u8 *ucVGR                       = "AT+VGR=7";//音量增益7
u8 *ucCODECCTL                  = "at^codecctl=3000,2500,0";//T1默认
u8 *ucOSSYSHWID                 = "AT^OSSYSHWID=1";
u8 *ucPrefmode2                  = "AT^prefmode=2";//网络模式2G
u8 *ucPrefmode4                  = "AT^prefmode=4";//网络模式3G
u8 *ucPrefmode8                  = "AT^prefmode=8";//网络模式2/3G自动切换
u8 *ucCSQ                       = "AT+CSQ?";
u8 *ucPPPCFG                    = "AT^PPPCFG=echat,ctnet@mycdma.cn,vnet.mobi";
u8 *ucZTTS_Abell                = "AT+ZTTS=1,\"2d4e745113663d6d20007f5edf57f95bb28b\"";

#endif
void Key3_PlayVoice(void);


void Task_RunStart(void)
{
  UART3_ToMcuMain();
  if(ApiAtCmd_bStartingUp()==1)//开机成功
  {
    if(TaskDrvobj.status.BootPrompt==FALSE)
    {
      VOICE_SetOutput(ATVOICE_FreePlay,"4100620065006C006C00",20);//播报Abell
      TaskDrvobj.status.BootPrompt=TRUE;
    }
    
    if(ApiAtCmd_bCardIn()==1)//检测到卡
    {
      if(ApiAtCmd_CSQValue()==31)
      {
        if(ApiAtCmd_bPPPStatusOpen()==1)
        {
          if(TaskDrvobj.status.AccountConfig==FALSE)
          {
            ApiPocCmd_WritCommand(PocComm_SetParam,0,0);//配置登录账号密码、IP
            TaskDrvobj.status.AccountConfig=TRUE;
            ApiPocCmd_WritCommand(PocComm_OpenPOC,0,0);//打开POC应用
            ApiPocCmd_WritCommand(PocComm_SetURL,0,0);//设置URL
          }
        }
      }
    }
    else
    {
      if(ApiAtCmd_bNoCard()==1)//未检测到卡
      {
      }
    }
  }
#if 0
  api_disp_icoid_output( eICO_IDRXNULL, TRUE, TRUE);//GPRS无信号图标
  VOICE_SetOutput(ATVOICE_FreePlay,"1c64227d517fdc7e",16);//播报搜索网络
  api_lcd_pwr_on_hint("   搜索网络...  ");
  NoUseNum=ApiAtCmd_WritCommand(ATCOMM_CSQ,(u8 *)0,0);//CSQ?
  VOICE_SetOutput(ATVOICE_FreePlay,"c0680d4e30526153",16);//播报检不到卡
  api_lcd_pwr_on_hint("    检不到卡    ");
  HDRCSQSignalIcons(); 
  api_disp_icoid_output( eICO_IDEmergency, TRUE, TRUE);//开机搜到信号，显示3G图标
  api_lcd_pwr_on_hint("   正在登陆..     ");
  ApiPocCmd_WritCommand(PocComm_SetParam,0,0);//配置echat账号、IP
  VOICE_SetOutput(ATVOICE_FreePlay,"636b28577b764696",16);//播报正在登陆
  api_lcd_pwr_on_hint("   正在登陆...    ");
  ApiPocCmd_WritCommand(PocComm_OpenPOC,0,0);
  VOICE_SetOutput(ATVOICE_FreePlay,"517fdc7ee14ff753315f",20);//播报网络信号弱
  VOICE_SetOutput(ATVOICE_FreePlay,"1c64227d517fdc7e",16);//播报搜索网络
  api_lcd_pwr_on_hint("   搜索网络...  ");
#endif
}

#if 1//WCDMA 卓智达
void Task_RunNormalOperation(void)
{
  UART3_ToMcuMain();
//解决写频时，影响其他机器使用（其他机器处于接收状态）
  if(WriteFreq_Flag==TRUE)//解决写频时，群组内其他机器一直有滴滴滴的声音
  {
    ApiPocCmd_WritCommand(PocComm_EndPTT,0,0);
  }
/***********PTT状态检测*************************************************************************************************************************/
  
  if(ReadInput_KEY_PTT==0)//判断按下PTT的瞬间
  {
    EnterPttMomentCount++;
    if(EnterPttMomentCount==1)
      EnterPttMoment_Flag=TRUE;
    else
    {
      EnterPttMoment_Flag=FALSE;
      EnterPttMomentCount=3;
    }
    LoosenPttMoment_Flag=FALSE;
    LoosenPttMomentCount=0;
  }
  else
  {
    EnterPttMomentCount=0;
    EnterPttMoment_Flag=FALSE;
    LoosenPttMomentCount++;
    if(LoosenPttMomentCount==1)
      LoosenPttMoment_Flag=TRUE;
    else
    {
      LoosenPttMoment_Flag=FALSE;
      LoosenPttMomentCount=3;
    }
  }

  switch(ApiPocCmd_KeyPttState())//KeyPttState 0：未按PTT 1:按下ptt瞬间  2：按住PTT状态 3：松开PTT瞬间
  {
  case 0://未按PTT
        if(WriteFreq_Flag==FALSE)//解决写频时，群组内其他机器一直有滴滴滴的声音
        {
          if(EnterPttMoment_Flag==TRUE)
          {
            ApiPocCmd_WritCommand(PocComm_StartPTT,0,0);
          }
        }
    break;
  case 1://1:按下ptt瞬间
    ApiPocCmd_SetKeyPttState(2);
    if(LoosenPttMoment_Flag==TRUE)//如果松开PTT瞬间，发送endPTT指令
    {
      ApiPocCmd_WritCommand(PocComm_EndPTT,0,0);
      Set_RedLed(LED_OFF);
    }
    break;
  case 2://2：按住PTT状态
    Set_RedLed(LED_ON);
    Set_GreenLed(LED_OFF);
    if(TheMenuLayer_Flag!=0)//解决主呼时影响菜单界面信息显示，现在只要按PTT就会退出菜单
    {
        MenuDisplay(Menu_RefreshAllIco);
        api_lcd_pwr_on_hint(0,2,"                ");//清屏
        //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
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
    api_disp_icoid_output( eICO_IDTX, TRUE, TRUE);//发射信号图标
    api_disp_all_screen_refresh();// 全屏统一刷新
      
    if(LoosenPttMoment_Flag==TRUE)//如果松开PTT瞬间，发送endPTT指令
    {
      ApiPocCmd_WritCommand(PocComm_EndPTT,0,0);
      Set_RedLed(LED_OFF);
    }
    break;
  case 3://3：松开PTT瞬间
    ApiPocCmd_SetKeyPttState(0);
    api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
    api_disp_all_screen_refresh();// 全屏统一刷新
    if(EnterPttMoment_Flag==TRUE)
    {
      ApiPocCmd_WritCommand(PocComm_StartPTT,0,0);
    }
    break;
  default:
    break;
  }
  if(ReadInput_KEY_PTT==0)
  {
    switch(KeyDownUpChoose_GroupOrUser_Flag)
    {
    case 0://默认PTT状态
      break;
    case 1://=1，进入某群组
      VOICE_SetOutput(ATVOICE_FreePlay,"f25d09902d4e",12);//播报已选中
      DEL_SetTimer(0,40);
      while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
      //ApiPocCmd_WritCommand(PocComm_EnterGroup,ucPocOpenConfig,strlen((char const *)ucPocOpenConfig));
      KeyDownUpChoose_GroupOrUser_Flag=3;
      EnterKeyTimeCount=0;
      KeyUpDownCount=0;
      break;
    case 2://=2,呼叫某用户
     // if(GettheOnlineMembersDone==TRUE)
      {
        //GettheOnlineMembersDone=FALSE;
        VOICE_SetOutput(ATVOICE_FreePlay,"f25d09902d4e",12);//播报已选中
        DEL_SetTimer(0,60);
        while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
        //ApiPocCmd_WritCommand(PocComm_Invite,ucPocOpenConfig,strlen((char const *)ucPocOpenConfig));
        KeyDownUpChoose_GroupOrUser_Flag=3;
        TASK_Ptt_StartPersonCalling_Flag=TRUE;//判断主动单呼状态（0a）
        EnterKeyTimeCount=0;
      }
      break;
    case 3:
      break;
    case 4:
      break;
    default:
      break;
    }
  }
  else
  {
    //Set_RedLed(LED_OFF);

  }
/*******组呼键状态检测***********************************************************************************************************************************/
#if 0//WCDMA 卓智达
if(ReadInput_KEY_3==0)//组呼键
  {
    if(GetPersonalCallingMode()==1)//如果是单呼模式，则退出单呼
    {
      api_lcd_pwr_on_hint("    退出单呼    ");
      Delay_100ms(5);
      ApiPocCmd_WritCommand(PocComm_Cancel,(u8 *)ucQuitPersonalCalling,strlen((char const *)ucQuitPersonalCalling));
      api_lcd_pwr_on_hint("群组:   组呼模式");
    }
    else
    {
      if(TheMenuLayer_Flag!=0)//解决组呼键影响菜单界面信息显示，现在只要按组呼键就会退出菜单
      {
        MenuDisplay(Menu_RefreshAllIco);
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
      Key3_PlayVoice();
    }
  }
#endif
/*******个呼键状态检测***************************************************************************************************************************************/
  if(ReadInput_KEY_2==0)//个呼键
  {
  }
/*******报警键状态检测********************************************************************************************************************************************/
  if(ReadInput_KEY_4==0)//报警键
  {
  }
/***********判断正常进组；正常退出组;被单呼模式；退出单呼模式；主动开始单呼；单呼；主动退出单呼*************/
#if 0
  if(POC_EnterPersonalCalling_Flag==2)//1刚被呼
  {
  }
  else if(POC_EnterPersonalCalling_Flag==1)//2被呼状态
  {
  }
  else//进组、组内、退组、开始主呼、主呼中、主呼结束
  {
    if(POC_EnterGroupCalling_Flag==2)//1进组
    {
      if(POC_AtEnterPersonalCalling_Flag==2)//主动开始单呼模式
      {
      }
      else if(POC_AtEnterPersonalCalling_Flag==1)//单呼中
      {
      }
      else
      {
      }
      POC_EnterGroupCalling_Flag=1;//进入组内
    }
    else if(POC_EnterGroupCalling_Flag==1)//2组内
    {
      if(POC_AtEnterPersonalCalling_Flag==2)//1刚主呼
      {
      }
      else if(POC_AtEnterPersonalCalling_Flag==1)//2主呼中
      {
      }
      else//3主呼结束
      {
      }
    }
    else//退组
    {
      if(POC_QuitGroupCalling_Flag==2)
      {
        if(POC_QuitPersonalCalling_Flag==2)//被呼模式退组
        {
          POC_QuitPersonalCalling_Flag=0;
          Key_PersonalCalling_Flag=0;
        }
        if(POC_AtQuitPersonalCalling_Flag==2)//主呼模式退组
        {
          POC_AtQuitPersonalCalling_Flag=0;
          Key_PersonalCalling_Flag=0;
        }
        
        if(TASK_Ptt_StartPersonCalling_Flag==TRUE)//解决切换个呼,按PTT确认，播报单呼模式时，中间不应显示一下组呼信息，再显示个呼
        {
        }
        else
        {
          api_disp_icoid_output( eICO_IDPOWERM, TRUE, TRUE);//显示组呼图标
          PersonCallIco_Flag=0;
        }
        POC_QuitGroupCalling_Flag=1;
      }
    }

  }
#endif
/*********判断发射接收图标状态****************************************************************************************************************************/
#if 0
  if(PersonCallIco_Flag==1)
{
  if(POC_ReceivedVoiceStart_Flag==2)//刚接收语音状态
  {
    api_disp_icoid_output( eICO_IDVOX, TRUE, TRUE);//接收信号图标
    api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//消除菜单内被呼，状态栏未刷新的BUG
    api_disp_all_screen_refresh();// 全屏统一刷新
    POC_ReceivedVoiceStart_Flag=1;//接收语音状态
  }
  else//0空闲状态；1接收状态
  {
    if(POC_ReceivedVoiceEnd_Flag==2)//空闲状态
    {
      api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
      api_disp_all_screen_refresh();// 全屏统一刷新
      POC_ReceivedVoiceEnd_Flag=0;//默认无语音状态
    }
    else//空闲状态
    {}
  }
}
else
{
  if(POC_ReceivedVoiceStart_Flag==2)//刚接收语音状态
  {
    api_lcd_pwr_on_hint("                ");//清屏
    //api_lcd_pwr_on_hint4(UnicodeForGbk_SpeakerRightnowName());//显示当前说话人的昵称
    //api_lcd_pwr_on_hint4("1234567890123");//显示当前说话人的昵称
    POC_ReceivedVoiceStart_Flag=1;//接收语音状态
    //修复BUG： A机换组状态，B机呼A机后，A机按PTT却是换组（被呼后A机应该返回默认状态：）
    KeyDownUpChoose_GroupOrUser_Flag=0;
    KeyUpDownCount=0;
    //修复BUG：在菜单界面，B机呼A机，显示屏显示混乱的问题（现为被呼A机退出菜单）
    if(TheMenuLayer_Flag!=0)
    {
      MenuDisplay(Menu_RefreshAllIco);
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
    api_disp_icoid_output( eICO_IDVOX, TRUE, TRUE);//接收信号图标
    api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//消除菜单内被呼，状态栏未刷新的BUG
    api_disp_all_screen_refresh();// 全屏统一刷新

  }
  else if(POC_ReceivedVoiceStart_Flag==1)//0空闲状态；1接收状态//尝试解决闪屏问题
  {
    if(POC_ReceivedVoiceEnd_Flag==2)//空闲状态
    {
    api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
    api_lcd_pwr_on_hint("                ");//清屏
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称--3
    api_disp_all_screen_refresh();// 全屏统一刷新
    POC_ReceivedVoiceStart_Flag=0;//不在收到ff处清零，在收到endFLAG处理后清零
    POC_ReceivedVoiceEnd_Flag=0;//默认无语音状态
    Key_PersonalCalling_Flag=0;//解决被结束单呼后，按上下键任然是切换个呼成员
    }
    else//空闲状态
    {}
  }
  else
  {
  }
}
#endif
/********控制功放喇叭*************************************/
#if 0
if(ApiPocCmd_PlayReceivedVoice_Flag==TRUE)
{
  AUDIO_IOAFPOW(ON);
}
else
{
  if(ApiAtCmd_ZTTS_Flag==TRUE)
  {
    AUDIO_IOAFPOW(ON);
  }
  else
  {
    if(ApiPocCmd_Tone_Flag==TRUE)
    {
      AUDIO_IOAFPOW(ON);
    }
    else
    {
      AUDIO_IOAFPOW(OFF);
    }
  }
}
#endif

/*****如果没有在线成员******************************************/
}
#else //CDMA 中兴
void Task_RunNormalOperation(void)
{
  if(POC_GetAllGroupNameDone_Flag==TRUE)
  {
    Keyboard_Test();
  }
  
  UART3_ToMcuMain();
/***********PTT状态检测*************************************************************************************************************************/
  if(ReadInput_KEY_PTT==0)//判断按下PTT的瞬间
  {
    EnterPttMomentCount++;
    if(EnterPttMomentCount==1)
      EnterPttMoment_Flag=TRUE;
    else
    {
      EnterPttMoment_Flag=FALSE;
      EnterPttMomentCount=3;
    }
    LoosenPttMoment_Flag=FALSE;
    LoosenPttMomentCount=0;
  }
  else
  {
    EnterPttMomentCount=0;
    EnterPttMoment_Flag=FALSE;
    LoosenPttMomentCount++;
    if(LoosenPttMomentCount==1)
      LoosenPttMoment_Flag=TRUE;
    else
    {
      LoosenPttMoment_Flag=FALSE;
      LoosenPttMomentCount=3;
    }
  }
//解决写频时，影响其他机器使用（其他机器处于接收状态）
  if(WriteFreq_Flag==TRUE)//解决写频时，群组内其他机器一直有滴滴滴的声音
  {
    ApiPocCmd_WritCommand(PocComm_EndPTT,0,0);
  }
//解决换组或个呼是，按住PTT进入死循环收不到指令的问题
  
  if(KeyDownUpChoose_GroupOrUser_Flag==3)
  {KeyDownUpChoose_GroupOrUser_Flag=0;}

  switch(KeyPttState)//KeyPttState 0：未按PTT 1:按下ptt瞬间  2：按住PTT状态 3：松开PTT瞬间
  {
  case 0://未按PTT
    POC_ReceivedVoiceEndForXTSF_Flag=0;
#if 1
    if(KeyDownUpChoose_GroupOrUser_Flag==0)
    {
      //if(POC_ReceivedVoice_Flag==FALSE)
      {
        if(WriteFreq_Flag==FALSE)//解决写频时，群组内其他机器一直有滴滴滴的声音
        {
          if(EnterPttMoment_Flag==TRUE)
          {
            ApiPocCmd_WritCommand(PocComm_StartPTT,0,0);
          }
        }
      }
    }
#else
    if(KeyDownUpChoose_GroupOrUser_Flag==0)
    {
      if(POC_ReceivedVoice_Flag==TRUE)//解决对方说话时按PTT接收语音异常的问题
      {
        if(EnterPttMoment_Flag==TRUE)
        {
          VOICE_SetOutput(ATVOICE_FreePlay,"8179d153",8);//禁发
        }
      }
      else
      {
        if(WriteFreq_Flag==FALSE)//解决写频时，群组内其他机器一直有滴滴滴的声音
        {
          if(EnterPttMoment_Flag==TRUE)
          {
            ApiPocCmd_WritCommand(PocComm_StartPTT,ucStartPTT,strlen((char const *)ucStartPTT));
          }
        }
      }
    }
#endif
    break;
  case 1://1:按下ptt瞬间
    KeyPttState=2;
    if(LoosenPttMoment_Flag==TRUE)//如果松开PTT瞬间，发送endPTT指令
    {
      ApiPocCmd_WritCommand(PocComm_EndPTT,0,0);
      Set_RedLed(LED_OFF);
    }
    break;
  case 2://2：按住PTT状态
    if(POC_ReceivedVoice_Flag==TRUE)//如果正在接受语音
    {
    }
    else
    {
      //解决禁发时间到时，播报“系统释放”，机器已停止发射。但显示屏发射符号不消失，红色指示灯不熄灭
      if(POC_ReceivedVoiceEndForXTSF_Flag==2)
      {
        Set_RedLed(LED_OFF);
        KeyDownUpChoose_GroupOrUser_Flag=3;
      }
      else
      {
        Set_RedLed(LED_ON);
        Set_GreenLed(LED_OFF);
      if(TheMenuLayer_Flag!=0)//解决主呼时影响菜单界面信息显示，现在只要按PTT就会退出菜单
      {
        MenuDisplay(Menu_RefreshAllIco);
        api_lcd_pwr_on_hint("                ");//清屏
        //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
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
      api_disp_icoid_output( eICO_IDTX, TRUE, TRUE);//发射信号图标
      api_disp_all_screen_refresh();// 全屏统一刷新
      }
    }
    if(LoosenPttMoment_Flag==TRUE)//如果松开PTT瞬间，发送endPTT指令
    {
      ApiPocCmd_WritCommand(PocComm_EndPTT,0,0);
      
      Set_RedLed(LED_OFF);
    }
    break;
  case 3://3：松开PTT瞬间
    POC_ReceivedVoiceEndForXTSF_Flag=0;
    KeyPttState=0;
    api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
    api_disp_all_screen_refresh();// 全屏统一刷新
#if 1//解决快速按两次PTT异常的问题
    if(EnterPttMoment_Flag==TRUE)
    {
      ApiPocCmd_WritCommand(PocComm_StartPTT,0,0);
    }
#endif
    break;
  default:
    break;
  }
  
  if(ReadInput_KEY_PTT==0)
  {
    switch(KeyDownUpChoose_GroupOrUser_Flag)
    {
    case 0://默认PTT状态
      break;
    case 1://=1，进入某群组
      VOICE_SetOutput(ATVOICE_FreePlay,"f25d09902d4e",12);//播报已选中
      DEL_SetTimer(0,40);
      while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
      //ApiPocCmd_WritCommand(PocComm_EnterGroup,ucPocOpenConfig,strlen((char const *)ucPocOpenConfig));
      KeyDownUpChoose_GroupOrUser_Flag=3;
      EnterKeyTimeCount=0;
      KeyUpDownCount=0;
      break;
    case 2://=2,呼叫某用户
      if(GettheOnlineMembersDone==TRUE)
      {
        //GettheOnlineMembersDone=FALSE;
        VOICE_SetOutput(ATVOICE_FreePlay,"f25d09902d4e",12);//播报已选中
        DEL_SetTimer(0,60);
        while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
        //ApiPocCmd_WritCommand(PocComm_Invite,ucPocOpenConfig,strlen((char const *)ucPocOpenConfig));
        KeyDownUpChoose_GroupOrUser_Flag=3;
        TASK_Ptt_StartPersonCalling_Flag=TRUE;//判断主动单呼状态（0a）
        EnterKeyTimeCount=0;
      }
      break;
    case 3:
      break;
    case 4:
      break;
    default:
      break;
    }
  }
  else
  {
    //Set_RedLed(LED_OFF);

  }
/*******组呼键状态检测***********************************************************************************************************************************/
#if 0//WCDMA 卓智达
if(ReadInput_KEY_3==0)//组呼键
  {
    if(GetPersonalCallingMode()==1)//如果是单呼模式，则退出单呼
    {
      api_lcd_pwr_on_hint("    退出单呼    ");
      Delay_100ms(5);
      ApiPocCmd_WritCommand(PocComm_Cancel,(u8 *)ucQuitPersonalCalling,strlen((char const *)ucQuitPersonalCalling));
      api_lcd_pwr_on_hint("群组:   组呼模式");
    }
    else
    {
      if(TheMenuLayer_Flag!=0)//解决组呼键影响菜单界面信息显示，现在只要按组呼键就会退出菜单
      {
        MenuDisplay(Menu_RefreshAllIco);
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
      Key3_PlayVoice();
    }
  }
#endif
/*******个呼键状态检测***************************************************************************************************************************************/
  if(ReadInput_KEY_2==0)//个呼键
  {
    if(POC_EnterPersonalCalling_Flag==1)//解决被呼状态下，按个呼键无效（应该是被呼状态下，让个呼键失效）
    {
      VOICE_SetOutput(ATVOICE_FreePlay,"ab887c542d4e",12);//个呼中
      DEL_SetTimer(0,50);
      while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    }
    else
    {
      //GettheOnlineMembersDone=FALSE;//解决个呼按键与上下键逻辑混乱问题，个呼键按下直到播报第一个成员后才可以按上下键切换个呼成员
      if(TheMenuLayer_Flag!=0)//解决个呼键影响菜单界面信息显示，现在只要按个呼键就会退出菜单
      {
          MenuDisplay(Menu_RefreshAllIco);
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
      api_lcd_pwr_on_hint("  个呼成员选择  ");
      PersonalCallingNum=0;//解决按单呼键直接选中，单呼用户并不是播报的用户
      Key_PersonalCalling_Flag=1;
      VOICE_SetOutput(ATVOICE_FreePlay,"2a4e7c542000106258540990e962",28);//个呼成员选择
      DEL_SetTimer(0,150);
      while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
      //VOICE_SetOutput(ATVOICE_FreePlay,ApiAtCmd_GetUserName(0),ApiAtCmd_GetUserNameLen(0));//首次获取组内成员播报第一个成员
      api_lcd_pwr_on_hint4("                ");//清屏
      //api_lcd_pwr_on_hint4(UnicodeForGbk_AllUserName(0));//显示当前选中的群组名
      ApiPocCmd_WritCommand(PocComm_UserListInfo,"0E000000000001",strlen((char const *)"0E000000000001"));
      KeyDownUpChoose_GroupOrUser_Flag=2;
      KeyPersonalCallingCount=0;//解决单呼模式，上下键成员非正常顺序，第一个成员在切换时会第二、第三个碰到
    }
  }
/*******报警键状态检测********************************************************************************************************************************************/
  if(ReadInput_KEY_4==0)//报警键
  {
  }
  
/***********判断正常进组；正常退出组;被单呼模式；退出单呼模式；主动开始单呼；单呼；主动退出单呼*************/
  if(POC_EnterPersonalCalling_Flag==2)//1刚被呼
  {
    MenuDisplay(Menu_RefreshAllIco);
    api_lcd_pwr_on_hint("                ");//清屏
    api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//空图标-与选对应
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainUserName());//显示当前用户昵称
    api_disp_icoid_output( eICO_IDPOWERH, TRUE, TRUE);//显示个呼图标
    PersonCallIco_Flag=1;
    api_disp_all_screen_refresh();// 全屏统一刷新
    POC_EnterPersonalCalling_Flag=1;//在单呼模式
    POC_EnterGroupCalling_Flag=1;
  }
  else if(POC_EnterPersonalCalling_Flag==1)//2被呼状态
  {
    //api_lcd_pwr_on_hint("   2被呼状态     ");
  }
  else//进组、组内、退组、开始主呼、主呼中、主呼结束
  {
    if(POC_EnterGroupCalling_Flag==2)//1进组
    {
      if(POC_AtEnterPersonalCalling_Flag==2)//主动开始单呼模式
      {
      }
      else if(POC_AtEnterPersonalCalling_Flag==1)//单呼中
      {
      }
      else
      {
        MenuDisplay(Menu_RefreshAllIco);
        api_lcd_pwr_on_hint("                ");//清屏
        api_disp_icoid_output( eICO_IDPOWERM, TRUE, TRUE);//显示组呼图标
        api_disp_icoid_output( BatteryLevel, TRUE, TRUE);
        api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//空图标-与选对应
//        api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称--2
        PersonCallIco_Flag=0;
        api_disp_all_screen_refresh();// 全屏统一刷新//可能会对POC开机PoC指令识别有影响
      }
      POC_EnterGroupCalling_Flag=1;//进入组内
    }
    else if(POC_EnterGroupCalling_Flag==1)//2组内
    {
      if(POC_AtEnterPersonalCalling_Flag==2)//1刚主呼
      {
        MenuDisplay(Menu_RefreshAllIco);
        api_lcd_pwr_on_hint("                ");//清屏
        api_disp_icoid_output( eICO_IDPOWERH, TRUE, TRUE);//显示个呼图标
        api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//空图标-与选对应
        //api_lcd_pwr_on_hint4(UnicodeForGbk_MainUserName());//显示当前用户昵称
        PersonCallIco_Flag=1;
        api_disp_all_screen_refresh();// 全屏统一刷新//可能会对POC开机PoC指令识别有影响
        POC_AtEnterPersonalCalling_Flag=1;
      }
      else if(POC_AtEnterPersonalCalling_Flag==1)//2主呼中
      {
      }
      else//3主呼结束
      {
      }
    }
    else//退组
    {
      if(POC_QuitGroupCalling_Flag==2)
      {
        if(POC_QuitPersonalCalling_Flag==2)//被呼模式退组
        {
          POC_QuitPersonalCalling_Flag=0;
          Key_PersonalCalling_Flag=0;
        }
        if(POC_AtQuitPersonalCalling_Flag==2)//主呼模式退组
        {
          POC_AtQuitPersonalCalling_Flag=0;
          Key_PersonalCalling_Flag=0;
        }
        
        if(TASK_Ptt_StartPersonCalling_Flag==TRUE)//解决切换个呼,按PTT确认，播报单呼模式时，中间不应显示一下组呼信息，再显示个呼
        {
          //api_lcd_pwr_on_hint("   个呼BUG     ");
        }
        else
        {
          api_disp_icoid_output( eICO_IDPOWERM, TRUE, TRUE);//显示组呼图标
          PersonCallIco_Flag=0;
        }
        POC_QuitGroupCalling_Flag=1;
      }
    }

  }
/*********判断发射接收图标状态****************************************************************************************************************************/
if(PersonCallIco_Flag==1)
{
  if(POC_ReceivedVoiceStart_Flag==2)//刚接收语音状态
  {
    api_disp_icoid_output( eICO_IDVOX, TRUE, TRUE);//接收信号图标
    api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//消除菜单内被呼，状态栏未刷新的BUG
    api_disp_all_screen_refresh();// 全屏统一刷新
    POC_ReceivedVoiceStart_Flag=1;//接收语音状态
  }
  else//0空闲状态；1接收状态
  {
    if(POC_ReceivedVoiceEnd_Flag==2)//空闲状态
    {
      api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
      api_disp_all_screen_refresh();// 全屏统一刷新
      POC_ReceivedVoiceEnd_Flag=0;//默认无语音状态
    }
    else//空闲状态
    {}
  }
}
else
{
  if(POC_ReceivedVoiceStart_Flag==2)//刚接收语音状态
  {
    api_lcd_pwr_on_hint("                ");//清屏
    //api_lcd_pwr_on_hint4(UnicodeForGbk_SpeakerRightnowName());//显示当前说话人的昵称
    //api_lcd_pwr_on_hint4("1234567890123");//显示当前说话人的昵称
    POC_ReceivedVoiceStart_Flag=1;//接收语音状态
    //修复BUG： A机换组状态，B机呼A机后，A机按PTT却是换组（被呼后A机应该返回默认状态：）
    KeyDownUpChoose_GroupOrUser_Flag=0;
    KeyUpDownCount=0;
    //修复BUG：在菜单界面，B机呼A机，显示屏显示混乱的问题（现为被呼A机退出菜单）
    if(TheMenuLayer_Flag!=0)
    {
      MenuDisplay(Menu_RefreshAllIco);
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
    api_disp_icoid_output( eICO_IDVOX, TRUE, TRUE);//接收信号图标
    api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//消除菜单内被呼，状态栏未刷新的BUG
    api_disp_all_screen_refresh();// 全屏统一刷新

  }
  else if(POC_ReceivedVoiceStart_Flag==1)//0空闲状态；1接收状态//尝试解决闪屏问题
  {
    if(POC_ReceivedVoiceEnd_Flag==2)//空闲状态
    {
    api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
    api_lcd_pwr_on_hint("                ");//清屏
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称--3
    api_disp_all_screen_refresh();// 全屏统一刷新
    POC_ReceivedVoiceStart_Flag=0;//不在收到ff处清零，在收到endFLAG处理后清零
    POC_ReceivedVoiceEnd_Flag=0;//默认无语音状态
    Key_PersonalCalling_Flag=0;//解决被结束单呼后，按上下键任然是切换个呼成员
    }
    else//空闲状态
    {}
  }
  else
  {
  }
}

/********控制功放喇叭*************************************/
#if 1
if(ApiPocCmd_PlayReceivedVoice_Flag==TRUE)
{
  AUDIO_IOAFPOW(ON);
}
else
{
  if(ApiAtCmd_ZTTS_Flag==TRUE)
  {
    AUDIO_IOAFPOW(ON);
  }
  else
  {
    if(ApiPocCmd_Tone_Flag==TRUE)
    {
      AUDIO_IOAFPOW(ON);
    }
    else
    {
      AUDIO_IOAFPOW(OFF);
    }
  }
}

#else
/*if(KeyPttState==2)//暂时解决按ptt，功放打开的问题
{
  AUDIO_IOAFPOW(OFF);
}
else*/
{
  if(ApiPocCmd_Tone_Flag==TRUE)//8b0003 解决按PTT无提示音的问题
  {
    AUDIO_IOAFPOW(ON);
  }
  else
  {
    if(UpDownSwitching_Flag==TRUE)//按上下键换组换人状态
    {
      AUDIO_IOAFPOW(ON);
    }
    else
    {
      if(ApiAtCmd_TrumpetVoicePlay_Flag==1)
      {
        AUDIO_IOAFPOW(ON);//在VOICE_SetOutput()加了打开，在识别POC:91加了功放打开;PTT键
      }
      else if(ApiAtCmd_TrumpetVoicePlay_Flag==2)
      {
        AUDIO_IOAFPOW(ON);
      }
      else
      {
        AUDIO_IOAFPOW(OFF);
      }
    }
  }
}
#endif

/*****如果没有在线成员******************************************/
if(PocNoOnlineMember_Flag2==TRUE)
{
  PocNoOnlineMember_Flag2=FALSE;
  MenuMode_Flag=0;
  api_lcd_pwr_on_hint("                ");//清屏
  //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
  //用于PTT键及上下键返回默认状态
  KeyDownUpChoose_GroupOrUser_Flag=0;
  KeyUpDownCount=0;
  Key_PersonalCalling_Flag=0;//进入组呼标志位
  KeyDownUpChoose_GroupOrUser_Flag=0;//解决（个呼键→返回键→OK或PTT）屏幕显示错误的BUG
}
}
#endif

void TASK_WriteFreq(void)
{
  UART3_ToMcuMain();
}
void TASK_RunLoBattery(void)
{
#if 1
  api_lcd_pwr_on_hint(0,2," 电量低  请充电  ");
  VOICE_SetOutput(ATVOICE_FreePlay,"3575606c3575cf914e4f0cfff78b73513a6745513575",44);//电量低请充电
  DEL_SetTimer(0,1000);
  while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
  BEEP_Time(10);
#else
  ApiAtCmd_WritCommand(ATCOMM0_OSSYSHWID,(u8 *)"at+GPSFUNC=0",strlen((char const *)"at+GPSFUNC=0"));//
  ApiAtCmd_WritCommand(ATCOMM0_OSSYSHWID,(u8 *)"at+pwroff",strlen((char const *)"at+pwroff"));//
#endif
}
void Delay_100ms(u8 T)
{
  u16 i;
  u8 j,k,l;
    for(j = 0; j < 83; j++)
    for(l = 0; l < 10; l++)
        for(k = 0; k < 100; k++)
          for(i = 0; i < T; i++)
      {
        nop();
      }
  return;
}

void Key3_PlayVoice(void)
{
  switch(Key3Option)
  {
  case Key3_OptionZero://播报本机账号、当前群组、电池电量
    //当前用户：
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint4(Get_GBK_ActiveUserID());//显示当前用户名
    //VOICE_SetOutput(ATVOICE_FreePlay,Get_Unicode_ActiveUserID(),strlen((char const *)Get_Unicode_ActiveUserID()));//播报当前用户手机号
    DEL_SetTimer(0,285);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    //当前群组
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint(HexToChar_MainGroupId());//显示当前群组ID
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
    //VOICE_SetOutput(ATVOICE_FreePlay,ApiAtCmd_GetMainWorkName(),strlen((char const *)ApiAtCmd_GetMainWorkName()));
    DEL_SetTimer(0,200);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    //电量播报
    KeyBatteryReport();
    DEL_SetTimer(0,20);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    break;
  case Key3_OptionOne://播报本机账号、电池电量
    //当前用户：
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint4(Get_GBK_ActiveUserID());//显示当前用户名
    //VOICE_SetOutput(ATVOICE_FreePlay,Get_Unicode_ActiveUserID(),strlen((char const *)Get_Unicode_ActiveUserID()));//播报当前用户手机号
    DEL_SetTimer(0,285);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    //当前群组
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint(HexToChar_MainGroupId());//显示当前群组ID
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
    //电量播报
    KeyBatteryReport();
    DEL_SetTimer(0,20);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    break;
  case Key3_OptionTwo://播报本机账号
    //当前用户：
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint4(Get_GBK_ActiveUserID());//显示当前用户名
    //VOICE_SetOutput(ATVOICE_FreePlay,Get_Unicode_ActiveUserID(),strlen((char const *)Get_Unicode_ActiveUserID()));//播报当前用户手机号
    DEL_SetTimer(0,200);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    //当前群组
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint(HexToChar_MainGroupId());//显示当前群组ID
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
    break;
  case Key3_OptionThree://播报当前群组
    //当前群组
    api_lcd_pwr_on_hint(0,2,"                ");//显示当前群组昵称
    //api_lcd_pwr_on_hint(HexToChar_MainGroupId());//显示当前群组ID
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
    //VOICE_SetOutput(ATVOICE_FreePlay,ApiAtCmd_GetMainWorkName(),strlen((char const *)ApiAtCmd_GetMainWorkName()));
    DEL_SetTimer(0,20);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    break;
  case Key3_OptionFour://播报电池电量
    //电量播报
    KeyBatteryReport();
    DEL_SetTimer(0,20);
    while(1){if(DEL_GetTimer(0) == TRUE) {break;}}
    break;
  default:
    break;
  }
}

