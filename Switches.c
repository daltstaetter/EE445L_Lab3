// Switches.c

#include "lm3s1968.h"
#include <stdio.h>

extern void SysTick_Wait10ms(unsigned long delay);
extern void DisableInterrupts(void);

extern volatile int flagA0;
extern volatile int flagB0;
extern volatile int flagF1;
extern volatile int flagF2;
extern volatile int clear_flag;

static void Delay(unsigned long count)
{
		while(count)
	{
		count--;
	}
}
	
static void PortA_Init(void) // PL = 0 (Highest) SetTime()
{
//----------- PA4 -> setNewTime(), PL = 0 (Highest)--------------------------
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // enable port A
		Delay(100000); // give it time to enable the port
		GPIO_PORTA_DIR_R &= ~0x10;   // make PA4 input
	  GPIO_PORTA_AFSEL_R &= ~0x10; // disable alt funct on PF0-1
		GPIO_PORTA_DEN_R |= 0x10;    // enable digital I/O on PF0-3
		GPIO_PORTA_IS_R &= ~0x10;    // makes PA0 level-triggered interrupts 
		GPIO_PORTA_IBE_R &= ~0x10;   //sets it so it looks at GPIO_IEV
		GPIO_PORTA_ICR_R = 0x10;     // clear flag0, do this every ISR call
		GPIO_PORTA_IEV_R |= 0x10;    // interrupt triggers on HIGH level
		GPIO_PORTA_IM_R |= 0x10;     // arm interrupt
		NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFFFF1F)|0x0000000; // sets bits 5-7 to 0. pri = 0; 
		NVIC_EN0_R = NVIC_EN0_INT0; // enables intr in PA, its a friendly operation	
		// NVIC_DIS?_R disables interrupts for that particular port letter ?	
//--------------------------------------------------------------------
}

static void PortB_Init(void) // PL = 7 (Lowest) EnAlarm()
{
//----------- PortB PB0 -> EnableAlarm(),Priority Level(PL)=4  ---------
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // enable port B
		Delay(100000); // give it time to enable the port
		GPIO_PORTB_DIR_R &= ~0x01;   // make PB0 input
		GPIO_PORTB_AFSEL_R &= ~0x01; // disable alt funct on PF0-1
		GPIO_PORTB_DEN_R |= 0x01;    // enable digital I/O on PF0-3
		GPIO_PORTB_IS_R &= ~0x01;    // makes PA0 level-triggered interrupts 
		GPIO_PORTB_IBE_R &= ~0x01;   //sets it so it looks at GPIO_IEV
		GPIO_PORTB_ICR_R = 0x01;     // clear flag0, do this every ISR call
		GPIO_PORTB_IEV_R |= 0x01;    // interrupt triggers on HIGH level
		GPIO_PORTB_IM_R |= 0x01;     // arm interrupt
		NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFF8FFF)|0x0000E000; // sets bits 13-15 to 001 respect. pri = 4; 
		NVIC_EN0_R = NVIC_EN0_INT1; // enables intr in PA, its a friendly operation	
//		// NVIC_DIS0_R disables interrupts for that particular port letter	
//----------------------------------------------------------------------
}

static void PortC_Init(void) // increment and decrement buttons
{
//----------- PortB PC0 -> Inc&Dec Minute Hand  ---------
// PC5 -> increment, PC7 -> decrement
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC; // enable port B
		Delay(100000); // give it time to enable the port
		GPIO_PORTC_DIR_R &= ~0xA0;   // make PB5,PC7 input
		GPIO_PORTC_AFSEL_R &= ~0xA0; // disable alt funct on PB5,PC7
		GPIO_PORTC_DEN_R |= 0xA0;    // enable digital I/O on PB5,PC7
//----------------------------------------------------------------------
}

