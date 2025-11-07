#include "DS18B20.h"


//18B20函数实现
void DS18B20_Init(void){
	DS18B20_ConvertT();		//上电先转换一次温度，防止第一次读数据错误
	Delay(1000);			//等待转换完成
}
//转换温度
void DS18B20_ConvertT(void)
{
	OneWire_Init();
	OneWire_SendByte(DS18B20_SKIP_ROM);
	OneWire_SendByte(DS18B20_CONVERT_T);
}
//读取温度
float DS18B20_ReadT(void)
{
	unsigned char TLSB,TMSB;
	int Temp;
	float T;
	OneWire_Init();
	OneWire_SendByte(DS18B20_SKIP_ROM);
	OneWire_SendByte(DS18B20_READ_SCRATCHPAD);
	TLSB=OneWire_ReceiveByte();
	TMSB=OneWire_ReceiveByte();
	Temp=(TMSB<<8)|TLSB;
	T=Temp/16.0;
	return T;
}

void Show_Temperature(){
	DS18B20_ConvertT();	//转换温度
	T=DS18B20_ReadT();	//读取温度
	//Delay(500);
	if(T<0)				//如果温度小于0
	{
		LCD_ShowChar(1,10,'-');	//显示负号
		T=-T;			//将温度变为正数
	}
	else				//如果温度大于等于0
	{
		LCD_ShowChar(1,10,'+');	//显示正号
	}
	LCD_ShowNum(1,11,(unsigned int)T,2);		//显示温度整数部分
	LCD_ShowChar(1,13,'.');		//显示小数点
	LCD_ShowNum(1,14,(unsigned int)(T*100)%100,2);//显示温度小数部分
}

