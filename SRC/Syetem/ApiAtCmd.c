#include "AllHead.h"

#if 1//WCDMA
const u8 *ucTxCIMI              ="AT+CIMI";
const u8 *ucTxZPPPOPEN          ="AT+ZPPPOPEN";
const u8 *ucTxCSQ               ="AT+CSQ?";
const u8 *ucTxPlayZtts          = "AT+ZTTS=";
const u8 *ucTxPocOpenConfig     = "at+poc=000000010001";
const u8 *ucTxReset             ="at+cfun=1,1";

const u8 *ucRxZIND      ="ZIND:8";
const u8 *ucRxCIMI      ="CIMI:";
const u8 *ucRxCMEERROR  ="CME ERROR: 10";
const u8 *ucRxZPPPSTATUS="ZPPPSTATUS: OPENED";
const u8 *ucRxCSQ = "CSQ:";
const u8 *ucRxPASTATE1 = "PASTATE:1";
const u8 *ucRxZTTS0 = "ZTTS:0";

u8 KeyDownUpChoose_GroupOrUser_Flag=0;
#endif

typedef struct{
  union{
    struct{
      u16  bStartingUp     :1;
      u16  bCardIn         :1;
      u16  bNoCard         :1;
      u16  bPPPStatusOpen  :1;
      u16  bZTTSStates     :1;
      u16                  :11;
    }Bits;
    u8 Byte;
  }Msg;
  struct{
    u8 Buf[2];
    u8 Len;
    u8 Value;//HDRCSQ的值
  }CSQParam;
  u8 Buf[30];
  u8 Len;
}AtCmdDrv;

static AtCmdDrv AtCmdDrvobj;

void ApiAtCmd_PowerOnInitial(void)
{
  KeyDownUpChoose_GroupOrUser_Flag = 0;
  AtCmdDrvobj.Msg.Byte = 0;
  AtCmdDrvobj.CSQParam.Buf[0] = 0x00;
  AtCmdDrvobj.CSQParam.Buf[1] = 0x00;
  AtCmdDrvobj.CSQParam.Len = 0x00;
  AtCmdDrvobj.CSQParam.Value = 0x00;
}

