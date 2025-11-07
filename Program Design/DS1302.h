#ifndef __DS1302_H__
#define __DS1302_H__


#include <REGX52.H>


//1302引脚配置
#define DS1302_SCLK P3_6
#define DS1302_IO P3_4
#define DS1302_CE P3_5


//1302寄存器写入地址/指令定义
#define DS1302_SECOND		0x80
#define DS1302_MINUTE		0x82
#define DS1302_HOUR			0x84
#define DS1302_DATE			0x86
#define DS1302_MONTH		0x88
#define DS1302_DAY			0x8A
#define DS1302_YEAR			0x8C
#define DS1302_WP			0x8E


//1302函数声明
void DS1302_Init(void);
void DS1302_WriteByte(unsigned char Command, unsigned char Data);
unsigned char DS1302_ReadByte(unsigned char Command);
void DS1302_SetTime(void);
void DS1302_ReadTime(void);


//时间数组，索引0~6分别为年、月、日、时、分、秒、星期，设置为有符号的便于<0的判断
extern char DS1302_Time[];


#endif
