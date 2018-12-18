// OledDisplay.c

#include "OLED_1968/rit128x96x4.h"
#include <stdlib.h>
#include "OledDisplay.h"
#include "lm3s1968.h"
// only adding <stdio.h> for debugging, get rid of it after testing
#include <stdio.h>
#include <string.h>

static const int SIN[POSITIONS] = {  0, 105, 208, 309, 407, 500, 588, 669, 743, 809, 866, 914, 951, 978, 995,
		                 1000, 995, 978, 951, 914, 866, 809, 743, 669, 588, 500, 407, 309, 208, 105,
	                      0,-105,-208,-309,-407,-500,-588,-669,-743,-809,-866,-914,-951,-978,-995,
						        -1000,-995,-978,-951,-914,-866,-809,-743,-669,-588,-500,-407,-309,-208,-105};
	
static const int COS[POSITIONS] = {1000, 995, 978, 951, 914, 866, 809, 743, 669, 588, 500, 407, 309, 208, 105,
								         0,-105,-208,-309,-407,-500,-588,-669,-743,-809,-866,-914,-951,-978,-995,
		                 -1000,-995,-978,-951,-914,-866,-809,-743,-669,-588,-500,-407,-309,-208,-105,
	                       0, 105, 208, 309, 407, 500, 588, 669, 743, 809, 866, 914, 951, 978, 995};

void CreateClockFace(Time* timePtr)
{	// draws the analog clock face
	RIT128x96x4StringDraw("1",51,0,8);RIT128x96x4StringDraw("2",57,0,8);RIT128x96x4StringDraw("!",17,65,8);
	RIT128x96x4StringDraw("1",79,9,8);
	RIT128x96x4StringDraw("2",93,25,8);
	RIT128x96x4StringDraw("3",99,45,8);
	RIT128x96x4StringDraw("4",93,65,8);
	RIT128x96x4StringDraw("5",79,81,8);
	RIT128x96x4StringDraw("6",55,88,8);
	RIT128x96x4StringDraw("7",31,81,8);
	RIT128x96x4StringDraw("8",17,65,8);
	RIT128x96x4StringDraw("9",11,45,8);
	RIT128x96x4StringDraw("1",11,22,8);RIT128x96x4StringDraw("0",17,22,8);
	RIT128x96x4StringDraw("1",28,7,8);RIT128x96x4StringDraw("1",34,7,8);	
}												 
							

void PrintOneHand(Time* timePtr, int x_pos[],int y_pos[], int* index, int back_forw)
{
	
	// mod the input for x_Hours[j%60] so it
  // doesn't overruns its buffer size
	
	// remove the previous minute/hour hand before and 
	// after current position for possible setTime 
  // function and then print the updated hand
	
	RIT128x96x4_LineOff(timePtr->xPivot, timePtr->yPivot, x_pos[((*index)+2)%POSITIONS], y_pos[((*index)+2)%POSITIONS], 6);
	RIT128x96x4_LineOff(timePtr->xPivot, timePtr->yPivot, x_pos[((*index)+1)%POSITIONS], y_pos[((*index)+1)%POSITIONS], 6);
	RIT128x96x4_LineOff(timePtr->xPivot, timePtr->yPivot, x_pos[(*index)%POSITIONS], y_pos[(*index)%POSITIONS], 6);
	RIT128x96x4_LineOff(timePtr->xPivot, timePtr->yPivot, x_pos[((*index)-1)%POSITIONS], y_pos[((*index)-1)%POSITIONS], 6);
	RIT128x96x4_LineOff(timePtr->xPivot, timePtr->yPivot, x_pos[((*index)-2)%POSITIONS], y_pos[((*index)-2)%POSITIONS], 6);
	
	//timePtr->minute_index = (timePtr->minute_index + POSITIONS + back_forw);
	(*index) = (*index) + POSITIONS + back_forw;
	timePtr->direction = back_forw;
}
void DigitalTime(Time* timePtr,int setting)
{
	static char hourCharTime[4];
	static char minCharTime[4];
	static char minCharAlarm[4];
	static char hourCharAlarm[4];
	
	// this isn't necessary from what I've seen but it may be less buggy
	// by splitting it into separate cases since im using static variables
	if(setting == TIME)
	{
		getValue(hourCharTime,minCharTime,timePtr,setting);
		RIT128x96x4StringDraw(hourCharTime,98,88,15);
		RIT128x96x4StringDraw(hourCharTime+1,104,88,15);
		RIT128x96x4StringDraw(hourCharTime+2,110,88,15);
		RIT128x96x4StringDraw(minCharTime,116,88,15);
		RIT128x96x4StringDraw(minCharTime+1,122,88,15);
  }
	else
	{
		getValue(hourCharAlarm,minCharAlarm,timePtr,setting);
		RIT128x96x4StringDraw(hourCharAlarm,98,88,15);
		RIT128x96x4StringDraw(hourCharAlarm+1,104,88,15);
		RIT128x96x4StringDraw(hourCharAlarm+2,110,88,15);
		RIT128x96x4StringDraw(minCharAlarm,116,88,15);
		RIT128x96x4StringDraw(minCharAlarm+1,122,88,15);
	}
}