#if 1//WCDMA 卓智达
bool ApiAtCmd_WritCommand(AtCommType id, u8 *buf, u16 len)
{
  bool r = TRUE;
  DrvMC8332_TxPort_SetValidable(ON);
  switch(id)
  {
  case ATCOMM_CIMI:
    DrvGD83_UART_TxCommand((u8*)ucTxCIMI, strlen((char const*)ucTxCIMI));
    break;
  case ATCOMM_ZPPPOPEN:
    DrvGD83_UART_TxCommand((u8*)ucTxZPPPOPEN, strlen((char const*)ucTxZPPPOPEN));
    break;
  case ATCOMM_CSQ:
    DrvGD83_UART_TxCommand((u8*)ucTxCSQ, strlen((char const*)ucTxCSQ));
    break;
  case ATCOMM_RESET:
    DrvGD83_UART_TxCommand((u8*)ucTxReset, strlen((char const*)ucTxReset));
  case ATCOMM_Test:
    DrvGD83_UART_TxCommand(buf, len);
    break;
  default:
    break;
  }
  r = DrvMc8332_UART_TxTail();
  DrvMC8332_TxPort_SetValidable(OFF);
  return r;
}
#else //CDMA 中兴
bool ApiAtCmd_WritCommand(AtCommType id, u8 *buf, u16 len)
{
  bool r = TRUE;
  u8 Buf1[4];
  DrvMC8332_TxPort_SetValidable(ON);
  switch(id)
  {
  case ATCOMM3_GD83StartupReset://1
    DrvGD83_UART_TxCommand((u8*)ucGD83Reset,strlen((char const *)ucGD83Reset));
    //KaiJi_Flag=FALSE;
    break;
  case ATCOMM3_GD83Reset://1
    //DrvGD83_UART_TxCommand((u8*)ucGD83Reset,strlen((char const *)ucGD83Reset));
    api_lcd_pwr_on_hint("                ");//
    main_init();
    ApiAtCmd_SetLoginState();
    BootProcess_SIMST_Flag=0;
    CSQ_Flag=0;
    TaskStartDeleteDelay1=0;
    TaskStartDeleteDelay2=0;
    TaskStartDeleteDelay3=0;
    TaskStartDeleteDelay4=1;
    TaskStartDeleteDelay5=0;
    TaskStartDeleteDelay6=0;
    TaskStartDeleteDelay1Count=0;
    TaskStartDeleteDelay3Count=0;
    TaskStartDeleteDelay4Count=0;
    TaskStartDeleteDelay6Count=0;
    break;
  case ATCOMM0_OSSYSHWID://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM1_PPPCFG://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM2_ZTTS_Abell://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM4_GD83Mode://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM5_CODECCTL://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM6_CSQ://1
    DrvGD83_UART_TxCommand((u8*)ucCheckRssi, strlen((char const*)ucCheckRssi));
    break;
  case ATCOMM15_HDRCSQ:
    DrvGD83_UART_TxCommand((u8*)ucHDRCSQ, strlen((char const*)ucHDRCSQ));
    break;
  case ATCOMM7_VGR://1
    DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM8_CheckTcp://2
    DrvGD83_UART_TxCommand((u8*)ucCheckTcp, strlen((char const*)ucCheckTcp));
    DrvGD83_UART_TxCommand(buf, len);
    break; 
  case ATCOMM9_SetIp	://1
    DrvGD83_UART_TxCommand((u8*)ucSetIp, strlen((char const *)ucSetIp));
    //DrvGD83_UART_TxCommand(buf, len);
    break;
  case ATCOMM10_SendTcp://1
    if(len <= 1024 && len != 0x00)
    {
      DrvGD83_UART_TxCommand((u8*)ucSendTcp, strlen((char const *)ucSendTcp));
      AtCmd_NetParamCode();//获取IP地址
      DrvGD83_UART_TxCommand(AtCmdDrvobj.NetState.Buf, AtCmdDrvobj.NetState.Len);
      Buf1[0]=',';
      Buf1[1]='0';
      Buf1[2]='x';
      DrvGD83_UART_TxCommand(Buf1, 3);
      DrvGD83_UART_TxCommand(buf, len);
    }
    break; 
  case ATCOMM11_ZpppOpen ://1
    DrvGD83_UART_TxCommand((u8*)ucZpppOpen, strlen((char const *)ucZpppOpen));
    break;
  case ATCOMM12_CheckPPP ://2
    DrvGD83_UART_TxCommand((u8*)ucCheckPPP, strlen((char const *)ucCheckPPP));
    break;
  case ATCOMM13_CheckRssi:
    DrvGD83_UART_TxCommand((u8*)ucCheckRssi, strlen((char const *)ucCheckRssi));
    break;
  case ATCOMM14_CheckCard:
    DrvGD83_UART_TxCommand((u8*)ucCheckCard, strlen((char const *)ucCheckCard));
    break;
  default:
    break;
  }
  r = DrvMc8332_UART_TxTail();
  DrvMC8332_TxPort_SetValidable(OFF);
  return r;
}
#endif


void ApiAtCmd_100msRenew(void)
{
  if(GetTaskId()==Task_NormalOperation)//登录成功进入群组状态
    {
    }
}

void ApiCaretCmd_10msRenew(void)
{
  u8 * pBuf, Len;
  while((Len = DrvMC8332_CaretNotify_Queue_front(&pBuf)) != 0)
  {
  }
}

void ApiAtCmd_10msRenew(void)
{
  u8 * pBuf, ucRet, Len, i;
  while((Len = DrvMC8332_AtNotify_Queue_front(&pBuf)) != 0)
  {
/*****开机ZIND:8*********/
    ucRet = memcmp(pBuf, ucRxZIND, 6);
    if(ucRet == 0x00)
    {
      AtCmdDrvobj.Msg.Bits.bStartingUp=1;
    }
/****已插卡，获取卡号CIMI*************/
    ucRet = memcmp(pBuf, ucRxCIMI, 5);
    if(ucRet == 0x00)
    {
      AtCmdDrvobj.Msg.Bits.bCardIn=1;
      AtCmdDrvobj.Msg.Bits.bNoCard=0;//获取到卡号则清除无卡标志位
    }
/****未插卡CMEERROR**********************/
    ucRet = memcmp(pBuf, ucRxCMEERROR, 13);
    if(ucRet == 0x00)
    {
      AtCmdDrvobj.Msg.Bits.bNoCard=1;
    }
/****数据业务拨号ZPPPSTATUS*************/
    ucRet = memcmp(pBuf, ucRxZPPPSTATUS, 18);
    if(ucRet == 0x00)
    {
      AtCmdDrvobj.Msg.Bits.bPPPStatusOpen=1;
    }
/****信号获取及判断CSQ*********************/
    ucRet = memcmp(pBuf, ucRxCSQ, 4);
    if(ucRet == 0x00)
    {
      if(Len > 5)//去頭
      {
        Len -= 5;
      }
       for(i = 0x00; i < Len; i++)
       {
         AtCmdDrvobj.CSQParam.Buf[i] = pBuf[i + 5];//
       }
      AtCmdDrvobj.CSQParam.Len = i;
      AtCmdDrvobj.CSQParam.Value=CHAR_TO_Digital(AtCmdDrvobj.CSQParam.Buf,2);
    } 
/*******语音播放喇叭控制标志位*******************/
    ucRet = memcmp(pBuf, ucRxPASTATE1, 9);
    if(ucRet == 0x00)
    {
      AtCmdDrvobj.Msg.Bits.bZTTSStates=1;
    }
    ucRet = memcmp(pBuf, ucRxZTTS0, 6);
    if(ucRet == 0x00)
    {
      AtCmdDrvobj.Msg.Bits.bZTTSStates=0;
    }
/************************/
  }
}

