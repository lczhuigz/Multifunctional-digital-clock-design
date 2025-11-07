#ifndef __DS18B20_H__
#define __DS18B20_H__


#include "Delay.h"
#include "One_Wire.h"
#include "LCD1602.h"

//DS18B20指令
#define DS18B20_SKIP_ROM			0xCC
#define DS18B20_CONVERT_T			0x44
#define DS18B20_READ_SCRATCHPAD 	0xBE


//18B20函数声明
void DS18B20_ConvertT(void);
float DS18B20_ReadT(void);
void DS18B20_Init(void);


//温度显示函数声明
void Show_Temperature();


extern float T;//温度


#endif
