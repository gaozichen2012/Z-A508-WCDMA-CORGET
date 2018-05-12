#define  VOICELABEL
#include "AllHead.h"
#ifdef	VOICE_SEL

const u8 *ucPersonalMode                = "50006500720073006f006e0061006c0020004d006f0064006500";//personal mode
const u8 *ucABELL                       = "4100620065006C006C00";//ABELL
const u8 *ucGroupSelected               = "530065006c0065006300740065006400";//Group Selected
const u8 *ucNoSimCard                   = "6e006f002000730069006D0020006300610072006400";//NO SIM Card
const u8 *ucNetworkSearching            = "6e006500740077006f0072006b00200073006500610072006300680069006e006700";//network searching
const u8 *ucPowerLowPleaseCharge        = "f78b45513575";//Power Low Please Charge
const u8 *ucFivePercentPower            = "3575cf917e7606524b4e3500";
const u8 *ucTwentyPercentPower          = "3575cf917e7606524b4e32004153";
const u8 *ucFortyPercentPower           = "3575cf917e7606524b4e34004153";
const u8 *ucSixtyPercentPower           = "3575cf917e7606524b4e36004153";
const u8 *ucEightyPercentPower          = "3575cf917e7606524b4e38004153";
const u8 *ucOneHundredPercentPower      = "3575cf917e7606527e76";
const u8 *ucGroupMode                   = "07526263A47FC47E";
const u8 *ucHandsetMode                 = "2c54527b216a0f5f";
const u8 *ucHandfreeMode                = "4d51d063216a0f5f";
typedef struct{
	union{
		struct{
			u16 bFreePlay		: 1;
			u16 bCardError		: 1;
			u16 bSingalError	: 1;
			u16 bCheckNet		: 1;
			u16 bLanding		: 1;
			u16 bLandSuccess	: 1;
			u16 bLowAlarm		: 1;
			u16 bLowPowOff		: 1;
			u16 bBattery		: 1;
			u16			: 7;
		}Bits;
		u16 Byte;
	}Msg;
	struct{
                u8 *buf;
		u8 Len;
	}FreePlayData;
	u8 Timer;
}VOICE_DRV;

static VOICE_DRV VoiceDrvObj;

void VOICE_PowerOnInitial(void)
{
	VoiceDrvObj.Msg.Byte = 0x00;
	VoiceDrvObj.Timer = 0x00;
	VoiceDrvObj.FreePlayData.Len = 0x00;
}

void VOICE_Play(VOICEPLAY_TYPE id)
{
  switch(id)
  {
  case PersonalMode:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8 *)ucPersonalMode,strlen((char const*)ucPersonalMode));//personal calling Mode
    break;
  case ABELL:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8 *)ucABELL,strlen((char const*)ucABELL));//播报Abell
    break;
  case AllGroupName:
    VOICE_SetOutput(ATVOICE_FreePlay,GetAllGroupNameForVoice(GroupCallingNum),strlen((char const*)GetAllGroupNameForVoice(GroupCallingNum)));
    break;
  case GroupSelected:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8 *)ucGroupSelected,strlen((char const*)ucGroupSelected));//英文播报：Group Selected
    break;
  case NoSimCard:
    //VOICE_SetOutput(ATVOICE_FreePlay,"c0680d4e30526153",16);//中文播报：检不到卡
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucNoSimCard,strlen((char const*)ucNoSimCard));//中文播报:No SIM Card
    break;
  case NetworkSearching:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucNetworkSearching,strlen((char const*)ucNetworkSearching));//播报搜索网络
    break;
  case PowerLowPleaseCharge:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucPowerLowPleaseCharge,strlen((char const*)ucPowerLowPleaseCharge));//电量低请充电
    break;
  case FivePercentPower:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucFivePercentPower,strlen((char const*)ucFivePercentPower));//百分之五
    break;
  case TwentyPercentPower:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucTwentyPercentPower,strlen((char const*)ucTwentyPercentPower));//百分之20
    break;
  case FortyPercentPower:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucFortyPercentPower,strlen((char const*)ucFortyPercentPower));//百分之40
    break;
  case SixtyPercentPower:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucSixtyPercentPower,strlen((char const*)ucSixtyPercentPower));//百分之60
    break;
  case EightyPercentPower:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucEightyPercentPower,strlen((char const*)ucEightyPercentPower));//百分80
    break;
  case OneHundredPercentPower:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucOneHundredPercentPower,strlen((char const*)ucOneHundredPercentPower));//百分百
    break;
  case GroupMode:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucGroupMode,strlen((char const*)ucGroupMode));//切换群组
    break;
  case HandsetMode:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucHandsetMode,strlen((char const*)ucHandsetMode));//听筒模式
    break;
  case HandfreeMode:
    VOICE_SetOutput(ATVOICE_FreePlay,(u8*)ucHandfreeMode,strlen((char const*)ucHandfreeMode));//免提模式
    break;
  default:
    break;
  }
}


void VOICE_SetOutput(AtVoiceType Id, u8 *buf, u8 Len)
{
#if 0//WCDMA 卓智达
  ApiAtCmd_ZTTS_Flag=TRUE;
#endif
  ApiAtCmd_ZTTSCount=0;
  AUDIO_IOAFPOW(ON);
	//DrvMC8332_TxPort_SetValidable(ON);
	VoiceDrvObj.Msg.Byte |= Id;	
	VoiceDrvObj.FreePlayData.buf = buf;
	VoiceDrvObj.FreePlayData.Len = Len;
	if(Id != ATVOICE_FreePlay)
	{
		VoiceDrvObj.Timer = 0x00;
	}
	else
	{
		VoiceDrvObj.Timer = 0x00;
		VOICE_Renew();
	}
        
	return;
}

void VOICE_Renew()
{
  u8 i;
  const	u8 * pBuf;
  u16 ucMask;
  if (VoiceDrvObj.Msg.Byte != 0x00)
  {
    if(VoiceDrvObj.Timer == 0x00)
    {
      ucMask = 0x01;
      for(i = 0x00; i < 0x10; i++)
      {
        if((ucMask & VoiceDrvObj.Msg.Byte) != OFF)
        {
          VoiceDrvObj.Msg.Byte &= (~ucMask);
          if(VoiceDrvObj.Msg.Byte != 0x00)
          {
            VoiceDrvObj.Timer = 0x08;
          }
          switch(ucMask)
          {
            case ATVOICE_FreePlay:
              pBuf = VoiceDrvObj.FreePlayData.buf;
              i = VoiceDrvObj.FreePlayData.Len;
              break;
            default:
              pBuf = (void *)0;
              i = 0x00;
              break;
          }
          ApiAtCmd_PlayVoice(ucMask, (u8 *)pBuf, i);
          break;
        }
        ucMask <<= 0x01;
      }
    }
  }
}

void VOICE_1sProcess()
{
	if(VoiceDrvObj.Timer != 0x00)
	{
		VoiceDrvObj.Timer--;
	}
}

#endif