void PrintBothHands(Time* timePtr)
{
		// prints both updated clock hands
	RIT128x96x4_Line(timePtr->xPivot, timePtr->yPivot, timePtr->x_minute[timePtr->minute_index%POSITIONS], timePtr->y_minute[timePtr->minute_index%POSITIONS], 6);	
	RIT128x96x4_Line(timePtr->xPivot, timePtr->yPivot, timePtr->x_hour[timePtr->hour_index%POSITIONS], timePtr->y_hour[timePtr->hour_index%POSITIONS], 15);	
}

void getValue(char* a_hours, char* a_min, Time* timePtr,int setting)
{ // none of this code should be re-arranged. It will produce incorrect output it done
	// it will change the hour at the XX:48 minute rather than waiting for the 60th min
	static int prevMinTime,prevHourTime,prevMinAlarm,prevHourAlarm;
	static int flagTime = 0;
	static int flagAlarm = 0;
  static char hoursTime,hoursAlarm;
  static char minutesTime,minutesAlarm;
	
	prevMinTime = minutesTime;
	prevHourTime = hoursTime;
	
	prevMinAlarm = minutesAlarm;
	prevHourAlarm = hoursAlarm;
	
	
	if(setting == TIME)
	{
		minutesTime = (timePtr->minute_index%60);	// 0-59 minutes
		sprintf(a_min,"%02d",minutesTime);
		
		if(flagTime)
		{
			if(minutesTime/12 == 4)
			{
				return;
			}
			else
			{
				hoursTime = ((timePtr->hour_index%60))/5; // 0-11 minutes
				sprintf(a_hours,"%2d",hoursTime);
				flagTime = 0;
			}
		}
		
		// this code cannot be moved around
		if(minutesTime/12 == 4 && timePtr->direction == FORWARDS)
		{
			return;
		}

		// this code cannot be moved around this function moving it below the next
		// if(.) statement produces incorrect  output
		hoursTime = ((timePtr->hour_index%60))/5; // 0-11 minutes
		// I need this so that when advancing forward it stays on 12 if the hour = 0 from
		// the previous statement. this is the main code that runs when everything is moving
		// only forward when pressing the increment button
		if(hoursTime == 0) // zero corresponds to 12 on the clockface
		{
			hoursTime = 12;
		}
		
		if(timePtr->direction == BACKWARDS)
		{
			if(prevMinTime == 0 && minutesTime == 59)
			{
				hoursTime = prevHourTime-1;
				if(hoursTime == 0) // zero corresponds to 12 on the clockface
				{
					hoursTime = 12;
				}
				flagTime = 1;
			}
		}

		sprintf(a_hours,"%2d",hoursTime);
		a_hours[2] = ':';
	}
	// This basically repeats everything that was written above except
	// that is uses its own exclusive variables.
	else if(setting == ALARM)
	{
		minutesAlarm = (timePtr->minute_index%60);	// 0-59 minutes
		sprintf(a_min,"%02d",minutesAlarm);
		
		if(flagAlarm)
		{
			if(minutesAlarm/12 == 4)
			{
				return;
			}
			else
			{
				hoursAlarm = ((timePtr->hour_index%60))/5; // 0-11 minutes
				sprintf(a_hours,"%2d",hoursAlarm);
				flagAlarm = 0;
			}
		}
		
		// this code cannot be moved around
		if(minutesAlarm/12 == 4 && timePtr->direction == FORWARDS)
		{
			return;
		}

		// this code cannot be moved around this function moving it below the next
		// if(.) statement produces incorrect  output
		hoursAlarm = ((timePtr->hour_index%60))/5; // 0-11 minutes
		// I need this so that when advancing forward it stays on 12 if the hour = 0 from
		// the previous statement. this is the main code that runs when everything is moving
		// only forward when pressing the increment button
		if(hoursAlarm == 0) // zero corresponds to 12 on the clockface
		{
			hoursAlarm = 12;
		}
		
		if(timePtr->direction == BACKWARDS)
		{
			if(prevMinAlarm == 0 && minutesAlarm == 59)
			{
				hoursAlarm = prevHourAlarm-1;
				if(hoursAlarm == 0) // zero corresponds to 12 on the clockface
				{
					hoursAlarm = 12;
				}
				flagAlarm = 1;
			}
		}
		sprintf(a_hours,"%2d",hoursAlarm);
		a_hours[2] = ':';
	}
}

