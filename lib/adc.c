

#include	"adc.h"


u16 ADC_ReadResult(void)
{
	u16 adc;

	if(PCON2 & (1<<5))
	{
		adc = (u16)(ADC_RES & 3);
		adc = (adc << 8) | ADC_RESL;
	}
	else
	{
		adc = (u16)ADC_RES;
		adc = (adc << 2) | (ADC_RESL & 3);
	}

	ADC_CONTR &= ~ADC_FLAG;
	return adc;
}

//========================================================================
// 函数: u16	Get_ADC10bitResult(u8 channel)
// 描述: 查询法读一次ADC结果.
// 参数: channel: 选择要转换的ADC.
// 返回: 10位ADC结果.
// 版本: V1.0, 2012-10-22
//========================================================================
u16	Get_ADC10bitResult(u8 channel)	//channel = 0~7
{
	u8	i;

	if(channel > ADC_CH7)	return	1024;	//错误,返回1024,调用的程序判断	
	ADC_RES = 0;
	ADC_RESL = 0;

	ADC_CONTR = (ADC_CONTR & 0xe0) | ADC_START | channel; 
	NOP(4);			//对ADC_CONTR操作后要4T之后才能访问

	for(i=0; i<250; i++)		//超时
	{
		if(ADC_CONTR & ADC_FLAG)
		{
			return ADC_ReadResult();
		}
	}
	return	1024;	//错误,返回1024,调用的程序判断
}


//========================================================================
// 函数: void ADC_int(void) interrupt ADC_VECTOR
// 描述: ADC中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2012-10-22
//========================================================================
// void ADC_int (void) __interrupt (ADC_VECTOR)
// {
// 	ADC_CONTR &= ~ADC_FLAG;
// }
