// OledDisplay.h

#define POSITIONS 60
#define MINUTEHANDLENGTH 33 // length of the minute hand on OLED 
#define HOURHANDLENGTH 24   // length of the hour hand on OLED 
#define NULL 0
#define XPIVOT 55
#define YPIVOT 47
#define FORWARDS 1
#define DEFAULT 0
#define BACKWARDS -1
#define SETUP -1
#define TIME 1
#define ALARM 0


extern void SysTick_Wait10ms(unsigned long delay);
extern volatile unsigned long global_count0;
extern volatile int flagA0;



static int x_hour1[POSITIONS];
static int y_hour1[POSITIONS];

static int x_minute1[POSITIONS];
static int y_minute1[POSITIONS];

typedef struct Time
{
	// index into position array
	int hour_index;
	int minute_index;
	
	// arrays containing (x,y) coordinates for hour hand
	int* x_hour;
	int* y_hour;
	
	// arrays containing (x,y) coordinates for minute hand
	int* x_minute;
	int* y_minute;	
	
	// center of clock from which minute and hour hand stem from
	int xPivot; 
	int yPivot;
	
	// initial position for the default time for the Minute hand used 
	// as the basis for calculating the position of every other position 
	// the minute hand can take on as it sweeps around the clock
	int initMinute_x;
	int initMinute_y;
	
	// initial position for the default time for the hour hand used
	// as the basis for calculating the position of every other position
	// the hour hand can take on as it sweeps around the clock
	int initHour_x;
	int initHour_y;
	
	int direction;
}Time;

// ____user defined functions D.A. 2/4/2014
Time* Time_Init(void);
void SetTime(Time* timePtr, volatile int* global_flag);
void Clock_Init(Time* timePtr);
void DisplayFunction(Time* timePtr, unsigned long count0, signed int update);
void SetAlarmTime(Time* alarmPtr);
int TimerCompare(Time* timePtr, Time* alarmPtr);
void getValue(char* a_hours, char* a_min, Time* timePtr,int setting);
void DigitalTime(Time* timePtr, int setting);




