bool ApiAtCmd_PlayVoice(AtVoiceType id, u8 *buf, u8 len)
{
  bool r = TRUE;
  DrvMC8332_TxPort_SetValidable(ON);
  DrvGD83_UART_TxCommand((u8*)ucTxPlayZtts, strlen((char const *)ucTxPlayZtts));
  AtCmdDrvobj.Buf[0] = 0x31;	// 1
  AtCmdDrvobj.Buf[1] = 0x2C;	// ,
  AtCmdDrvobj.Buf[2] = 0x22;	// "
  DrvGD83_UART_TxCommand(AtCmdDrvobj.Buf, 3);
  switch(id)
  {
  case ATVOICE_FreePlay :
    DrvGD83_UART_TxCommand(buf, len);
    break;
  default:
    break;
  }
  AtCmdDrvobj.Buf[0] = 0x22;	// "
  DrvGD83_UART_TxCommand(AtCmdDrvobj.Buf, 1);
  r  = DrvMc8332_UART_TxTail();
  DrvMC8332_TxPort_SetValidable(OFF);
  return r;
}

void HDRCSQSignalIcons(void)
{
  if(MenuMode_Flag==0)
  {
    if(ApiAtCmd_CSQValue()>=31)//5格
    {
      api_disp_icoid_output( eICO_IDSPEAKER, TRUE, TRUE);//5格信号
    }
    else if(ApiAtCmd_CSQValue()>=28&&ApiAtCmd_CSQValue()<31)
    {
      api_disp_icoid_output( eICO_IDSCANPA, TRUE, TRUE);//4格信号
    }
    else if(ApiAtCmd_CSQValue()>=25&&ApiAtCmd_CSQValue()<28)
    {
      api_disp_icoid_output( eICO_IDSCAN, TRUE, TRUE);//3格信号
    }
    else if(ApiAtCmd_CSQValue()>=22&&ApiAtCmd_CSQValue()<25)
    {
      api_disp_icoid_output( eICO_IDRXNULL, TRUE, TRUE);//2格信号
    }
    else if(ApiAtCmd_CSQValue()>=19&&ApiAtCmd_CSQValue()<22)
    {
      api_disp_icoid_output( eICO_IDRXFULL, TRUE, TRUE);//1格信号
    }
    else
    {
      api_disp_icoid_output( eICO_IDMESSAGE, TRUE, TRUE);//0格信号
    }
    api_disp_all_screen_refresh();// 全屏统一刷新
  }

}

u16 ApiAtCmd_bStartingUp(void)
{
  return AtCmdDrvobj.Msg.Bits.bStartingUp;
}
u16 ApiAtCmd_bCardIn(void)
{
  return AtCmdDrvobj.Msg.Bits.bCardIn;
}
u16 ApiAtCmd_bNoCard(void)
{
  return AtCmdDrvobj.Msg.Bits.bNoCard;
}
u16 ApiAtCmd_bPPPStatusOpen(void)
{
  return AtCmdDrvobj.Msg.Bits.bPPPStatusOpen;
}
u8 ApiAtCmd_CSQValue(void)
{
  return AtCmdDrvobj.CSQParam.Value;
}
u16 ApiAtCmd_bZTTSStates(void)
{
  return AtCmdDrvobj.Msg.Bits.bZTTSStates;
}