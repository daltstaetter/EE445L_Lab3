// Speaker.c

extern volatile int flagB0; // global variable that turns on alarm
#include "lm3s1968.h"

void play_Alarm(void)
{
	static int cnt;
	unsigned long delayCnt;
	
	NVIC_DIS0_R = NVIC_DIS0_INT0;  // disables set time interrupt,
	NVIC_DIS0_R = NVIC_DIS0_INT30; // mode, & setAlarmTime interrupts	

	cnt = 0;
	while(flagB0)// button is not pressed
	{
		delayCnt = 1000000;
		cnt++;
		for(delayCnt = 50000; delayCnt != 0; delayCnt--)
		{
			if((delayCnt %25000) == 0)
			{
				GPIO_PORTH_DATA_R ^= 0x01;
			}
		}
		
		GPIO_PORTG_DATA_R &= ~0x04;
		for(delayCnt = 5000000; delayCnt != 0; delayCnt--)
		{
			if((delayCnt %15000) == 0)
			{
				GPIO_PORTH_DATA_R ^= 0x01;
			}
			delayCnt--;
		}
		
		for(delayCnt = 5000000; delayCnt != 0; delayCnt--)
		{
			if((delayCnt %12000) == 0)
			{
				GPIO_PORTH_DATA_R ^= 0x01;
			}
			delayCnt--;
		}
	}
	NVIC_EN0_R = NVIC_EN0_INT0;  // disables set time interrupt,
	NVIC_EN0_R = NVIC_EN0_INT30; // mode, & setAlarmTime interrupts
}



