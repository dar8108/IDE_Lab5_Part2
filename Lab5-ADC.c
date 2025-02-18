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

       // Switch 1 pressed 2nd time
       if (Timer1RunningFlag)
       {
           Timer1RunningFlag = FALSE;
           Timer32_1_Disable();
       }
       else // Switch 1 pressed 1st time
       {
           Timer1RunningFlag = TRUE;
           Timer32_1_Enable();
       }       
   }      
}
   
void Switch1_Interrupt_Init(void)
{
   // disable interrupts
   DisableInterrupts();
    
   // initialize the Switch as per previous lab
   Switch1_Init();
		
   //7-0 PxIFG RW 0h Port X interrupt flag
   //0b = No interrupt is pending.
   //1b = Interrupt is pending.
   // clear flag1 (reduce possibility of extra interrupt)	
   SWITCH_1_PORT->IFG &= ~SWITCH_1_PIN;

   //7-0 PxIE RW 0h Port X interrupt enable
   //0b = Corresponding port interrupt disabled
   //1b = Corresponding port interrupt enabled	
   // arm interrupt on  P1.1	
   SWITCH_1_PORT->IE |= SWITCH_1_PIN;

   //7-0 PxIES RW Undefined Port X interrupt edge select
   //0b = PxIFG flag is set with a low-to-high transition.
   //1b = PxIFG flag is set with a high-to-low transition
   // now set the pin to cause falling edge interrupt event
   // P1.1 is falling edge event
   SWITCH_1_PORT->IES |= SWITCH_1_PIN; 
	
   // now set the pin to cause falling edge interrupt event
   NVIC_IPR8 = (NVIC_IPR8 & 0x00FFFFFF)|0x40000000; // priority 2
	
   // enable Port 1 - interrupt 35 in NVIC	
   NVIC_ISER1 = 0x00000008;  
	
   // enable interrupts  (// clear the I bit	)
   EnableInterrupts();              
	
}

// main
int main(void)
{
	int tmp36_mV;
    int tempC;
    
	//initializations
	uart0_init();
	uart0_put("Lab5 ADC demo\r\n");

	Switch1_Interrupt_Init();
    // Set the Timer32-1 to 2Hz (0.5 sec between interrupts)
    Timer32_1_Init(&Timer32_1_ISR, SystemCoreClock/2, T32DIV1); // initialize Timer A32-1;
    Timer32_1_Enable();
	ADC0_InitSWTriggerCh6();
	EnableInterrupts();
    
    while(1)
    {
        if (Timer1RunningFlag)
        {
            #ifdef PHOTOCELL
	        uart0_put("ADC Input: ");
            putnumHex(analogIn);
            uart0_putchar('\t');
            putnumU(analogIn);
            uart0_put("\r\n");  
            #else
            uart0_put("Temp in Celsius: ");
            tmp36_mV = 3300*analogIn/16384.0;
            tempC = (tmp36_mV - 750) / 10.0;
            putnumU(tempC);
            uart0_put("\n\rTemp in Fahrenheit: ");
            putnumU(tempC*9/5.0 + 32);
            uart0_put("\r\n");  
            #endif                      
        }
    }
}

