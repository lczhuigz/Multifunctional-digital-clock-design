#include <REGX52.H>
#include "intrins.h"


//1602引脚配置
#define LCD_RS P2_6
#define LCD_RW P2_5
#define LCD_EN P2_7
#define LCD_DataPort P0
//1302引脚配置
#define DS1302_SCLK P3_6
#define DS1302_IO P3_4
#define DS1302_CE P3_5
//18B20引脚配置
#define OneWire_DQ P3_7
//1302寄存器写入地址/指令定义
#define DS1302_SECOND		0x80
#define DS1302_MINUTE		0x82
#define DS1302_HOUR			0x84
#define DS1302_DATE			0x86
#define DS1302_MONTH		0x88
#define DS1302_DAY			0x8A
#define DS1302_YEAR			0x8C
#define DS1302_WP			0x8E
//独立按键函数声明
unsigned char Key();
//延时函数声明
void Delay(unsigned int xms);
//T0定时器初始化函数声明
void Timer0Init(void);
//闹钟功能函数声明
void CheckAlarm();
//1302函数声明
void DS1302_Init(void);
void DS1302_WriteByte(unsigned char Command,Data);
unsigned char DS1302_ReadByte(unsigned char Command);
void DS1302_SetTime(void);
void DS1302_ReadTime(void);
//18B20函数声明
void DS18B20_ConvertT(void);
float DS18B20_ReadT(void);
void DS18B20_Init(void);
//单总线函数声明
unsigned char OneWire_Init(void);
void OneWire_SendBit(unsigned char Bit);
unsigned char OneWire_ReceiveBit(void);
void OneWire_SendByte(unsigned char Byte);
unsigned char OneWire_ReceiveByte(void);
//DS18B20指令
#define DS18B20_SKIP_ROM			0xCC
#define DS18B20_CONVERT_T			0x44
#define DS18B20_READ_SCRATCHPAD 	0xBE
//时间数组，索引0~6分别为年、月、日、时、分、秒、星期，设置为有符号的便于<0的判断
char DS1302_Time[]={25,01,01,11,31,55,3};
//闹钟时间设置
char AlarmTime[] = {25,01,01,11,31,59,3}; //年、月、日、 时、分、秒
//标志全局变量
unsigned char KeyNum,MODE,TimeSetSelect,TimeSetFlashFlag,AlarmFlag,BuzzerState,BuzzerCount,StopwatchMode,StopwatchState;
unsigned int StopwatchTime = 0;
float T;//温度
#define Buzzer P1_5
//星期计算函数声明
unsigned char CalculateWeekday(int year, int month, int day);
//1602函数声明
void LCD_Init();
void LCD_ShowChar(unsigned char Line,unsigned char Column,char Char);
void LCD_ShowString(unsigned char Line,unsigned char Column,char *String);
void LCD_ShowNum(unsigned char Line,unsigned char Column,unsigned int Number,unsigned char Length);
int LCD_Pow(int X,int Y);
//时间函数声明
void TimeSet(char* TimeArray,unsigned char* SelectPtr);//时间设置功能
void TimeShow(void);//时间显示功能
//秒表功能函数声明
void ShowStopwatch();// 显示秒表计时值
void StopwatchControl();//秒表按键功能 
//温度显示函数声明
void Show_Temperature();



