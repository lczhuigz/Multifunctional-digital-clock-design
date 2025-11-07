#include <REGX52.H>
#include "intrins.h"
#include "Delay.h"
#include "LCD1602.h"
#include "DS1302.h"
#include "Key.h"
#include "DS18B20.h"
#include "Timer0.h"



volatile unsigned int StopwatchTime = 0; // 被中断例程更新，声明为 volatile
//闹钟时间设置
char AlarmTime[] = {25,01,01,11,31,59,3}; //年、月、日、 时、分、秒
//标志全局变量
unsigned char KeyNum, MODE, TimeSetSelect, AlarmFlag, BuzzerState, BuzzerCount, StopwatchMode;
volatile unsigned char TimeSetFlashFlag = 0; // 在定时器中断中切换，声明为 volatile
volatile unsigned char StopwatchState = 0;   // 由主循环和中断共同使用，声明为 volatile

float T;//温度
#define Buzzer P1_5


void StopwatchControl();//秒表按键功能 
void CheckAlarm();//闹钟功能函数声明
unsigned char CalculateWeekday(int year, int month, int day);//星期计算函数声明
void TimeSet(char* TimeArray,unsigned char* SelectPtr);//时间设置功能
void TimeShow(void);//时间显示功能
void ShowStopwatch();// 显示秒表计时值



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
	unsigned int timeCopy;
	unsigned int seconds, milliseconds;
	// StopwatchTime 是 16 位，会被中断例程更新；在读取时短暂禁中断以保证原子性
	EA = 0; // 关总中断
	timeCopy = StopwatchTime;
	EA = 1; // 开总中断

	seconds = timeCopy / 1000; // 计算秒
	milliseconds = timeCopy % 1000; // 计算毫秒

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

//时间设置功能
void TimeSet(char* TimeArray,unsigned char* SelectPtr){
    
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
