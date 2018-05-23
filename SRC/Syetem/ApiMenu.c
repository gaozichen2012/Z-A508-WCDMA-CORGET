#include "AllHead.h"

#if 1
u8 ApiMenu_SwitchGroup_Flag=0;
u8 ApiMenu_SwitchCallUser_Flag=0;
u8 ApiMenu_SwitchOnlineUser_Flag=0;
u8 ApiMenu_GpsInfo_Flag=0;
u8 ApiMenu_NativeInfo_Flag=0;
u8 ApiMenu_BacklightTimeSet_Flag=0;
u8 ApiMenu_KeylockTimeSet_Flag=0;
u8 ApiMenu_BeiDouOrWritingFrequency_Flag=0;
#endif


void MenuDisplay(MenuDisplayType id)
{
  u8 Buf1[1];
  u8 Buf2[1];
  switch(id)
  {
  case Menu0:
    break;
  case Menu1:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"1/8");
     api_lcd_pwr_on_hint(0,2,"Group Select    ");
    break;
  case Menu2:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"2/8");
    api_lcd_pwr_on_hint(0,2,"INDV Select     ");
    break;
  case Menu3:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"3/8");
    api_lcd_pwr_on_hint(0,2,"Online INDV List");
    break;
  case Menu4:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"4/8");
    api_lcd_pwr_on_hint(0,2,"GPS Information ");
     break;
     
  case Menu5:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"5/8");
    api_lcd_pwr_on_hint(0,2,"Backlight Time  ");
     if(ApiMenu_BacklightTimeSet_Flag==2)
     {
       ApiMenu_BacklightTimeSet_Flag=0;
       if(BacklightTimeSetCount==7)
       {
         Buf1[0]=0;
         FILE_Write2(0x236,1,Buf1);//背光时间【秒】
       }
       else
       {
         Buf1[0]=BacklightTimeSetCount;
         FILE_Write2(0x236,1,Buf1);//背光时间【秒】
       }
     }
     break;
  case Menu6:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"6/8");
     api_lcd_pwr_on_hint(0,2,"Keypad Lock Time");
     if(ApiMenu_KeylockTimeSet_Flag==2)
     {
       ApiMenu_KeylockTimeSet_Flag=0;
       if(KeylockTimeSetCount==0x10)
       {
         //KeylockTimeCount=200;//如果=200则永远不锁屏
         Buf2[0]=KeylockTimeSetCount-0x10;
         FILE_Write2(0x247,1,Buf2);//键盘锁时间【秒】
       }
       else
       {
         //KeylockTimeCount=(KeylockTimeSetCount-0x10)*30;
         Buf2[0]=KeylockTimeSetCount-0x10;
         FILE_Write2(0x247,1,Buf2);//键盘锁时间【秒】
       }
     }
     break;
  case Menu7:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"7/8");
     api_lcd_pwr_on_hint(0,2,"Software Version");
    break;
  case Menu8:
    api_lcd_pwr_on_hint(0,0,"Menu            ");
    api_lcd_pwr_on_hint(13,0,"8/8");
     api_lcd_pwr_on_hint(0,2,"北斗/写频切换   ");
     break;
