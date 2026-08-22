
#ifndef	__GPIO_H
#define	__GPIO_H

#include	"config.h"

#define	GPIO_PullUp		0	//上拉准双向口
#define	GPIO_HighZ		1	//浮空输入
#define	GPIO_OUT_OD		2	//开漏输出
#define	GPIO_OUT_PP		3	//推挽输出

#define	GPIO_Pin_0		0x01	//IO引脚 Px.0
#define	GPIO_Pin_1		0x02	//IO引脚 Px.1
#define	GPIO_Pin_2		0x04	//IO引脚 Px.2
#define	GPIO_Pin_3		0x08	//IO引脚 Px.3
#define	GPIO_Pin_4		0x10	//IO引脚 Px.4
#define	GPIO_Pin_5		0x20	//IO引脚 Px.5
#define	GPIO_Pin_6		0x40	//IO引脚 Px.6
#define	GPIO_Pin_7		0x80	//IO引脚 Px.7
#define	GPIO_Pin_All	0xFF	//IO所有引脚
	
#define	GPIO_P0			0		//
#define	GPIO_P1			1
#define	GPIO_P2			2
#define	GPIO_P3			3
#define	GPIO_P4			4
#define	GPIO_P5			5


/*
 * Configure pins without allocating init configuration in RAM.
 * PORT: P0, P1, P2, P3, P4, P5
 * PINS: GPIO_Pin_0 through GPIO_Pin_7, GPIO_Pin_All, or a bitwise OR of pins
 * MODE: GPIO_PullUp, GPIO_HighZ, GPIO_OUT_OD, GPIO_OUT_PP
 * All arguments must be side-effect-free.
 * Example: GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
 */
#define GPIO_INIT(PORT, PINS, MODE) do { \
	PORT##M1 = (PORT##M1 & ~(PINS)) | (((MODE) & 1) ? (PINS) : 0); \
	PORT##M0 = (PORT##M0 & ~(PINS)) | (((MODE) & 2) ? (PINS) : 0); \
} while (0)

#endif
