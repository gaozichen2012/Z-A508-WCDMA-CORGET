#include "AllHead.h"
u8 BeidouRxData[75];
u8 BeidouRxDataLen=0;
u8 RxStartFlag[6];
//北斗消息体


typedef struct{
  bool  BDValid;
  u32 BDLongitude_Degree;//度
  u32 BDLongitude_Minute;//分
  u32 BDLongitude_Second;//小数点后的数
  u32 BDLatitude_Degree;//度
  u32 BDLatitude_Minute;//分
  u32 BDLatitude_Second;//小数点后的数
  u16 BDSpeed;//速度
  u16 BDDirection;//方向
}BeidouFunDrv;

static BeidouFunDrv BeidouFunDrvObj;

void ApiBeidou_PowerOnInitial(void)
{
  BeidouFunDrvObj.BDValid = FALSE;
  BeidouFunDrvObj.BDLongitude_Degree = 0;
  BeidouFunDrvObj.BDLongitude_Minute = 0;
  BeidouFunDrvObj.BDLongitude_Second = 0;
  BeidouFunDrvObj.BDLatitude_Degree  = 0;
  BeidouFunDrvObj.BDLatitude_Minute  = 0;
  BeidouFunDrvObj.BDLatitude_Second  = 0;
  BeidouFunDrvObj.BDSpeed = 0;
  BeidouFunDrvObj.BDDirection = 0;
}

void ApiBeidou_Get_location_Information(void)
{
  u8 *pBuf;
  u8 i=0,Ccomma=0,cDot=0,cHead=0;
/***************获取经纬度数据*********************************************************************************************************/

  pBuf=BeidouRxData;
  for(i=0;i<BeidouRxDataLen;i++)
  {
      if(',' == pBuf[i])//2
      {
        switch(Ccomma)
        {
        case 0:
          break;
        case 1:
          if(pBuf[i+1]=='A')
            BeidouFunDrvObj.BDValid=TRUE;
          else
            BeidouFunDrvObj.BDValid=FALSE;
          break;
        case 2:
          break;
        case 3:
          if((i - cHead) >= 4)
          {
           BeidouFunDrvObj.BDLatitude_Second  = CHAR_TO_Digital(&pBuf[cHead], i-cHead);//纬度后四位
          }
          break;
        case 4://N
          break;
        case 5:
          if((i - cHead) >= 4)
          {
            BeidouFunDrvObj.BDLongitude_Second = CHAR_TO_Digital(&pBuf[cHead], i-cHead);//经度后四位
          }
          break;
        case 6:
          break;
        case 7:
          break;
        default:
          break;
        }
        cHead = i+1;
        Ccomma++;
      }
    else
    {
      if('.' == pBuf[i])//2
      {
        switch(cDot)
        {
        case 0:
          break;
        case 1://纬度22
          if((i - cHead) >= 4)
          {
            BeidouFunDrvObj.BDLatitude_Degree  = CHAR_TO_Digital(&pBuf[cHead],2);//纬度前2位
            BeidouFunDrvObj.BDLatitude_Minute  = CHAR_TO_Digital(&pBuf[cHead+2],2);//纬度中2位   
          }
          break;
        case 2:
          if((i - cHead) >= 5)
          {
            BeidouFunDrvObj.BDLongitude_Degree = CHAR_TO_Digital(&pBuf[cHead], 3);//经度前2位
            BeidouFunDrvObj.BDLongitude_Minute = CHAR_TO_Digital(&pBuf[cHead+3],2);//经度中2位
          }
          break;
        case 3:
          BeidouFunDrvObj.BDSpeed = (u16)(1.85*CHAR_TO_Digital(&pBuf[cHead], i-cHead));//速度（1.85*海里/小时）
          //BDSpeed = 20;//速度（1.85*海里/小时）
          break;
        case 4:
          BeidouFunDrvObj.BDDirection=CHAR_TO_Digital(&pBuf[cHead], i-cHead);
          break;
        default:
          break;
        }
        cHead = i+1;
        cDot++;
      }
    }
  }
}

u32  CHAR_TO_Digital(u8 * pBuf, u8 Len)
{
	u8 i;
	u32 buf = 0;
	for(i = 0; i < Len; i++)
	{
		buf *= 10;
		buf += (pBuf[i] - 0x30);
	}
	return buf;
}
void  Digital_TO_CHAR(u8 * pBuf, u32 data, u8 Len)
{
	u8 i;
	for(i = Len-1; i != 0; i--)
	{
		pBuf[i] = data%10 + 0x30;
		data /= 10;
	}
	pBuf[0] = data+0x30;
}

void  CHAR_TO_DIV_CHAR(u8 * pPrimary, u8 * pDestination, u8 Len)
{
	u8 i, j , buf;
	for(i =0; i != Len; i++)
	{
		buf = (pPrimary[i] >> 0x04) & 0x0F;
		j = i * 2;
		if(buf < 0x0A)
		{
			pDestination[j] = buf + 0x30;
		}
		else
		{
			pDestination[j] = buf + 0x57;
		}
		buf = pPrimary[i] & 0x0F;
		if(buf < 0x0A)
		{
			pDestination [j + 1] = buf + 0x30;
		}
		else
		{
			pDestination[j + 1] = buf + 0x57;
		}
	}
}

bool beidou_valid(void)
{
  return BeidouFunDrvObj.BDValid;
}

u32 beidou_longitude_degree(void)//度
{
  return BeidouFunDrvObj.BDLongitude_Degree;
}
u32 beidou_longitude_minute(void)//分
{
  return BeidouFunDrvObj.BDLongitude_Minute;
}
u32 beidou_longitude_second(void)//小数点后的数
{
  return BeidouFunDrvObj.BDLongitude_Second;
}
u32 beidou_latitude_degree(void)//度
{
  return BeidouFunDrvObj.BDLatitude_Degree;
}
u32 beidou_latitude_minute(void)//分
{
  return BeidouFunDrvObj.BDLatitude_Minute;
}
u32 beidou_latitude_second(void)//小数点后的数
{
  return BeidouFunDrvObj.BDLatitude_Second;
}