/*  case Menu8:
    api_lcd_pwr_on_hint3("菜单            ");
    api_lcd_pwr_on_hint5("8/8");
     api_lcd_pwr_on_hint("语音播报        ");
    break;*/
  case Menu_Locking_NoOperation:
    MenuDisplay(Menu_RefreshAllIco);
    get_screen_display_group_name();//选择显示当前群组昵称（群组或单呼临时群组）
    break;
  case Menu_unLocking:
    MenuDisplay(Menu_RefreshAllIco);
    get_screen_display_group_name();//选择显示当前群组昵称（群组或单呼临时群组）
    break;
  case Menu_RefreshAllIco:
    if(ApiAtCmd_CSQValue()>=31)//5格
    {
      api_disp_icoid_output( eICO_IDSPEAKER, TRUE, TRUE);//5格信号
    }
    else if(ApiAtCmd_CSQValue()>=25&&ApiAtCmd_CSQValue()<31)
    {
      api_disp_icoid_output( eICO_IDSCANPA, TRUE, TRUE);//4格信号
    }
    else if(ApiAtCmd_CSQValue()>=20&&ApiAtCmd_CSQValue()<25)
    {
      api_disp_icoid_output( eICO_IDSCAN, TRUE, TRUE);//3格信号
    }
    else if(ApiAtCmd_CSQValue()>=15&&ApiAtCmd_CSQValue()<20)
    {
      api_disp_icoid_output( eICO_IDRXNULL, TRUE, TRUE);//2格信号
    }
    else if(ApiAtCmd_CSQValue()>=10&&ApiAtCmd_CSQValue()<15)
    {
      api_disp_icoid_output( eICO_IDRXFULL, TRUE, TRUE);//1格信号
    }
    else
    {
      api_disp_icoid_output( eICO_IDMESSAGE, TRUE, TRUE);//0格信号
    }
    
    if(NetworkType_2Gor3G_Flag==3)
      api_disp_icoid_output( eICO_IDEmergency, TRUE, TRUE);//3G图标
    else
      //api_disp_icoid_output( eICO_IDPOWERL, TRUE, TRUE);//图标：2G
      api_disp_icoid_output( eICO_IDEmergency, TRUE, TRUE);//3G图标
    
    if(LockingState_Flag==FALSE)
      api_disp_icoid_output( eICO_IDBANDWIDTHN, TRUE, TRUE);//无锁屏空图标
    else
      api_disp_icoid_output( eICO_IDBANDWIDTHW, TRUE, TRUE);//锁屏图标

    api_disp_icoid_output( BatteryLevel, TRUE, TRUE);//电池电量图标
    api_disp_icoid_output( eICO_IDTALKAR, TRUE, TRUE);//默认无发射无接收信号图标
//if(ShowTime_Flag==FALSE)
    {
      if(VoiceType_FreehandOrHandset_Flag==0)
        api_disp_icoid_output( eICO_IDTemper, TRUE, TRUE);//免提模式
      else
        api_disp_icoid_output( eICO_IDMONITER, TRUE, TRUE);//听筒模式图标
      
      if(get_current_working_status()==m_group_mode)
        api_disp_icoid_output( eICO_IDPOWERM, TRUE, TRUE);//显示组呼图标
      else
        api_disp_icoid_output( eICO_IDPOWERH, TRUE, TRUE);//显示个呼图标
#if 0
      if(KeyDownUpChoose_GroupOrUser_Flag==0)
        api_disp_icoid_output( eICO_IDMESSAGEOff, TRUE, TRUE);//空图标-与选对应
      else
        api_disp_icoid_output( eICO_IDLOCKED, TRUE, TRUE);//选
#endif
    }
      break;
  case Menu_UnlockStep1_Ok:
    api_lcd_pwr_on_hint(0,2,"                ");//清屏
    api_lcd_pwr_on_hint(0,2,"Press #         ");
    break;
  default:
    break;
  }
}

