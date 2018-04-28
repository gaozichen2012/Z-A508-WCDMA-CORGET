/******************************************************************************
; File Name		: Delay.h
; Author		: ZhangYaLing
; Version		: 1.0
; Description	: delay driver program process
; Date			: 2009.05.04
; Modify		:
******************************************************************************/
#ifndef	DELAY_H
#define DELAY_H

#ifdef	DELLABEL
	#define DELAPI
#else
	#define DELAPI	extern
#endif
DELAPI u8 ApiAtCmd_ZTTSCount;
DELAPI u8 ShowTime_Flag;
DELAPI u8 TaskStartDeleteDelay1Count;
DELAPI u8 TaskStartDeleteDelay3Count;
DELAPI u8 TaskStartDeleteDelay4Count;
DELAPI u8 TaskStartDeleteDelay6Count;
DELAPI bool LockingState_Flag;
DELAPI u16 TimeCount;//超时时间
DELAPI u16 TimeCount3;
DELAPI u8 EnterKeyTimeCount;
DELAPI u8 BacklightTimeCount;//背光灯时间(需要设置进入eeprom)
DELAPI u16 KeylockTimeCount;//键盘锁时间(需要设置进入eeprom)
DELAPI u8 UpDownSwitchingCount;//没按一次换组换呼键至零一次
DELAPI void DEL_PowerOnInitial(void);
DELAPI void DEL_Interrupt(void);
DELAPI void DEL_Renew(void);
DELAPI void DEL_Soft1ms(u16 iLen); 
DELAPI bool DEL_SetTimer(u8 cId,u16 iLen);
DELAPI bool DEL_GetTimer(u8 cId);
//DELAPI bool UpgradeNoATReturn_Flag;
//DELAPI bool UpgradeNoATReturn_Flag2;
#endif
/******************************************************************************
;------------end page
******************************************************************************/
