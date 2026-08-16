
#ifndef		__CONFIG_H
#define		__CONFIG_H


/*********************************************************/

// #define MAIN_Fosc		22118400L	//定义主时钟
// #define MAIN_Fosc		12000000L	//定义主时钟
// #define MAIN_Fosc		11059200L	//定义主时钟
// #define MAIN_Fosc		 5529600L	//定义主时钟
// #define MAIN_Fosc		24000000L	//定义主时钟
#define MAIN_Fosc		32000000L	//定义主时钟
#define STDIO_USART		USART1
#define UART_QUEUE_MODE 1

/*********************************************************/

#include	"stc15.h"


/**************************************************************************/

#define Main_Fosc_KHZ	(MAIN_Fosc / 1000)

/***********************************************************/

#endif