void SubmenuMenuDisplay(SubmenuMenuDisplayType id)
{
  u8 Buf1[22];//={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  u8 Buf2[22];//={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  switch(id)
  {
  case GroupSwitch: 
    MenuDisplay(Menu_RefreshAllIco);
    api_lcd_pwr_on_hint(0,2,"                ");//清屏
    //api_lcd_pwr_on_hint4(UnicodeForGbk_MainWorkName());//显示当前群组昵称
    break;
  case GpsInfoMenu:
     api_lcd_pwr_on_hint(0,2,"                ");//清屏
    api_lcd_pwr_on_hint(0,0,"                ");//清屏
    if(beidou_valid()==FALSE)
    {
      api_lcd_pwr_on_hint(0,0,"Lat :00.000000 ");//清屏
      api_lcd_pwr_on_hint(0,2,"Lng :000.000000  ");//清屏
    }
    else
    {
      //换算并显示经度
      Buf1[0]=0x4c;
      Buf1[1]=0x6e;
      Buf1[2]=0x67;
      Buf1[3]=0x20;
      Buf1[4]=0x3a;
      COML_DecToAsc(poc_latitude_integer(), Buf1+5);
      COML_StringReverse(2,Buf1+5);
      Buf1[7]=0x2e;
      COML_DecToAsc(poc_latitude_float(), Buf1+8);
      COML_StringReverse(6,Buf1+8);
      Buf1[14]='\0';
      api_lcd_pwr_on_hint(0,0,Buf1);
      //换算并显示纬度
      Buf2[0]=0x4c;
      Buf2[1]=0x61;
      Buf2[2]=0x74;
      Buf2[3]=0x20;
      Buf2[4]=0x3a;
      COML_DecToAsc(poc_longitude_integer(), Buf2+5);
      COML_StringReverse(3,Buf2+5);
      Buf2[8]=0x2e;
      COML_DecToAsc(poc_longitude_float(), Buf2+9);
      COML_StringReverse(6,Buf2+9);
      Buf2[15]='\0';
      api_lcd_pwr_on_hint(0,2,Buf2);
    }
    break;
  case NativeInfoMenu:
    MCU_VERSIONForMenu();
    break;
  case BacklightTimeSet:
    Level3MenuDisplay(BacklightTimeSetCount);
    break;
  case KeylockTimeSet:
    Level3MenuDisplay(KeylockTimeSetCount);
    break;
  case BeiDouOrWritingFrequencySwitch:
    api_lcd_pwr_on_hint(0,0,"                ");//清屏
     api_lcd_pwr_on_hint(0,2,"  非此版本功能  ");
    break;
  }
}

void Level3MenuDisplay(Level3MenuDisplayType id)
{
  switch(id)
  {
  case BacklightTimeSet_0s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"7/7");
    api_lcd_pwr_on_hint(0,2,"Close           ");
    break;
  case BacklightTimeSet_10s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"1/7");
    api_lcd_pwr_on_hint(0,2,"5s              ");
    break;
  case BacklightTimeSet_20s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"2/7");
    api_lcd_pwr_on_hint(0,2,"10s             ");
    break;
  case BacklightTimeSet_30s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"3/7");
    api_lcd_pwr_on_hint(0,2,"15s             ");
    break;
  case BacklightTimeSet_40s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"4/7");
    api_lcd_pwr_on_hint(0,2,"20s             ");
    break;
  case BacklightTimeSet_50s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"5/7");
    api_lcd_pwr_on_hint(0,2,"25s             ");
    break;
  case BacklightTimeSet_60s:
    api_lcd_pwr_on_hint(0,0,"Backlight       ");
    api_lcd_pwr_on_hint(13,0,"6/7");
    api_lcd_pwr_on_hint(0,2,"30s             ");
    break;
  case KeylockTimeSet_0s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"7/7");
    api_lcd_pwr_on_hint(0,2,"Close            ");
    break;
  case KeylockTimeSet_30s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"1/7");
    api_lcd_pwr_on_hint(0,2,"5s              ");
    break;
  case KeylockTimeSet_60s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"2/7");
    api_lcd_pwr_on_hint(0,2,"10s             ");
    break;
  case KeylockTimeSet_90s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"3/7");
    api_lcd_pwr_on_hint(0,2,"15s             ");
    break;
  case KeylockTimeSet_120s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"4/7");
    api_lcd_pwr_on_hint(0,2,"20s             ");
    break;
  case KeylockTimeSet_150s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"5/7");
    api_lcd_pwr_on_hint(0,2,"25s             ");
    break;
  case KeylockTimeSet_180s:
    api_lcd_pwr_on_hint(0,0,"Keypad Lock     ");
    api_lcd_pwr_on_hint(13,0,"6/7");
    api_lcd_pwr_on_hint(0,2,"30s             ");
    break;
  default:
    break;
  }

}
