#ifndef __LCD1602_H__
#define __LCD1602_H__


#include <REGX52.H>
#include "Delay.h"


//1602引脚配置
#define LCD_RS P2_6
#define LCD_RW P2_5
#define LCD_EN P2_7
#define LCD_DataPort P0


//1602函数声明
void LCD_Init();
void LCD_ShowChar(unsigned char Line,unsigned char Column,char Char);
void LCD_ShowString(unsigned char Line,unsigned char Column,char *String);
void LCD_ShowNum(unsigned char Line,unsigned char Column,unsigned int Number,unsigned char Length);
int LCD_Pow(int X,int Y);


#endif