static void PortF_Init(void) // PL = 5 (Medium) chmod() setAlarm()
{
// PortF PF2 -> setAlarmTime() & PF1->DisplayMode(), they have same Priority lvl
//--------------------- Priority Level(PL) = 5 -----------------------------
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
		Delay(100000); // give it time to enable the port
  	GPIO_PORTF_DIR_R &= ~0x06; // make PF2-PF1 inputs
		GPIO_PORTF_AFSEL_R &= ~0x06;// disable alt funct on PF2-1
		GPIO_PORTF_DEN_R |= 0x06; // enable digital I/O on PF2-1
		GPIO_PORTF_IS_R &= ~0x06; // makes PF2-1 level-triggered interrupts 
		GPIO_PORTF_IBE_R &= ~0x06; //sets it so it looks at GPIO_IEV
		GPIO_PORTF_ICR_R = 0x06;     // clear flag0, do this every ISR call
		GPIO_PORTF_IEV_R |= 0x06; // interrupt triggers on HIGH level
		GPIO_PORTF_IM_R |= 0x06; // interrupt triggers on HIGH level
		NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00A00000;//|0x00600000; 
//		
//		// sets bits 21-23 to prior_lvl=3;
//		// bit21 = 1, bit22 = 1, bit23 = 0
    NVIC_EN0_R = NVIC_EN0_INT30; // enables intr in PF, its a friendly operation			
    // NVIC_DIS0_R disables interrupts for that particular port letter
//--------------------------------------------------------------------
}

static void PortG_Init(void)
{
//----------- PortG -------------------------
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOG; // enable port B
		Delay(100000); // give it time to enable the port
		GPIO_PORTG_DIR_R |= 0x04; // enables the on-board LED
		GPIO_PORTG_DEN_R |= 0x04;
	  GPIO_PORTG_AFSEL_R &= ~0x04;
//----------------------------------------------------------------------
}

static void PortH_Init(void)
{
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOH;
	Delay(100000);
	GPIO_PORTH_DIR_R |= 0x0F;
	GPIO_PORTH_DEN_R |= 0x0F;
	GPIO_PORTH_AFSEL_R &= ~0x0F;
	GPIO_PORTH_DATA_R = 0x00;
}

void GPIO_Ports_Init(void)
{
	PortA_Init();
	PortB_Init();
	PortC_Init();
	PortF_Init();
	PortG_Init();
	PortH_Init();
}

void GPIOPortA_Handler(void) // PL = 0 (Highest) SetTime()
{
	// PL = 0 (Highest) SetTime()
	
	// acknowledge the interrupt
	// we only have to check bit 0 
	GPIO_PORTA_ICR_R = 0x10;     
	SysTick_Wait10ms(1); // switch debouncing
	
	if(GPIO_PORTA_DATA_R & 0x10)
	{
		//printf("PA0->setTime");
		flagA0 = 1; // used to enter SetTime()
	}
}

void GPIOPortB_Handler(void) // PL = 7 (Lowest) EnAlarm()
{ 
	// PL = 7 (Lowest) EnAlarm()
	
	// acknowledge the interrupt
	// we only have to check bit 0 
	GPIO_PORTB_ICR_R = 0x01;
	SysTick_Wait10ms(1); // switch debouncing
	
	if(GPIO_PORTB_DATA_R & 0x01)
	{
		if(flagB0 == 1)
		{
			flagB0 = 0; // disables alarm once this occurs
								  // while the alarm is already going off
		}
		else
		{
			flagB0 = 1; // enables alarm
		}
	}
	
	// this leave with flagB0 = 1, enabling the alarm or
	// or leaves with flagB0 = disabling the alarm
  // you press the same button to enable it as you do to disable it
}

void GPIOPortF_Handler(void) // PL = 5 (Medium) PF1->setAlarm() PF2->chmod() 
{
	// PL = 5 (Medium) chmod() setAlarm()
	
	// acknowledge the interrupt
	// we only have to check
	// both bit 0 and bit 1 
	GPIO_PORTF_ICR_R = 0x01;
	SysTick_Wait10ms(1); // switch debouncing
	
	// PF1 interrupt -> SetAlarm()
	if(GPIO_PORTF_RIS_R & 0x00000002)
	{
		// ACK interrupt
		GPIO_PORTF_ICR_R = 0x02;  // clear flag, ack intrp
		if(GPIO_PORTF_DATA_R & 0x02)
		{
			flagF1 = 1;
		}
	}
	// PF2 interrupt -> change Display mode
	else if(GPIO_PORTF_RIS_R & 0x00000004)
	{
		// ACK interrupt
		GPIO_PORTF_ICR_R = 0x04; 
		if(GPIO_PORTF_DATA_R & 0x04)
		{
			if(flagF2) // if digital is on flagF2=0 turns it off
			{
				flagF2 = 0;
				clear_flag = 1;
			}
			else
			{
				flagF2 = 1;
				clear_flag = 1;
			}
		}
	}
}







