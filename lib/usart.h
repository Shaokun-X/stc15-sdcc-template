
#ifndef __USART_H
#define __USART_H	 

#include	"config.h"

#define	COM_TX1_Lenth	16
#define	COM_RX1_Lenth	16
#define	COM_TX2_Lenth	16
#define	COM_RX2_Lenth	16

#define	USART1	1
#define	USART2	2

#define	UART_ShiftRight	0		//同步移位输出
#define	UART_8bit_BRTx	(1<<6)	//8位数据,可变波特率
#define	UART_9bit		(2<<6)	//9位数据,固定波特率
#define	UART_9bit_BRTx	(3<<6)	//9位数据,可变波特率

#define	UART1_SW_P30_P31	0
#define	UART1_SW_P36_P37	(1<<6)
#define	UART1_SW_P16_P17	(2<<6)	//必须使用内部时钟
#define	UART2_SW_P10_P11	0
#define	UART2_SW_P46_P47	1


#define	TimeOutSet1		5
#define	TimeOutSet2		5

#define	BRT_Timer1	1
#define	BRT_Timer2	2

typedef struct
{ 
	u8	id;				//串口号

	u8	TX_read;		//发送读指针
	u8	TX_write;		//发送写指针
	u8	B_TX_busy;		//忙标志

	u8 	RX_Cnt;			//接收字节计数
	u8	RX_TimeOut;		//接收超时
	u8	B_RX_OK;		//接收块完成
} COMx_Define; 

#if STDIO_USART == USART1

extern volatile COMx_Define	COM1;
extern	u8	__xdata TX1_Buffer[COM_TX1_Lenth];	//发送缓冲
extern	u8 	__xdata RX1_Buffer[COM_RX1_Lenth];	//接收缓冲
void TX1_write2buff(u8 dat);	//写入发送缓冲，指针+1

#elif STDIO_USART == USART2

extern volatile COMx_Define COM2;
extern	u8	__xdata TX2_Buffer[COM_TX2_Lenth];	//发送缓冲
extern	u8 	__xdata RX2_Buffer[COM_RX2_Lenth];	//接收缓冲
void TX2_write2buff(u8 dat);	//写入发送缓冲，指针+1

#else
#error "STDIO_USART must be USART1 or USART2"
#endif

/*
 * Configure USART1 for 8-bit variable baud using Timer2, without a
 * configuration in RAM.
 * BAUD_RATE: nonzero compile-time baud rate whose Timer2 divisor fits in
 *            16 bits, for example 9600UL or 115200UL
 * RX_ENABLE: ENABLE, DISABLE
 * INTERRUPT: ENABLE, DISABLE
 * PRIORITY: PriorityHigh, PriorityLow
 * PIN_ROUTE: UART1_SW_P30_P31, UART1_SW_P36_P37, UART1_SW_P16_P17
 * LOOPBACK: ENABLE, DISABLE
 * All arguments must be compile-time constants.
 */
#define USART1_TIMER2_INIT(BAUD_RATE, RX_ENABLE, INTERRUPT, PRIORITY, PIN_ROUTE, LOOPBACK) do { \
	COM1.id = 1; \
	COM1.TX_read = 0; \
	COM1.TX_write = 0; \
	COM1.B_TX_busy = 0; \
	COM1.RX_Cnt = 0; \
	COM1.RX_TimeOut = 0; \
	COM1.B_RX_OK = 0; \
	PS = ((PRIORITY) == PriorityHigh); \
	SCON = (SCON & 0x3f) | UART_8bit_BRTx; \
	AUXR &= ~(1 << 4); \
	AUXR |= 0x01; \
	AUXR &= ~(1 << 3); \
	AUXR |= (1 << 2); \
	TH2 = (u8)((65536UL - ((MAIN_Fosc / 4) / (BAUD_RATE))) >> 8); \
	TL2 = (u8)(65536UL - ((MAIN_Fosc / 4) / (BAUD_RATE))); \
	IE2 &= ~(1 << 2); \
	AUXR |= (1 << 4); \
	ES = ((INTERRUPT) == ENABLE); \
	REN = ((RX_ENABLE) == ENABLE); \
	P_SW1 = (P_SW1 & 0x3f) | ((PIN_ROUTE) & 0xc0); \
	PCON2 = (PCON2 & ~(1 << 4)) | (((LOOPBACK) == ENABLE) << 4); \
} while (0)

//void COMx_write2buff(COMx_Define *COMx, u8 dat);	//写入发送缓冲，指针+1
//void PrintString(COMx_Define *COMx, u8 *puts);

#endif
