#ifndef __APIDISPLAY_H
#define __APIDISPLAY_H

#include "stm8s.h"
#ifndef EXT_HZK
	#include "DrvGT20L16S1Y.h"
#endif
#include "DrvFont.h"

#define	DISP_IDLE	0x00
#define	DISP_RUN	0x01
#define	BASETYPE	0x0f
#define	NROWTYPE	0x10
#define	PAGE_MAX	0x04
#define	COL_MAX		0x80

#define LCD_DISP_LEN_MAX 	16

typedef enum{
  d_PersonalMode                = 0x00,
  d_ABELL                       = 0x01,
  d_AllGroupName                = 0x02,
  d_NoSimCard                   = 0x03,
  d_NetworkSearching            = 0x04,
  d_PowerLowPleaseCharge        = 0x05,
  d_AllUserName                 = 0x06,
  d_LoggingIn                   = 0x07,
  d_menu                        = 0x08,
  d_menu_groupselect            = 0x09,
  d_menu_gpsinformation         = 0x0A,
  d_menu_backlighttime          = 0x0B,
  d_menu_keypadlocktime         = 0x0C,
  d_menu_softwareversion        = 0x0D,
  d_presswell                   = 0x0F,//按#
  d_longitude_and_latitude0     = 0x10,
  d_backlight                   = 0x11,
  d_keypadlock                  = 0x12,
  d_close                       = 0x13,
  d_individualcall              = 0x14,//临时群组
  d_version                     = 0x15,
  d_press_ok_then_well          = 0x16,
  d_status_offline              = 0x17,
}DISPLAY_TYPE;

typedef struct
{
	u16 iAdr;
	u8  xLen;
	u8  yLen;
}ICO_INF;
typedef struct
{
	DISP_TYPE DispType;
	u8 DispAddX;
	u8 DispAddY;
	u8 DispLenth;
}DISP_CHAR;
typedef struct
{
	u8 DispAddX;
	u8 DispAddY;
	u8 DispHigh;
	u8 DispWidth;
}DISP_ICO;

#define LCD_ICO_BUF_LEN 32
extern void DISPLAY_Show(DISPLAY_TYPE id);
extern void api_disp_icoid_output(u8 IcoIdIndex, bool IcoDefault,bool on_off);
extern void api_disp_all_screen_refresh(void);
extern void api_disp_all_screen_refresh(void);
#if 1//WCDMA 卓智达
void api_lcd_pwr_on_hint(u8 x,u8 y,u8 *CharData);
#else
extern void api_lcd_pwr_on_hint(u8 *CharData);
extern void api_lcd_pwr_on_hint2(u8 *CharData);
extern void api_lcd_pwr_on_hint3(u8 *CharData);
extern void api_lcd_pwr_on_hint5(u8 *CharData);//菜单页右上角序号
extern void api_lcd_pwr_on_hint6(u8 *CharData);
extern void api_lcd_pwr_on_hint7(u8 *CharData);
#endif
extern void api_lcd_pwr_on_hint4(u8 *CharData);//UNICODE显示
extern void api_diap_ico_pos_get(DISP_ICO *pIcoInfo, u16 IcoID);
extern void api_disp_ico_output(DISP_ICO IcoInfo, u8 *IcoData);

//extern u8 r_dat_bat(u32 address,u8 byte_long,u8 *p_arr);
extern unsigned char r_dat_bat(unsigned long address,unsigned long byte_long,unsigned char *p_arr);
extern void SendByte(u32 cmd);
extern u8 ReadByte(void);
#endif