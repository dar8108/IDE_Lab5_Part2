/*
* Rochester Institute of Technology
* Department of Computer Engineering
* CMPE 460  Interfacing Digital Electronics
* LJBeato
* 1/14/2021
*
* Filename: main_timer_template.c
*/
#include <stdio.h>
#include <stdlib.h>

#include "msp.h"
#include "uart.h"
#include "switches.h"
#include "Timer32.h"
#include "CortexM.h"
#include "Common.h"
#include "ADC14.h"
// The sprintf function seemed to cause a hange in the interrupt service routine.
// I think if we increase the HEAP size, it will work
// change to Heap_Size       EQU     0x00000200 in startup_msp432p401r_uvision.s


BOOLEAN Timer1RunningFlag = FALSE;
BOOLEAN Timer2RunningFlag = FALSE;

unsigned long MillisecondCounter = 0;
unsigned int analogIn = 0;


// Interrupt Service Routine for Timer32-1
void Timer32_1_ISR(void)
{
    analogIn = ADC_In();
}

// Interrupt Service Routine for Timer32-2
void Timer32_2_ISR(void)
{

}

// PORT 1 IRQ Handler
// LJBeato
// Will be triggered if any pin on the port causes interrupt
//
// Derived From: Jonathan Valvano
void PORT1_IRQHandler(void)
{
	// First we check if it came from Switch1 ?
   if(P1->IFG & SWITCH_1_PIN)  
   {
       // acknowledge P1.1 is pressed, by setting BIT1 to zero - remember P1.1 is switch 1   
	   // clear flag, acknowledge   
       SWITCH_1_PORT->IFG &= ~SWITCH_1_PIN;

       // Switch 1 pressed --> start timer
       if (!Timer1RunningFlag)
       {
           Timer1RunningFlag = TRUE;
           Timer32_1_Enable();
       }
       else // Pressed again --> stop timer
       {
           Timer1RunningFlag = FALSE;
           Timer32_1_Disable();
       }           
   }
   
}


// main
int main(void)
{
	//char temp[64];
    
	//initializations
	uart0_init();
	uart0_put("Lab5 ADC demo\r\n");

	Switch1_Init();
    // Set the Timer32-1 to 2Hz (0.5 sec between interrupts)
    Timer32_1_Init(&Timer32_1_ISR, SystemCoreClock/2, T32DIV1); // initialize Timer A32-1;
	ADC0_InitSWTriggerCh6();
	EnableInterrupts();
    
    while(1)
    {
	    uart0_put("ADC Input: ");
        putnumHex(analogIn);
        uart0_put("\n\r");
    }
}

