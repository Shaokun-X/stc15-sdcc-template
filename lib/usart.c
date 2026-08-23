
#include <stdio.h>
#include "usart.h"

#if STDIO_USART == USART1
volatile COMx_Define	COM1;
u8	__xdata TX1_Buffer[COM_TX1_Lenth];	//发送缓冲
u8 	__xdata RX1_Buffer[COM_RX1_Lenth];	//接收缓冲
#elif STDIO_USART == USART2
volatile COMx_Define COM2;
u8	__xdata TX2_Buffer[COM_TX2_Lenth];	//发送缓冲
u8 	__xdata RX2_Buffer[COM_RX2_Lenth];	//接收缓冲
#else
#error "STDIO_USART must be USART1 or USART2"
#endif

/*************** 装载串口发送缓冲 *******************************/
#if STDIO_USART == USART1
void TX1_write2buff(u8 dat)	//写入发送缓冲，指针+1
{
	#if(UART_QUEUE_MODE == 1)
	u8 next = COM1.TX_write + 1;
	if(next >= COM_TX1_Lenth)	next = 0;

	/* Keep one slot empty so equal indices always mean an empty queue. */
	while(next == COM1.TX_read)	;

	TX1_Buffer[COM1.TX_write] = dat;	//装发送缓冲
	COM1.TX_write = next;

	if(COM1.B_TX_busy == 0)		//空闲
	{  
		COM1.B_TX_busy = 1;		//标志忙
		TI = 1;					//触发发送中断
	}
	#else
    //以下是阻塞方式发送方法
    SBUF = dat;
    COM1.B_TX_busy = 1;//标志忙
    while(COM1.B_TX_busy);
    #endif
}
#elif STDIO_USART == USART2
void TX2_write2buff(u8 dat)	//写入发送缓冲，指针+1
{
	#if(UART_QUEUE_MODE == 1)
	u8 next = COM2.TX_write + 1;
	if(next >= COM_TX2_Lenth)	next = 0;

	/* Keep one slot empty so equal indices always mean an empty queue. */
	while(next == COM2.TX_read)	;

	TX2_Buffer[COM2.TX_write] = dat;	//装发送缓冲
	COM2.TX_write = next;

	if(COM2.B_TX_busy == 0)		//空闲
	{  
		COM2.B_TX_busy = 1;		//标志忙
		SET_TI2();				//触发发送中断
	}
    #else
    //以下是阻塞方式发送方法
    S2BUF = dat;
    COM2.B_TX_busy = 1;//标志忙
    while(COM2.B_TX_busy);
    #endif
}
#else
#error "STDIO_USART must be USART1 or USART2"
#endif
/*
void COMx_write2buff(COMx_Define *COMx, u8 dat)	//写入发送缓冲，指针+1
{
	if(COMx->id == 1)	TX1_write2buff(dat);
	if(COMx->id == 2)	TX2_write2buff(dat);
}

void PrintString(COMx_Define *COMx, u8 *puts)
{
    for (; *puts != 0;	puts++)  COMx_write2buff(COMx,*puts); 	//遇到停止符0结束
}
*/

/********************* UART1中断函数************************/
// void UART1_int (void) __interrupt (UART1_VECTOR)
// {
// 	if(RI)
// 	{
// 		RI = 0;
// 		if(COM1.B_RX_OK == 0)
// 		{
// 			if(COM1.RX_Cnt >= COM_RX1_Lenth)	COM1.RX_Cnt = 0;
// 			RX1_Buffer[COM1.RX_Cnt++] = SBUF;
// 			COM1.RX_TimeOut = TimeOutSet1;
// 		}
// 	}

// 	if(TI)
// 	{
// 		TI = 0;

// 		#if(UART_QUEUE_MODE == 1)//判断是否使用队列模式
// 		if(COM1.TX_read != COM1.TX_write)
// 		{
// 		 	SBUF = TX1_Buffer[COM1.TX_read];
// 			if(++COM1.TX_read >= COM_TX1_Lenth)		COM1.TX_read = 0;
// 		}
// 		else	COM1.B_TX_busy = 0;
//         #else
//         COM1.B_TX_busy = 0;//使用阻塞方式发送直接清除繁忙标志
//         #endif
// 	}
// }

/********************* UART2中断函数************************/
// void UART2_int (void) __interrupt (UART2_VECTOR)
// {
// 	if(RI2)
// 	{
// 		CLR_RI2();
// 		if(COM2.B_RX_OK == 0)
// 		{
// 			if(COM2.RX_Cnt >= COM_RX2_Lenth)	COM2.RX_Cnt = 0;
// 			RX2_Buffer[COM2.RX_Cnt++] = S2BUF;
// 			COM2.RX_TimeOut = TimeOutSet2;
// 		}
// 	}

// 	if(TI2)
// 	{
// 		CLR_TI2();

// 		#if(UART_QUEUE_MODE == 1)//判断是否使用队列模式
// 		if(COM2.TX_read != COM2.TX_write)
// 		{
// 		 	S2BUF = TX2_Buffer[COM2.TX_read];
// 			if(++COM2.TX_read >= COM_TX2_Lenth)		COM2.TX_read = 0;
// 		}
// 		else	COM2.B_TX_busy = 0;
//         #else
//         COM2.B_TX_busy = 0;//使用阻塞方式发送直接清除繁忙标志
//         #endif
// 	}

// }

int putchar(int c)
{
#if STDIO_USART == USART1
    TX1_write2buff(c);
#elif STDIO_USART == USART2
	TX2_write2buff(c);
#else
#error "STDIO_USART must be USART1 or USART2"
#endif
    return c;
}
