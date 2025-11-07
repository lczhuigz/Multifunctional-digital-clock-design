#include <REGX52.H>
#include "Delay.h"


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
