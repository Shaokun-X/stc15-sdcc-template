
#ifndef	__TIMER_H
#define	__TIMER_H

#include	"config.h"

#define	Timer0						0
#define	Timer1						1
#define	Timer2						2
#define	Timer3						3
#define	Timer4						4

#define	TIM_16BitAutoReload			0
#define	TIM_16Bit					1
#define	TIM_8BitAutoReload			2
#define	TIM_16BitAutoReloadNoMask	3

#define	TIM_CLOCK_1T				0
#define	TIM_CLOCK_12T				1
#define	TIM_CLOCK_Ext				2

/*
 * Configure Timer0 or Timer1 directly.
 * MODE: TIM_16BitAutoReload, TIM_16Bit, TIM_8BitAutoReload
 *       Timer0 also supports TIM_16BitAutoReloadNoMask.
 * PRIORITY: PriorityHigh, PriorityLow
 * INTERRUPT: ENABLE, DISABLE
 * CLOCK: TIM_CLOCK_1T, TIM_CLOCK_12T, TIM_CLOCK_Ext
 * CLOCK_OUT: ENABLE, DISABLE
 * VALUE: 0 through 65535
 * RUN: ENABLE, DISABLE
 * All arguments must be side-effect-free.
 */
#define TIMER0_INIT(MODE, PRIORITY, INTERRUPT, CLOCK, CLOCK_OUT, VALUE, RUN) do { \
	TR0 = 0; \
	ET0 = ((INTERRUPT) == ENABLE); \
	PT0 = ((PRIORITY) == PriorityHigh); \
	TMOD = (TMOD & ~0x07) | (MODE) | (((CLOCK) == TIM_CLOCK_Ext) ? 0x04 : 0); \
	AUXR = (AUXR & ~0x80) | (((CLOCK) == TIM_CLOCK_1T) << 7); \
	INT_CLKO = (INT_CLKO & ~0x01) | ((CLOCK_OUT) == ENABLE); \
	TH0 = (u8)((VALUE) >> 8); \
	TL0 = (u8)(VALUE); \
	TR0 = ((RUN) == ENABLE); \
} while (0)

#define TIMER1_INIT(MODE, PRIORITY, INTERRUPT, CLOCK, CLOCK_OUT, VALUE, RUN) do { \
	TR1 = 0; \
	ET1 = ((INTERRUPT) == ENABLE); \
	PT1 = ((PRIORITY) == PriorityHigh); \
	TMOD = (TMOD & ~0x70) | ((MODE) << 4) | (((CLOCK) == TIM_CLOCK_Ext) ? 0x40 : 0); \
	AUXR = (AUXR & ~0x40) | (((CLOCK) == TIM_CLOCK_1T) << 6); \
	INT_CLKO = (INT_CLKO & ~0x02) | (((CLOCK_OUT) == ENABLE) << 1); \
	TH1 = (u8)((VALUE) >> 8); \
	TL1 = (u8)(VALUE); \
	TR1 = ((RUN) == ENABLE); \
} while (0)

/*
 * Configure Timer2. It is always 16-bit auto-reload and has no priority bit.
 * INTERRUPT: ENABLE, DISABLE
 * CLOCK: TIM_CLOCK_1T, TIM_CLOCK_12T, TIM_CLOCK_Ext
 * CLOCK_OUT: ENABLE, DISABLE
 * VALUE: 0 through 65535
 * RUN: ENABLE, DISABLE
 * All arguments must be side-effect-free.
 */
#define TIMER2_INIT(INTERRUPT, CLOCK, CLOCK_OUT, VALUE, RUN) do { \
	AUXR &= ~(1 << 4); \
	IE2 = (IE2 & ~(1 << 2)) | (((INTERRUPT) == ENABLE) << 2); \
	AUXR = (AUXR & ~((1 << 2) | (1 << 3))) | (((CLOCK) == TIM_CLOCK_1T) << 2) | (((CLOCK) == TIM_CLOCK_Ext) << 3); \
	INT_CLKO = (INT_CLKO & ~0x04) | (((CLOCK_OUT) == ENABLE) << 2); \
	TH2 = (u8)((VALUE) >> 8); \
	TL2 = (u8)(VALUE); \
	AUXR |= (((RUN) == ENABLE) << 4); \
} while (0)

#endif
