
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


/* Busy-wait delays for the STC15 1T core, calibrated for SDCC using
 * MAIN_Fosc. Recheck the generated assembly after changing toolchains or
 * optimization settings.
 */

#include	"delay.h"
#include 	"config.h"

/* SDCC emits a 1T MOV, a 2T inner DJNZ, and a 2T outer DJNZ. The formula
 * rounds the inner-loop count to the nearest whole iteration.
 */
#define DELAY_US_LOOPS ((MAIN_Fosc - 2000000UL) / 2000000UL)

/* SDCC's 16-bit decrement loop takes about 7T per iteration; decrementing
 * the high byte adds 1T once every 256 iterations.
 */
#define DELAY_MS_LOOPS (MAIN_Fosc / 7000UL)


void delay_us(unsigned char us)
{
	if(!us)	return;

#if MAIN_Fosc < 8000000UL
	/* A calibrated inner loop does not fit here; use the shortest loop. */
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


void  delay_ms(unsigned int ms)
{
     unsigned int i;

	 if(!ms)	return;

	 do{
	      i = DELAY_MS_LOOPS;
		  while(--i)	;
	 }while(--ms);
}
