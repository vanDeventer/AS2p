/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: May 22, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 

/*
 * Purpose of this version:
 * The purpose of this version of the software package is to introduce internal interrupts.
 * It does that by using Timer Counters. (You might should use an oscilloscope connected between LED3 and R20.)
*/

volatile unsigned char buttons;		// This registers holds a copy of PINC when an external interrupt 6 has occurred.
volatile unsigned char bToggle = 0;	// This registers is a boolean that is set when an interrupt 6 occurs and cleared when serviced in the code.
//These "volatile" registers is available outside of the main loop (i.e., to the interrupt handlers)

#include <avr/io.h> // input output header file for this AVR chip.
#include <avr/pgmspace.h>	// Contains some type definitions and functions.
#include <avr/interrupt.h>	// Contains ISR (Interrupt Service Routines) or interrupt handler details.
#include "gpio.h"	// set the data direction of our io ports.

int initExtInt(void)
{
	//Set up external Interrupts
	// The five Switches are ORed to Pin PE6 which is alternatively Int6
	EICRB |= (0<<ISC61) | (1<<ISC60);  //Any logical change to INT6 generates an interrupt
	EIMSK |= (1<<INTF6);
	return(6);
}


int initTimer2()
{
	/// Set up an internal Interrupt that will occur every 5 milliseconds.
	/// It uses the Timer Counter 2 in CTC mode with a pre-scaler of 256 and a value of 155 (it should be 155.25).
	//
	TCCR2A = (1<<WGM21); // | (0<<WGM20);  //CTC mode
	//TCCR2A |= (0<<COM2A1) | (0<<COM2A0); // Mormal port operation, OC2A is disconnected.
	TCCR2A |= (1<<CS22) | (1<<CS21); //| (0<<CS20); /// Divide source frequency source by 256.
	TCNT2 = 0;	/// Make sure the timer counter is set to 0.
	OCR2A = 100;
	TIMSK2 = (1<<OCF2A); // Interrupt flag register to enable output compare.
	return(2);
}


int main(void)
{
	unsigned char temp = 0x0F;		// Allocate memory for temp. It is initialized to 15 for demonstration purposes only.
		
	temp = initGPIO();				// Set up the data direction register for both ports C and G
	temp = initTimer2();			// Setup 5ms internal interrupt
	sei();							// Set Global Interrupts
		
	while(1)
	{
		temp++;
	}

	return(0); // This line of code will never be executed since it is after the "while(1)" block
}

SIGNAL(SIG_OUTPUT_COMPARE2)
{
	if (PINC & 0x01)
		PORTC &= 0x00;	//Turn on Led3 if S3 is on
	else
		PORTC |= 0x01;
}