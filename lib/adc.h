
#ifndef	__ADC_H
#define	__ADC_H

#include	"config.h"

#define	ADC_P10		0x01	//IO引脚 Px.0
#define	ADC_P11		0x02	//IO引脚 Px.1
#define	ADC_P12		0x04	//IO引脚 Px.2
#define	ADC_P13		0x08	//IO引脚 Px.3
#define	ADC_P14		0x10	//IO引脚 Px.4
#define	ADC_P15		0x20	//IO引脚 Px.5
#define	ADC_P16		0x40	//IO引脚 Px.6
#define	ADC_P17		0x80	//IO引脚 Px.7
#define	ADC_P1_All	0xFF	//IO所有引脚

#define ADC_90T		(3<<5)
#define ADC_180T	(2<<5)
#define ADC_360T	(1<<5)
#define ADC_540T	0
#define ADC_FLAG	(1<<4)	//软件清0
#define ADC_START	(1<<3)	//自动清0
#define ADC_CH0		0
#define ADC_CH1		1
#define ADC_CH2		2
#define ADC_CH3		3
#define ADC_CH4		4
#define ADC_CH5		5
#define ADC_CH6		6
#define ADC_CH7		7

#define ADC_RES_H2L8	1
#define ADC_RES_H8L2	0


/*
 * Configure the ADC directly.
 * PINS: ADC_P10 through ADC_P17, ADC_P1_All, or a bitwise OR of pins
 * SPEED: ADC_90T, ADC_180T, ADC_360T, ADC_540T
 * POWER: ENABLE, DISABLE
 * ADJUSTMENT: ADC_RES_H2L8, ADC_RES_H8L2
 * INTERRUPT: ENABLE, DISABLE
 * PRIORITY: PriorityHigh, PriorityLow
 * All arguments must be side-effect-free.
 */
#define ADC_INIT(PINS, SPEED, POWER, ADJUSTMENT, INTERRUPT, PRIORITY) do { \
	P1ASF = (PINS); \
	ADC_CONTR = (ADC_CONTR & ~ADC_90T) | (SPEED); \
	PCON2 = (PCON2 & ~(1 << 5)) | (((ADJUSTMENT) == ADC_RES_H2L8) << 5); \
	EADC = ((INTERRUPT) == ENABLE); \
	PADC = ((PRIORITY) == PriorityHigh); \
	ADC_CONTR = (ADC_CONTR & 0x7f) | (((POWER) == ENABLE) << 7); \
} while (0)

/* POWER: ENABLE, DISABLE */
#define ADC_POWER_CONTROL(POWER) do { \
	ADC_CONTR = (ADC_CONTR & 0x7f) | (((POWER) == ENABLE) << 7); \
} while (0)

/* CHANNEL: ADC_CH0 through ADC_CH7 */
#define ADC_START_CONVERSION(CHANNEL) do { \
	ADC_CONTR = (ADC_CONTR & 0xe0) | ADC_START | (CHANNEL); \
} while (0)

u16		ADC_ReadResult(void);
u16		Get_ADC10bitResult(u8 channel);	//channel = 0~7

#endif
