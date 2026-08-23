
/*---------------------------------------------------------------------*/
/* --- STC MCU International Limited ----------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.GXWMCU.com --------------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了宏晶科技的资料及程序   */
/*---------------------------------------------------------------------*/


/*************	功能说明	**************

本文件为STC15xxx系列的延时程序,用户几乎可以不修改这个程序.

******************************************/

#include	"delay.h"
#include 	"config.h"

/* SDCC emits a MOV and two DJNZ loops; this compensates their per-us cost. */
#define DELAY_US_LOOPS ((MAIN_Fosc - 3000000UL) / 4000000UL)


void delay_us(unsigned char us)
{
	if(!us)	return;

#if MAIN_Fosc < 8000000UL
	/* At low clock rates, the call and outer loop provide the delay. */
	do {
		__asm__("nop");
	} while(--us);
#else
	unsigned char i;

	do
	{
		i = DELAY_US_LOOPS;
		while(--i)	;
	} while(--us);
#endif
}


//========================================================================
// 函数: void  delay_ms(unsigned int ms)
// 描述: 延时函数。
// 参数: ms,要延时的ms数, 支持0~65535ms. 自动适应主时钟.
// 返回: none.
// 版本: VER1.0
// 日期: 2013-4-1
// 备注: 
//========================================================================
void  delay_ms(unsigned int ms)
{
     unsigned int i;

	 if(!ms)	return;

	 do{
	      i = MAIN_Fosc / 13000;
		  while(--i)	;   //14T per loop
	 }while(--ms);
}