// input: unfilled struct
// purpose: to fill the struct (x,y) positions
// for the hour and minute hands
static void CalculateClockHandPositions(Time* timePtr)
{
	int i; // indices
								 
	for(i = 0; i < POSITIONS; i++)
	{
		// This sets the (x,y) coordinates for all possible hour hand coordinates
		// there are 60 coordinates, one for every 5 minute increment
		timePtr->x_hour[i] = (timePtr->initHour_x*1000 + SIN[i]*HOURHANDLENGTH + 500)/1000;
		// the cosine makes it negative so adding HOURHANDLENGTH gets it back to the initial position
		timePtr->y_hour[i] = (timePtr->initHour_y*1000 - COS[i]*HOURHANDLENGTH + 500)/1000 + HOURHANDLENGTH;
	
		
		// This sets the (x,y) coordinates for all possible minute hand coordinates
		// there are 60 coordinates, one position for every minute
		timePtr->x_minute[i] = (timePtr->initMinute_x*1000 + SIN[i]*MINUTEHANDLENGTH + 500)/1000;
		// the cosine makes it negative so adding MINUTEHANDLENGTH gets it back to the initial position
		timePtr->y_minute[i] = (timePtr->initMinute_y*1000 - COS[i]*MINUTEHANDLENGTH + 500)/1000 + MINUTEHANDLENGTH;
	}
}

Time* Time_Init(void)
{
	// create a timePtr to contain the struct
	// of type Time* which is a pointer to a Time struct
	// casted to (Time*) bc thats the data type
	// of size(Time) bc thats how large a struct is
	
	Time* timePtr = (Time*)malloc(sizeof(Time)); 
	
	timePtr->hour_index = 0;
	timePtr->minute_index = 0;
	
	timePtr->xPivot = XPIVOT;
	timePtr->yPivot = YPIVOT;
	
	timePtr->initHour_x = 0;
	timePtr->initHour_y = 0;
	
	timePtr->initMinute_x = 0;
	timePtr->initMinute_y = 0;
	
	// set minute hand to 12 O'clock position
	timePtr->initMinute_x = XPIVOT;
	timePtr->initMinute_y = YPIVOT - MINUTEHANDLENGTH;
													 
	// set minute hand to 12 O'clock position
	timePtr->initHour_x = XPIVOT;
	timePtr->initHour_y = YPIVOT - HOURHANDLENGTH;
	
	// set pointers to position Array
	timePtr->x_hour = &x_hour1[0];
	timePtr->y_hour = &y_hour1[0];
	
	timePtr->x_minute = &x_minute1[0];
	timePtr->y_minute = &y_minute1[0];;
	
	return timePtr;
}

	
static void CreateClockDisplay(Time* timePtr, int back_forw)
{		
	PrintOneHand(timePtr,timePtr->x_minute,timePtr->y_minute,&timePtr->minute_index,back_forw);
	PrintBothHands(timePtr);
	// move the hour hand if the minute hand has move 12 spots, 
	// I added 6 for the same effect as rounding so it looks more fluid
	if(((timePtr->minute_index + 11) % 12) == 0)
	{
		// remove the previous hour hands before and after 
	  // current position for possible setTime() function
		PrintOneHand(timePtr,timePtr->x_hour,timePtr->y_hour,&timePtr->hour_index,back_forw);
		RIT128x96x4_Line(timePtr->xPivot, timePtr->yPivot, timePtr->x_hour[timePtr->hour_index%POSITIONS], timePtr->y_hour[timePtr->hour_index%POSITIONS], 15);
	}
}
		
static void ChangeTimeManually(Time* timePtr,long* seconds,int update)
{
	// now I am not so sure why I included this if() to begin with 
//	if(*seconds == -1)
//	{
//	  // this is here bc we first want to put the clock hands on the screen on the
//		// screen at startup/initialization without them changing automatically to 12:01
//		CreateClockDisplay(timePtr,DEFAULT);
//		(*seconds)++;
//	}
//	else
	{
		if(update == FORWARDS) // move forward
		{ // this is for moving forwards in setTime
			CreateClockDisplay(timePtr,FORWARDS);
			*seconds = 0; // bc we just set a newTime and want to begin a new Sec count
			GPIO_PORTH_DATA_R ^= 0x0D; // flashes & tics the OLED screen
		}
		else if(update == BACKWARDS) // move backward
		{ 
			CreateClockDisplay(timePtr,BACKWARDS);
			*seconds = 0; // bc we just set a newTime and want to begin a new Sec count
			GPIO_PORTH_DATA_R ^= 0x0D; // flashes & tics the OLED screen
		}
		else if(update == DEFAULT) // move backward
		{// at the beginning of the program this sets seconds cnt to 0, where it should be
			CreateClockDisplay(timePtr,DEFAULT);
			(*seconds)++; // bc we just set AlarmTime and want to Revert to current time
		}
	}
}

