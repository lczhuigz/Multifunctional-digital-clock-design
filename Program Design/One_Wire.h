#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__


#include <REGX52.H>


//18B20引脚配置
#define OneWire_DQ P3_7


//单总线函数声明
unsigned char OneWire_Init(void);
void OneWire_SendBit(unsigned char Bit);
unsigned char OneWire_ReceiveBit(void);
void OneWire_SendByte(unsigned char Byte);
unsigned char OneWire_ReceiveByte(void);

#endif