void main()
{
	LCD_Init();
	DS1302_Init();
	DS18B20_ConvertT();	
	DS18B20_Init();	
	Timer0Init();
	LCD_ShowString(1,1,"  -  -  ");//静态字符初始化显示
	LCD_ShowString(2,1,"  :  :  ");
	DS1302_Time[6] = CalculateWeekday(DS1302_Time[0], DS1302_Time[1], DS1302_Time[2]);
	DS1302_SetTime();
	
	while(1)
	{
		KeyNum=Key();//读取键码
		if(KeyNum==1)//按键1按下
		{
			if(MODE==0){MODE=1;TimeSetSelect=0;}//功能切换
			else if(MODE==1){MODE=0;DS1302_SetTime();}
			else if(MODE==3){MODE=0;}
		}
		if(KeyNum==5){
			if(MODE==0){
				MODE=2;
				LCD_Init(); // 切换到秒表模式时初始化LCD
				LCD_ShowString(1,1,"Stopwatch");
				StopwatchTime = 0; // 重置秒表时间
				StopwatchState = 0; // 重置秒表状态
			}
			else if(MODE==2){
				MODE=3;
				LCD_Init();
				LCD_ShowString(1,1,"  -  -  ");
				LCD_ShowString(2,1,"  :  :  ");
			}
			else if(MODE == 3)
			{
				MODE=0;
				LCD_Init();
				LCD_ShowString(1,1,"  -  -  ");
				LCD_ShowString(2,1,"  :  :  ");
			}
			
		}
		switch(MODE)//根据不同的功能执行不同的函数
		{
			case 0:TimeShow();CheckAlarm();Show_Temperature();break;
			case 1:TimeSet(DS1302_Time,&TimeSetSelect);break;
			case 2:StopwatchControl();break;
			case 3:TimeSet(AlarmTime,&TimeSetSelect);LCD_ShowString(2, 10, "W:");LCD_ShowString(1, 10, "SETTING");break;
		}
		if (BuzzerState) {
            BuzzerCount++;
            if (BuzzerCount >= 50) { // 蜂鸣器鸣叫 5 秒（100ms * 10）
                BuzzerState = 0;    // 关闭蜂鸣器
                BuzzerCount = 0;
				Buzzer = 0;
            }
            Buzzer = !Buzzer;
			Delay(5);
			Buzzer = !Buzzer;
			Delay(5);
			Buzzer = !Buzzer;
			Delay(5);// 蜂鸣器状态取反
        } else {
            Buzzer = 0; // 关闭蜂鸣器
        }
}
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
//秒表按键功能
void StopwatchControl() { 
    // 按键2：开始/暂停秒表
    if (KeyNum == 6) {
        StopwatchState = !StopwatchState;
    }
    // 按键3：清零秒表
    if (KeyNum == 7) {
        StopwatchTime = 0;
        StopwatchState = 0;
    }
    // 显示秒表计时值
    ShowStopwatch();
}
// 显示秒表计时值
void ShowStopwatch() {
    unsigned int seconds = StopwatchTime / 1000; // 计算秒
    unsigned int milliseconds = StopwatchTime % 1000; // 计算毫秒

    LCD_ShowNum(2, 1, seconds / 60, 2); // 显示分钟
    LCD_ShowChar(2, 3, ':');           // 显示冒号
    LCD_ShowNum(2, 4, seconds % 60, 2); // 显示秒
    LCD_ShowChar(2, 6, '.');           // 显示小数点
    LCD_ShowNum(2, 7, milliseconds / 10, 2); // 显示毫秒（取前两位）
}
void TimeShow(void)//时间显示功能
{	
	DS1302_ReadTime();//读取时间
	LCD_ShowNum(1,1,DS1302_Time[0],2);//显示年
	LCD_ShowNum(1,4,DS1302_Time[1],2);//显示月
	LCD_ShowNum(1,7,DS1302_Time[2],2);//显示日
	LCD_ShowNum(2,1,DS1302_Time[3],2);//显示时
	LCD_ShowNum(2,4,DS1302_Time[4],2);//显示分
	LCD_ShowNum(2,7,DS1302_Time[5],2);//显示秒
	LCD_ShowString(2, 10, "W:");          // 显示星期
 	LCD_ShowNum(2, 12, DS1302_Time[6], 1); // 显示星期值
}

void TimeSet(char* TimeArray,unsigned char* SelectPtr)//时间设置功能
{
	if(KeyNum==2)//按键2按下
	{
		(*SelectPtr)++;//设置选择位加1
		*SelectPtr%=7;//越界清零
	}
	if(KeyNum==3)//按键3按下
	{
		TimeArray[*SelectPtr]++;//时间设置位数值加1
		 
		if(TimeArray[0]>99){TimeArray[0]=0;}//年越界判断
		if(TimeArray[1]>12){TimeArray[1]=1;}//月越界判断

		if( TimeArray[1]==1 || TimeArray[1]==3 || TimeArray[1]==5 || TimeArray[1]==7 || 
			TimeArray[1]==8 || TimeArray[1]==10 || TimeArray[1]==12)//日越界判断
		{
			if(TimeArray[2]>31){TimeArray[2]=1;}//大月
		}
		else if(TimeArray[1]==4 || TimeArray[1]==6 || TimeArray[1]==9 || TimeArray[1]==11)
		{
			if(TimeArray[2]>30){TimeArray[2]=1;}//小月
		}
		else if(TimeArray[1]==2)
		{
			if(TimeArray[0]%4==0)
			{
				if(TimeArray[2]>29){TimeArray[2]=1;}//闰年2月
			}
			else
			{
				if(TimeArray[2]>28){TimeArray[2]=1;}//平年2月
			}
		}
		TimeArray[6] = CalculateWeekday(TimeArray[0], TimeArray[1], TimeArray[2]);//更新星期
		if(TimeArray[3]>23){TimeArray[3]=0;}//时越界判断
		if(TimeArray[4]>59){TimeArray[4]=0;}//分越界判断
		if(TimeArray[5]>59){TimeArray[5]=0;}//秒越界判断
		if(TimeArray[6]>6){TimeArray[6]=0;}//星期越界判断

	}
	if(KeyNum==4)//按键4按下
	{
		TimeArray[*SelectPtr]--;//时间设置位数值减1
		if(TimeArray[0]<0){TimeArray[0]=99;}//年越界判断
		if(TimeArray[1]<1){TimeArray[1]=12;}//月越界判断
		if( TimeArray[1]==1 || TimeArray[1]==3 || TimeArray[1]==5 || TimeArray[1]==7 || 
			TimeArray[1]==8 || TimeArray[1]==10 || TimeArray[1]==12)//日越界判断
		{
			if(TimeArray[2]<1){TimeArray[2]=31;}//大月
			if(TimeArray[2]>31){TimeArray[2]=1;}
		}
		else if(TimeArray[1]==4 || TimeArray[1]==6 || TimeArray[1]==9 || TimeArray[1]==11)
		{
			if(TimeArray[2]<1){TimeArray[2]=30;}//小月
			if(TimeArray[2]>30){TimeArray[2]=1;}
		}
		else if(TimeArray[1]==2)
		{
			if(TimeArray[0]%4==0)
			{
				if(TimeArray[2]<1){TimeArray[2]=29;}//闰年2月
				if(TimeArray[2]>29){TimeArray[2]=1;}
			}
			else
			{
				if(TimeArray[2]<1){TimeArray[2]=28;}//平年2月
				if(TimeArray[2]>28){TimeArray[2]=1;}
			}
		}
		if(TimeArray[3]<0){TimeArray[3]=23;}//时越界判断
		if(TimeArray[4]<0){TimeArray[4]=59;}//分越界判断
		if(TimeArray[5]<0){TimeArray[5]=59;}//秒越界判断
		if(TimeArray[6]<0){TimeArray[6]=6;}//星期越界判断
		TimeArray[6] = CalculateWeekday(TimeArray[0], TimeArray[1], TimeArray[2]);
	}
	//更新显示，根据TimeSetSelect和TimeSetFlashFlag判断可完成闪烁功能
	if(*SelectPtr==0 && TimeSetFlashFlag==1){LCD_ShowString(1,1,"  ");}
	else {LCD_ShowNum(1,1,TimeArray[0],2);}
	if(*SelectPtr==1 && TimeSetFlashFlag==1){LCD_ShowString(1,4,"  ");}
	else {LCD_ShowNum(1,4,TimeArray[1],2);}
	if(*SelectPtr==2 && TimeSetFlashFlag==1){LCD_ShowString(1,7,"  ");}
	else {LCD_ShowNum(1,7,TimeArray[2],2);}
	if(*SelectPtr==3 && TimeSetFlashFlag==1){LCD_ShowString(2,1,"  ");}
	else {LCD_ShowNum(2,1,TimeArray[3],2);}
	if(*SelectPtr==4 && TimeSetFlashFlag==1){LCD_ShowString(2,4,"  ");}
	else {LCD_ShowNum(2,4,TimeArray[4],2);}
	if(*SelectPtr==5 && TimeSetFlashFlag==1){LCD_ShowString(2,7,"  ");}
	else {LCD_ShowNum(2,7,TimeArray[5],2);}
	if(*SelectPtr==6 && TimeSetFlashFlag==1){LCD_ShowString(2,12,"  ");}
	else {LCD_ShowNum(2,12,TimeArray[6],1);}
}

void Timer0_Routine() interrupt 1
{
	static unsigned int T0Count;
	static unsigned char station_toggle;
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	T0Count++;
	if(T0Count>=100)//每100ms进入一次
	{	
		T0Count=0;
		station_toggle++;
		if (station_toggle>=4)
		{
			station_toggle = 0;
			TimeSetFlashFlag=!TimeSetFlashFlag;//闪烁标志位取反
		}
		if (StopwatchState) {
            StopwatchTime += 100; // 增加 100ms
        }
	}
}
//LCD1602写命令
void LCD_WriteCommand(unsigned char Command)
{
	LCD_RS=0;
	LCD_RW=0;
	LCD_DataPort=Command;
	LCD_EN=1;
	Delay(1);
	LCD_EN=0;
	Delay(1);
}
//LCD1602写数据
void LCD_WriteData(unsigned char Data)
{
	LCD_RS=1;
	LCD_RW=0;
	LCD_DataPort=Data;
	LCD_EN=1;
	Delay(1);
	LCD_EN=0;
	Delay(1);
}
//LCD1602设置光标位置
void LCD_SetCursor(unsigned char Line,unsigned char Column)
{
	if(Line==1)
	{
		LCD_WriteCommand(0x80|(Column-1));
	}
	else if(Line==2)
	{
		LCD_WriteCommand(0x80|(Column-1+0x40));
	}
}
//LCD1602初始化函数
void LCD_Init()
{
	LCD_WriteCommand(0x38);//八位数据接口，两行显示，5*7点阵
	LCD_WriteCommand(0x0c);//显示开，光标关，闪烁关
	LCD_WriteCommand(0x06);//数据读写操作后，光标自动加一，画面不动
	LCD_WriteCommand(0x01);//光标复位，清屏
}
//在LCD1602指定位置上显示一个字符
void LCD_ShowChar(unsigned char Line,unsigned char Column,char Char)
{
	LCD_SetCursor(Line,Column);
	LCD_WriteData(Char);
}
//在LCD1602指定位置开始显示所给字符串
void LCD_ShowString(unsigned char Line,unsigned char Column,char *String)
{
	unsigned char i;
	LCD_SetCursor(Line,Column);
	for(i=0;String[i]!='\0';i++)
	{
		LCD_WriteData(String[i]);
	}
}
// 在LCD1602指定位置开始显示所给数字
void LCD_ShowNum(unsigned char Line,unsigned char Column,unsigned int Number,unsigned char Length)
{
	unsigned char i;
	LCD_SetCursor(Line,Column);
	for(i=Length;i>0;i--)
	{
		LCD_WriteData(Number/LCD_Pow(10,i-1)%10+'0');
	}
}
//LCD1602乘方运算
int LCD_Pow(int X,int Y)
{
	unsigned char i;
	int Result=1;
	for(i=0;i<Y;i++)
	{
		Result*=X;
	}
	return Result;
}
//独立按键函数实现
unsigned char Key()
{
	unsigned char KeyNumber=0;
	if(P3_0==0){Delay(20);while(P3_0==0);Delay(20);KeyNumber=1;}
	if(P3_1==0){Delay(20);while(P3_1==0);Delay(20);KeyNumber=2;}
	if(P3_2==0){Delay(20);while(P3_2==0);Delay(20);KeyNumber=3;}
	if(P3_3==0){Delay(20);while(P3_3==0);Delay(20);KeyNumber=4;}
	if(P1_0==0){Delay(20);while(P1_0==0);Delay(20);KeyNumber=5;}
	if(P1_1==0){Delay(20);while(P1_1==0);Delay(20);KeyNumber=6;}
	if(P1_2==0){Delay(20);while(P1_2==0);Delay(20);KeyNumber=7;}
	if(P1_3==0){Delay(20);while(P1_3==0);Delay(20);KeyNumber=8;}
	return KeyNumber;
}
//延时函数实现
void Delay(unsigned int xms)
{
	unsigned char i, j;
	while(xms--)
	{
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
	}
}
//T0定时器初始化函数实现
void Timer0Init(void)
{
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
	PT0=0;
}
//DS1302初始化
void DS1302_Init(void)
{
	DS1302_CE=0;
	DS1302_SCLK=0;
}
//DS1302写一个字节
void DS1302_WriteByte(unsigned char Command,Data)
{
	unsigned char i;
	DS1302_CE=1;
	for(i=0;i<8;i++)
	{
		DS1302_IO=Command&(0x01<<i);
		DS1302_SCLK=1;
		DS1302_SCLK=0;
	}
	for(i=0;i<8;i++)
	{
		DS1302_IO=Data&(0x01<<i);
		DS1302_SCLK=1;
		DS1302_SCLK=0;
	}
	DS1302_CE=0;
}
//DS1302读一个字节
unsigned char DS1302_ReadByte(unsigned char Command)
{
	unsigned char i,Data=0x00;
	Command|=0x01;	//将指令转换为读指令
	DS1302_CE=1;
	for(i=0;i<8;i++)
	{
		DS1302_IO=Command&(0x01<<i);
		DS1302_SCLK=0;
		DS1302_SCLK=1;
	}
	for(i=0;i<8;i++)
	{
		DS1302_SCLK=1;
		DS1302_SCLK=0;
		if(DS1302_IO){Data|=(0x01<<i);}
	}
	DS1302_CE=0;
	DS1302_IO=0;	//读取后将IO设置为0，否则读出的数据会出错
	return Data;
}
//DS1302设置时间  
void DS1302_SetTime(void)
{
	DS1302_WriteByte(DS1302_WP,0x00);
	DS1302_WriteByte(DS1302_YEAR,DS1302_Time[0]/10*16+DS1302_Time[0]%10);//十进制转BCD码后写入
	DS1302_WriteByte(DS1302_MONTH,DS1302_Time[1]/10*16+DS1302_Time[1]%10);
	DS1302_WriteByte(DS1302_DATE,DS1302_Time[2]/10*16+DS1302_Time[2]%10);
	DS1302_WriteByte(DS1302_HOUR,DS1302_Time[3]/10*16+DS1302_Time[3]%10);
	DS1302_WriteByte(DS1302_MINUTE,DS1302_Time[4]/10*16+DS1302_Time[4]%10);
	DS1302_WriteByte(DS1302_SECOND,DS1302_Time[5]/10*16+DS1302_Time[5]%10);
	DS1302_WriteByte(DS1302_DAY,DS1302_Time[6]/10*16+DS1302_Time[6]%10);
	DS1302_WriteByte(DS1302_WP,0x80);
}
//DS1302读取时间
void DS1302_ReadTime(void)
{
	unsigned char Temp;
	Temp=DS1302_ReadByte(DS1302_YEAR);
	DS1302_Time[0]=Temp/16*10+Temp%16;//BCD码转十进制后读取
	Temp=DS1302_ReadByte(DS1302_MONTH);
	DS1302_Time[1]=Temp/16*10+Temp%16;
	Temp=DS1302_ReadByte(DS1302_DATE);
	DS1302_Time[2]=Temp/16*10+Temp%16;
	Temp=DS1302_ReadByte(DS1302_HOUR);
	DS1302_Time[3]=Temp/16*10+Temp%16;
	Temp=DS1302_ReadByte(DS1302_MINUTE);
	DS1302_Time[4]=Temp/16*10+Temp%16;
	Temp=DS1302_ReadByte(DS1302_SECOND);
	DS1302_Time[5]=Temp/16*10+Temp%16;
	Temp=DS1302_ReadByte(DS1302_DAY);
	DS1302_Time[6]=Temp/16*10+Temp%16;
}
//闹钟功能函数实现
void CheckAlarm() {
    if (DS1302_Time[0] == AlarmTime[0] && 
        DS1302_Time[1] == AlarmTime[1] && 
        DS1302_Time[2] == AlarmTime[2] &&
		DS1302_Time[3] == AlarmTime[3] &&
		DS1302_Time[4] == AlarmTime[4] &&
		DS1302_Time[5] == AlarmTime[5] &&
		DS1302_Time[6] == AlarmTime[6]) {
        AlarmFlag = 1; // 触发闹钟
		BuzzerState = 1;
		}else {
        AlarmFlag = 0; // 关闭闹钟
    }
}
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
//单总线函数实现
unsigned char OneWire_Init(void)
{
	unsigned char i;
	unsigned char AckBit;
	OneWire_DQ=1;
	OneWire_DQ=0;
	i = 247;while (--i);		//Delay 500us
	OneWire_DQ=1;
	i = 32;while (--i);			//Delay 70us
	AckBit=OneWire_DQ;
	i = 247;while (--i);		//Delay 500us
	return AckBit;
}
//单总线发送一位
void OneWire_SendBit(unsigned char Bit)
{
	unsigned char i;
	OneWire_DQ=0;
	i = 4;while (--i);			//Delay 10us
	OneWire_DQ=Bit;
	i = 24;while (--i);			//Delay 50us
	OneWire_DQ=1;
}
//单总线接收一位
unsigned char OneWire_ReceiveBit(void)
{
	unsigned char i;
	unsigned char Bit;
	OneWire_DQ=0;
	i = 2;while (--i);			//Delay 5us
	OneWire_DQ=1;
	i = 2;while (--i);			//Delay 5us
	Bit=OneWire_DQ;
	i = 24;while (--i);			//Delay 50us
	return Bit;
}
//单总线发送一个字节
void OneWire_SendByte(unsigned char Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		OneWire_SendBit(Byte&(0x01<<i));
	}
}
//单总线接收一个字节
unsigned char OneWire_ReceiveByte(void)
{
	unsigned char i;
	unsigned char Byte=0x00;
	for(i=0;i<8;i++)
	{
		if(OneWire_ReceiveBit()){Byte|=(0x01<<i);}
	}
	return Byte;
}
//星期计算函数实现
unsigned char CalculateWeekday(int year, int month, int day) {
	unsigned char weekday;
    if (month < 3) {
        month += 12;
        year--;
    }
    weekday = (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
    return (weekday + 1);
}