static void PeriodicTimeChange(unsigned long* count0,long* seconds,Time* timePtr)
{
	static unsigned long temp = 0;

	// 5 interrupts->1	
	// if the count is a multiple of 5, update display
	// Int Freq is 5Hz => LED updates every second
	if(!((*count0)%5) )
	{	
		// this only updates the display when its absolutely necessary
		// and so it doesn't do this for all time until the next interrupt
		if(temp != (*count0))
		{
			GPIO_PORTG_DATA_R = GPIO_PORTG_DATA_R^0x04; // toggle PG2
			temp = (*count0);
			GPIO_PORTH_DATA_R ^= 0x01; // implements a Tic-Toc sound
			// These flash the screen for some reason
			// not sure why
			if(!((*seconds)%60) && ((*seconds) != 0))
			{
				// update the display if its been 60 seconds
				// seconds != 0 is included as a corner case for
				// when the seconds is first initialized so that it
				// doesn't start at 12:01
				CreateClockDisplay(timePtr, FORWARDS);
			}	
			(*seconds)++;	
		}
	}
}

void DisplayFunction(Time* timePtr, unsigned long count0, signed int update)
{ // temp is a local private variable that only has scope within this function
	
	static long seconds = -1;	
	
	// this is for changing any of the alarm times or printing the 
	// previous time after a new alarm/real time was changed
	if((seconds == SETUP) || (update == FORWARDS) || (update == BACKWARDS) || (update == DEFAULT)) // move forward
	{ // this is for moving forwards in setTime
		// Changes minute/
		ChangeTimeManually(timePtr, &seconds,update);
	}
	// this is for moving the hands only when it is 
	// supposed to every minute/hour that occurs
	else  
	{	
		PeriodicTimeChange(&count0,&seconds,timePtr);
	}	
}
	
void Clock_Init(Time* timePtr)//,int forw_back)
{
  CalculateClockHandPositions(timePtr);
	CreateClockFace(timePtr);
	
	DisplayFunction(timePtr,global_count0,0);
}

void SetTime(Time* timePtr, volatile int* global_flag)
{
	static unsigned long prevTime0;
	// clear any pending interrupts that were triggered
	GPIO_PORTB_RIS_R = 0;
	GPIO_PORTF_RIS_R = 0;
	// disable all interrupts except the SysTick_Handler()
	// writing to the port to disable it is a friendly operation for that IRQ
	NVIC_DIS0_R = NVIC_DIS0_INT0; 
	NVIC_DIS0_R = NVIC_DIS0_INT1; 
	NVIC_DIS0_R = NVIC_DIS0_INT30; 
	
	prevTime0 = global_count0;
	// this
	while((global_count0-prevTime0) < 50)
	{
		// ^^^ this waits 10 sec for a switch to be pressed
		// reads PC5(incr) & PC7(decr)
		if(GPIO_PORTC_DATA_R & 0x20)
		{
			SysTick_Wait10ms(1);
			if(GPIO_PORTC_DATA_R & 0x20)
			{
				// increment timePtr index of minute hand and then draw it
				DisplayFunction(timePtr, global_count0,FORWARDS);
				return;
			}	
		}
		else	if(GPIO_PORTC_DATA_R & 0x80)
		{
			SysTick_Wait10ms(1);
			if(GPIO_PORTC_DATA_R & 0x80)
			{
				DisplayFunction(timePtr, global_count0,BACKWARDS);
				return;
			}	
		}
	}
	
	// stop
	*global_flag = 0; // reset flag
  global_count0 = 0; // this resets the seconds count
	
	// re-enable lower priority interrupts
	NVIC_EN0_R = NVIC_EN0_INT0; 
	NVIC_EN0_R = NVIC_EN0_INT1; 
	NVIC_EN0_R = NVIC_EN0_INT30; 
}

int TimerCompare(Time* timePtr, Time* alarmPtr)
{
	int tMin;
	int tHour;
	int aMin;
	int aHour;
	
	tMin = (timePtr->minute_index)%POSITIONS;
	tHour = (timePtr->hour_index)%POSITIONS;
	
	aMin = (alarmPtr->minute_index)%POSITIONS;
	aHour = (alarmPtr->hour_index)%POSITIONS;

	if(tMin == aMin)
	{
		if(tHour == aHour)
		{
			return 1;
		}
	}
	
	return 0;
}
