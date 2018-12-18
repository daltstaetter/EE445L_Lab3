// OLEDTestMain.c
// Runs on LM3S1968
// Test Output.c by sending various characters and strings to
// the OLED display and verifying that the output is correct.
// Daniel Valvano
// July 28, 2011

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
   Section 3.4.5

 Copyright 2011 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdio.h>
#include "Output.h"
#include "rit128x96x4.h"
#include "SysTick.h"
#include "sysctl.h"
#include "hw_types.h"
#include "lm3s1968.h"
#include "OledDisplay.h"
#include "PLL.h"
#include "switches.h"
#include "Speaker.h"
#define FIVE_HERTZ 10000000

extern void DisableInterrupts(void); // Disable interrupts
extern void EnableInterrupts(void);  // Enable interrupts
extern void StartCritical(void);     // low power mode
extern void EndCritical(void);
extern void WaitForInterrupt(void);

volatile unsigned long global_count0 = 0;

volatile int flagA0 = 0;
volatile int flagB0 = 0;
volatile int flagF1 = 0;
volatile int flagF2 = 0;
volatile int clear_flag = 0;

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
//Code Composer Studio Code
void Delay(unsigned long ulCount)
{
	__asm (	"    subs    r0, #1\n"
			"    bne     Delay\n"
			"    bx      lr\n");
}

#else
//Keil uVision Code
__asm void
Delay(unsigned long ulCount)
{
	subs    r0, #1
	bne     Delay
	bx      lr
}

#endif

int main(void)
{ 
	static int doOnce0 = 1;
	static int doOnce1 = 1;
	
	int volatile result;
  Time* myTimePtr;
	Time* myAlarmPtr;
	DisableInterrupts();

{ // system initalizations
  SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ); // 50 MHz
	SysTick_Init(FIVE_HERTZ); // initialize SysTick timer, see SysTick.c
	GPIO_Ports_Init();
  Output_Init();
  Output_Color(15);
	
	myAlarmPtr = Time_Init();
	myTimePtr = Time_Init();
	Output_Clear();
	printf("Time                                %d/%d/14",2+((2+myTimePtr->hour_index)/86400)%31,9+(myTimePtr->hour_index)/86400);
	
	Clock_Init(myTimePtr);
	EnableInterrupts();
}
	while(1)   // main loop
	{	
		GPIO_PORTC_DATA_R ^= 0x10;
		
		DisplayFunction(myTimePtr, global_count0,5);
		DigitalTime(myTimePtr,TIME);

		// start with lowest priority and go in towards highest priority
		// and use break to disable those set interrupts
		while(flagA0) // this sets a new time for the clock
		{		// Highest Priority Task - no other function matters when this is
				// occuring so we can halt execution of everything else till this 
				// is serviced.
				
				// clear the GPIO_RIS_R for interrupts that shouldn't be serviced
				// after executing this setTime Function
				// do setTime function
			SetTime(myTimePtr, &flagA0);	

			DigitalTime(myTimePtr,TIME);
		}
		while(flagF1) // sets AlarmTime
		{	
			if(doOnce0)
			{
				Output_Clear();
				printf("                                     Alarm");
				printf("                                       Set");
				Clock_Init(myAlarmPtr); // create clock face and xy_Positions
				doOnce0 = 0;
				DigitalTime(myAlarmPtr,ALARM);
			}
			
			SetTime(myAlarmPtr, &flagF1);
			DigitalTime(myAlarmPtr,ALARM);
			
			if(flagF1 == 0)
			{	// get it ready to display the actual time again
				Output_Clear();
				printf("Time                                %d/%d/14",2+((2+myTimePtr->hour_index)/86400)%31,9+(myTimePtr->hour_index)/86400);
				Clock_Init(myTimePtr);
				doOnce0 = 1;
				DigitalTime(myAlarmPtr,ALARM);
			}
		}
		
		if(flagB0) // Alarm Sound
		{
			if(doOnce1)
			{ 
				Output_Clear();
				printf("Time                                %d/%d/14",2+((11+myTimePtr->hour_index)/86400)%31,9+(myTimePtr->hour_index)/86400);
				RIT128x96x4StringDraw("AE",0,88,15);

				Clock_Init(myTimePtr);
				doOnce1 = 0;
			}
			if(TimerCompare(myTimePtr,myAlarmPtr))
			{
				// sound alarm
				while(flagB0)
				{
					Clock_Init(myTimePtr);
					play_Alarm();
					doOnce1 = 1;
				}
				Output_Clear();
				printf("Time                                %d/%d/14",2+((8+myTimePtr->hour_index)/86400)%31,9+(myTimePtr->hour_index)/86400);
				Clock_Init(myTimePtr);
			}
		}		
	};
}












