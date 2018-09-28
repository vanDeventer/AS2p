/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: September 26, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 

/*
 * Purpose of this version:
 * The purpose of this version is to demonstrate CAN communication with a car.
*/

#include <stdio.h>
#include <string.h>
#include <avr/io.h> // input output header file for this AVR chip.
#include <avr/pgmspace.h>	// Contains some type definitions and functions.
#include <avr/interrupt.h>	// Contains ISR (Interrupt Service Routines) or interrupt handler details
#include "global.h"
#include "lcd.h"
#include "gpio.h"
#include "can_lib.h"
#include "can_drv.h"
#include "ECU_codes.h"


#define DISPLAYLENGTH 16	/* number of characters on the display */
#define DTOP DCAN /* this needs to be updated when new states are added to enumeration of dState */
#define BUTTON_PORT PINC
#define BUTTON_1 (1<<3)
#define BUTTON_2 (1<<4)
#define BUTTON_3 (1<<5)
#define BUTTON_4 (1<<6)
#define BUTTON_5 (1<<7)

enum dStates {DBOOT, DADC, DTEXT, DCAN};	/* enumeration of states (C programming, p) */
enum subMode {NORMAL, TEDIT, UECHO};
char *dbStateName[] = {"Auto Sys CAN", "ADC", "Text", "CAN"}; /* initialization of Pointer Array (C programming, p113) */	
volatile unsigned int dbState;		/* display's state (activated by buttons)*/
volatile unsigned char buttons;		// This registers holds a copy of PINC when an external interrupt 6 has occurred.
volatile unsigned char bToggle = 0;	// This registers is a boolean that is set when an interrupt 6 occurs and cleared when serviced in the code.
volatile unsigned char textEdit= 0;	// Boolean to enable text editing.
volatile uint16_t adc_value;  //Allocate the double byte memory space into which the result of the 10 bits Analog to Digital Converter (ADC) is stored.

volatile uint16_t can_time = 0;
uint16_t speed = 500;
uint8_t extended = 0;
uint32_t supported_PIDs = 0;
uint8_t rxbuffer[8], txbuffer[8], textbuffer[17];

int initADC(){
	//Set up analog to digital conversion (ADC)
	//ADMUX register
	//AVcc with external capacitor on AREF pin (the 2 following lines)
	ADMUX &= ~(1<<REFS1);  //Clear REFS1 (although it should be 0 at reset)
	ADMUX |= (1<<REFS0);   //Set REFS0
	ADMUX &= (0b11100000); //Single ended input on ADC0
	ADMUX &= ~(1<<ADLAR);  //Making sure ADLAR is zero (somehow it was set to 1)
	//The ACDC control and status register B ADCSRB
	ADCSRB &= ~(1<<ADTS2) & ~(1<<ADTS1) & ~(1<<ADTS0);  //Free running mode
	//The ADC control and status register A ADCSRA
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) |(1<<ADPS0);//set sampling frequency pre-scaler to a division by 128
	ADCSRA |= (1<<ADEN)  | (1<<ADATE) | (1<<ADIE);//enable ADC, able ADC auto trigger, enable ADC interrupt
	return(0);
}

/** This function sets up the External Interrupt, and specifically Interrupt 6 that is connected to the push buttons */
int initExtInt(void)
{
	//Set up external Interrupts
	// The five Switches are ORed to Pin PE6 which is alternatively Int6
	EICRB |= (0<<ISC61) | (1<<ISC60);  //Any logical change to INT6 generates an interrupt
	EIMSK |= (1<<INTF6);
	return(0);
}

/** This function initializes the LCD display and should be called only once before the while(1) loop in the main(). */
int initDisplay(void)
{
	dbState = DBOOT;
	lcdInit();	//initialize the LCD
	lcdClear();	//clear the LCD
	lcdHome();	//go to the home of the LCD
	lcdPrintData(dbStateName[dbState], strlen(dbStateName[dbState])); //Display the text on the LCD
	return(0);
}

/** This function initializes the TimerCounter 0 to provide a PWM to dim the display (backlight) */
void TimerCounter0setup(int start)
{
	//Setup mode on Timer counter 0 to PWM phase correct
	TCCR0A = (0<<WGM01) | (1<<WGM00);
	//Set OC0A on compare match when counting up and clear when counting down
	TCCR0A |= (1<<COM0A1) | (1<<COM0A0);
	//Setup pre-scaller for timer counter 0
	TCCR0A |= (0<<CS02) | (0<<CS01) | (1<<CS00);  //Source clock IO, no pre-scaling

	//Setup output compare register A
	OCR0A = start;
}

void can_time_reset(void)
{
	can_time = CANTIM;
}

int16_t can_time_lapsed(void)
{
	return (can_time-CANTIM);
}

uint8_t OBD_PID_check(uint8_t pid)
{
	uint16_t u16_temp = 0;
	uint8_t u8_temp = 0;
	// calculated from http://en.wikipedia.org/wiki/OBD-II_PIDs
	switch (pid)
	{
		//The first 32 supported PIDs
		case 0:		
					// CRAP AVR cant handle 32-bit operations, thats why all the casting...
					supported_PIDs = ((uint32_t)rxbuffer[3]<<24)|((uint32_t)rxbuffer[4]<<16)|((uint32_t)rxbuffer[5]<<8)|((uint32_t)rxbuffer[6]<<0);
					break;								
								
		case ENGINE_COOLANT_TEMP:               //     A-40              [degree C]
					u16_temp = rxbuffer[3] - 40;
					u8_temp = snprintf(textbuffer, 16,"%d degC ", u16_temp);
					//lcdClear();
					//lcdHome();
					//lcdPrintData("Coolant temp:",13);
					lcdGotoXY(0,1);
					lcdPrintData(textbuffer, u8_temp);
					lcdGotoXY(0,0);
					break;
									
		case ENGINE_RPM:                        //   ((A*256)+B)/4    [RPM]
					u16_temp = ((rxbuffer[3]<<8) + rxbuffer[4])/4;
					u8_temp = snprintf(textbuffer, 16,"%u rpm ", u16_temp);
					//lcdClear();
					//lcdHome();
					//lcdPrintData("Engine rpm:",11);
					lcdGotoXY(0,1);
					lcdPrintData(textbuffer, u8_temp);
					lcdGotoXY(0,0);
					break;
									
		case VEHICLE_SPEED:                     // A                  [km/h]
					u16_temp = rxbuffer[3];
					u8_temp = snprintf(textbuffer, 16, "%u km/h ", u16_temp);
					//lcdClear();
					//lcdHome();
					//lcdPrintData("Vehicle speed:",14);
					lcdGotoXY(0,1);
					lcdPrintData(textbuffer, u8_temp);
					lcdGotoXY(0,0);
					break;
									
		case THROTTLE:                          // A*100/255
					u16_temp = (uint16_t)rxbuffer[3]*100/255;
					u8_temp = snprintf(textbuffer, 16, "%d %% ", u16_temp);
					lcdClear();
					lcdHome();
					lcdPrintData("Throttle:",9);
					lcdGotoXY(0,1);
					lcdPrintData(textbuffer, u8_temp);
					lcdGotoXY(0,0);
					break;
									
		case ECU_VOLTAGE:                       //((A*256)+B)/1000 [V]
					u16_temp = (rxbuffer[3]<<8) + rxbuffer[4];
					u8_temp = snprintf(textbuffer, 16, "%u.%u V", u16_temp/1000, u16_temp%1000);
					lcdClear();
					lcdHome();
					lcdPrintData("ECU voltage:",12);
					lcdGotoXY(0,1);
					lcdPrintData(textbuffer, u8_temp);
					lcdGotoXY(0,0);
					break;
		case INTAKE_TEMP:                       // A - 40
					u16_temp = rxbuffer[3] - 40;
					u8_temp = snprintf(textbuffer, 16, "%d degC ", u16_temp);
					lcdClear();
					lcdHome();
					lcdPrintData("Intake temp:",12);
					lcdGotoXY(0,1);
					lcdPrintData(textbuffer, u8_temp);
					lcdGotoXY(0,0);
					break;
					
		default:
					pid = 0;
					break;
                        
	}
	
	return pid;
}

//uint8_t OBD_poll_pins(void)
//{
	//uint8_t pid;
	//
	//// Poll the buttons and set the pid. For more info on OBD-II PID see http://en.wikipedia.org/wiki/OBD-II_PIDs
	//if (BUTTON_PORT & BUTTON_1)
	//{
		//pid = ENGINE_COOLANT_TEMP;
	//}
	//else if (BUTTON_PORT & BUTTON_2)
	//{
		//pid = THROTTLE;
	//}
	//else if (BUTTON_PORT & BUTTON_3)
	//{
		//pid = VEHICLE_SPEED;
	//}
	//else if (BUTTON_PORT & BUTTON_4)
	//{
		//pid = ECU_VOLTAGE;
	//}
	//else if (BUTTON_PORT & BUTTON_5)
	//{
		//pid = INTAKE_TEMP;
	//}
	//else
	//{
		//pid = ENGINE_RPM;
	//}
	//return pid;
//}


/** This function is called when cycling up the states.*/
int dbStateUp(void)
{
	if (++dbState > DTOP)
		dbState = DADC;
	lcdClear();	//clear the LCD
	lcdHome();	//go to the home of the LCD
	lcdPrintData(dbStateName[dbState], strlen(dbStateName[dbState])); //Display the text on the LCD
	return 0;
}

/** This function is called when cycling down the states.*/
int dbStateDown(void)
{
	if (dbState-- <= DADC)
		dbState = DTOP;
	lcdClear();	//clear the LCD
	lcdHome();	//go to the home of the LCD
	lcdPrintData(dbStateName[dbState], strlen(dbStateName[dbState])); //Display the text on the LCD
	return 0;
}

/** This function uses the push buttons to let the user to change states upon boot up.*/
int DbBOOThandler(void)
{
	switch(buttons & 0b11111000){
		case 0b10000000:			//S5 center button
			/* do nothing */
			break;
		case 0b01000000:			//S4  upper button
			dbStateUp();
			break;
		case 0b00100000:			//S3 left button
			/* do nothing */
			break;
		case 0b00010000:			//S2 lower button
			dbStateDown();
			break;
		case 0b00001000:			//S1 right button
			/* do nothing */
			break;
		default:
			/* button released, do nothing */
			break;
	}
	return 0;
}

/** This function uses the push buttons to let the user change states and potentially channels while in ADC state.*/
int DbADChandler(void)
{
	switch(buttons & 0b11111000){
		case 0b10000000:			//S5 center button
			/* do nothing */
			break;
		case 0b01000000:			//S4  upper button
			dbStateUp();
			break;
		case 0b00100000:			//S3 left button
			/* change channel using ADMUX */
			break;
		case 0b00010000:			//S2 lower button
			dbStateDown();
			break;
		case 0b00001000:			//S1 right button
			/* change channel using ADMUX */			
			break;
		default:
			/* do nothing */
		break;
	}
	return 0;
}


/** This function uses the push buttons to let the user write text.*/
unsigned int DbTEXThandler(char *s, unsigned int position)
{
	switch(buttons & 0b11111000)
	{
		case 0b10000000:			//S5 center button
			PORTC |= 0b00000100;	//Turn on LED5
			textEdit = !textEdit;
			if (textEdit){
				lcdGotoXY(10, 0);     //Position the cursor on
				lcdPrintData("input", 5); //Inform the user of edit mode in upper right part of the LCD
			} else {
				lcdGotoXY(10, 0);     //Position the cursor on
				lcdPrintData("     ", 5); //Clear the upper right part of the LCD
			}
			break;
		case 0b01000000:			//S4  upper button
			if (s[position] < 'z' && textEdit)	// if you did not reach the ASCII letter z or 0x7A, you can go to the next letter in the table
				s[position]++;
			else if (!textEdit)
				dbStateUp();
			break;
		case 0b00100000:			//S3 left button
			if (position > 0 && textEdit) 		//If you have not reached the right side of the display, you can move the cursor one step to the right
				position--;
			break;
		case 0b00010000:			//S2 lower button
			if (s[position] > 'A' && textEdit)	// if you did not reach the ASCII letter A, you can go to the previous letter in the table
				s[position]--;
			else if (!textEdit)
				dbStateDown();
			break;
		case 0b00001000:			//S1 right button
			if (position < (DISPLAYLENGTH-1) && textEdit)		//If you have not reached the right side of the display with index starting at 0, you can move the cursor one step to the right
			{
				position++;
				s[position] = 'A';
				s[position+1] ='\0';
			}
			break;
		default:
			PORTC &= 0b11111011;	//Turn off LED5
			break;
	}
	return position;
}

/** This function uses the push buttons to let the user to change states upon boot up.*/
uint8_t DbCAN(void)
{
	uint8_t pid;
	
	switch(buttons & 0b11111000){
		case 0b10000000:			//S5 center button
			pid = VEHICLE_SPEED;
			lcdGotoXY(10, 0);     //Position the cursor on
			lcdPrintData("Speed", 5); //Inform the user of edit mode in upper right part of the LCD
			break;
		case 0b01000000:			//S4  upper button
			dbStateUp();
			break;
		case 0b00100000:			//S3 left button
			pid = ENGINE_RPM;
			lcdGotoXY(10, 0);     //Position the cursor on
			lcdPrintData("RPM  ", 5); //Inform the user of edit mode in upper right part of the LCD
			break;
		case 0b00010000:			//S2 lower button
			dbStateDown();
			break;
		case 0b00001000:			//S1 right button
			pid = ENGINE_COOLANT_TEMP;
			lcdGotoXY(10, 0);     //Position the cursor on
			lcdPrintData("Tempe", 5); //Inform the user of edit mode in upper right part of the LCD
			break;
		default:
			/* button released, do nothing */
			break;
	}
	return pid;
}


/** This is the main function of our application. There is one and only one such function.
 * The code starts executing here.
 */
int main(void)
{
	unsigned char temp ;		//Allocate memory for  temp
 	char cursor = 0;				/* allocate a variable to keep track of the cursor position and initialize it to 0 */
	char textLine[DISPLAYLENGTH + 1];	/* allocate a consecutive array of 16 characters where your text will be stored with an end of string */
	char text[10];				//Allocate an array of 10 bytes to store text
	uint16_t adcBuffer;			// Allocate the memory to hold ADC results that is not disturbed by interrupts
	
		textLine[0] = 'A';				/* initialize the first ASCII character to A or 0x41 */
	textLine[1] = '\0';				/* initialize the second character to be an end of text string */

	temp = initGPIO();		//Set up the data direction register for both ports B, C and G
	temp = initDisplay();	//Set up the display
	temp = initExtInt();	//Set up the external interrupt for the push buttons
	temp = initADC();		// Setup the Analog to Digital Converter
	TimerCounter0setup(128);// enable the dimming of the display backlight with PWM using TimerCounter 0 and pin OCR0
	ADCSRA |= (1<<ADSC);	//Start ADC
	
	
	
	
	uint8_t pid = 0;				// Parameter id (OBD-II PIDs (On-board diagnostics Parameter IDs) are codes used to request data from a vehicle, used as a diagnostic tool.)
	st_cmd_t txmessage, rxmessage;	// Type defined in Atmel files.
	uint8_t u8_temp, i;
	uint16_t u16_temp;
	
	// Clock pre-scaler Reset
    CLKPR = 0x80;  
	CLKPR = 0x00;
	
	// Reset CAN peripheral
	Can_reset();
	
	// Allow the CAN peripheral to auto configure the baud rate
	//can_auto_baudrate(CAN_AUTOBAUD);
	
	// Configure the can to 500kbit
	can_init(CAN_BAUDRATE);
	
	// Set pre-scaler to can timer.
	CANTCON = CANBT1;
	
	sei();					// Set Global Interrupts

	while(1)
	{
		ADCSRA &= ~(1<<ADIE);		//disable ADC interrupt to prevent value update during the conversion
		adcBuffer = adc_value;
		ADCSRA |= (1<<ADIE);		//re-enable ADC interrupt
		OCR0A = adcBuffer >> 2;		// using the top 8 bits of the ADC, load OCR0A to compare to the timer Counter 0 to generate aPWM for the display back light
		if (adcBuffer>852){			// turning on the LEDs as a function of the ADC, regardless of the state the software is in.
			PORTC = 0b00000111;
			PORTG |= 0b00000011;
		}
		else if(adcBuffer>682){
			PORTC = 0b00000011;
			PORTG |= 0b00000011;
		}
		else if(adcBuffer>511){
			PORTC = 0b00000001;
			PORTG |= 0b00000011;			
		}
		else if(adcBuffer>341){
			PORTC = 0b00000000;
			PORTG |= 0b00000011;
		}
		else if(adcBuffer>170){
			PORTC = 0b00000000;
			PORTG |= 0b00000001;
			PORTG &= 0b11111101;
		}
		else{
			PORTC = 0b00000000;
			PORTG &= 0b00000000;
		}
		
		
// --- Here start the CAN loop -----------------

		
		txmessage.pt_data = &txbuffer[0];	// Point to the data to be sent
		// Build the data to be sent
		txbuffer[0] = 0x02;
		txbuffer[1] = 0x01;
		txbuffer[2] = 0;	// First, get supported PIDs
		txbuffer[3] = 0;
		txbuffer[4] = 0;
		txbuffer[5] = 0;
		txbuffer[6] = 0;
		txbuffer[7] = 0;
	
		// Send both a 11-bit frame and 29-bit frame and listen to what we receive.
		rxmessage.pt_data = &rxbuffer[0];
		for(i=0; i<8; i++) rxbuffer[i]=0;
		
		// --- Rx Command
		rxmessage.cmd = CMD_RX;
		
		// --- Enable Rx
		while(can_cmd(&rxmessage) != CAN_CMD_ACCEPTED);

		txmessage.ctrl.ide = 0;				// Extended format
		txmessage.id.std = PID_REQUEST;		// Set ID to "0x7DF"
		txmessage.dlc = 8;					// Length of data
		txmessage.cmd = CMD_TX_DATA;		// Command used by the CAN driver
	
		// --- Enable Tx (send)
		while(can_cmd(&txmessage) != CAN_CMD_ACCEPTED);
		
		// --- Wait for Tx completed        
		while(1)
		{
			u8_temp = can_get_status(&txmessage);
			if (u8_temp != CAN_STATUS_NOT_COMPLETED) break; // Out of while
		}
	
		txmessage.ctrl.ide = 1;				// Extended format
		txmessage.id.ext = 0x18DB33F1;		// Set ID to "0x18DB33F1"
		txmessage.dlc = 8;					// Length of data
		txmessage.cmd = CMD_TX_DATA;		// Command used by the CAN driver
	
		// --- Enable Tx (send)
		while(can_cmd(&txmessage) != CAN_CMD_ACCEPTED);
		// --- Wait for Tx completed        
		while(1)
		{
			u8_temp = can_get_status(&txmessage);
			if (u8_temp != CAN_STATUS_NOT_COMPLETED) break; // Out of while
		}
		
		// Reset timer
		can_time_reset();
		
		while(1)
		{			
			// --- Wait for Rx completed
			while(1)
			{
				u8_temp = can_get_status(&rxmessage);
				// Break if RX complete or timeout after ~100ms.
				if (u8_temp != CAN_STATUS_NOT_COMPLETED || can_time_lapsed() > 30000) break; // Out of while				
			}	
			
			// if timeout then abort ~100ms
			if ( can_time_lapsed() > 30000)
			{
				rxmessage.cmd = CMD_ABORT;
				can_cmd(&rxmessage);
				delay_ms(250);
				rxmessage.cmd = CMD_RX_DATA;
				break;
			}
					
			// ---- Exit if CAN error(s)	
			if (u8_temp == CAN_STATUS_ERROR || rxmessage.cmd == CMD_ABORT) 
			{
				// Handle ERROR
				break; // Out of the function
			}
			else if (rxmessage.id.std == PID_REPLY || rxmessage.id.ext == 0x18DAF110) // Is the received packet a pid reply from the ECU?
			{
				//rxbuffer[1] = rxbuffer[1] - 0x40; // remove 0x40 from the mode reply to match the mode request. However, not needed in this demo
			
				// Is it a reply on the requested pid?
				if (rxbuffer[2] == pid)
				{
					//Check what PID it is and print it.
					OBD_PID_check(pid);
				}	
				
				delay_ms(250);
		
//				pid = OBD_poll_pins();
			
				// Stupid AVR!! It has problems to resolve 32-bit variables. This is not needed when debugging, however it might be good to know it the ECU supports the requested PID.
				/*
				if (!((uint32_t)supported_PIDs & (uint32_t)1<<(32-pid))) // Check if PID is supported by ECU
				{
					lcdHome();
					lcdPrintData(" ERROR: PID not ",16);
					lcdGotoXY(0,1);
					lcdPrintData("   supported!!  ",16);
					pid = 0;
				}				
				*/
						
				txmessage.pt_data = &txbuffer[0];	// Point to the data to be sent
				// Build the data to be sent
				txbuffer[0] = 0x02;
				txbuffer[1] = 0x01;
				txbuffer[2] = pid;	
				txbuffer[3] = 0;
				txbuffer[4] = 0;
				txbuffer[5] = 0;
				txbuffer[6] = 0;
				txbuffer[7] = 0;
					
				if ( rxmessage.ctrl.ide == 0 )	// received message is STD format
				{
					txmessage.ctrl.ide = 0;				// Extended format
					txmessage.id.std = PID_REQUEST;		// Set ID to "0x7DF"
					txmessage.dlc = 8;					// Length of data
					txmessage.cmd = CMD_TX_DATA;		// Command used by the CAN driver
				}
				else							// received message is EXT format
				{
					txmessage.ctrl.ide = 1;				// Extended format
					txmessage.id.ext = 0x18DB33F1;		// Set ID to "0x18DB33F1"
					txmessage.dlc = 8;					// Length of data
					txmessage.cmd = CMD_TX_DATA;		// Command used by the CAN driver
				}
								
            
				// --- Enable Tx (send)
				while(can_cmd(&txmessage) != CAN_CMD_ACCEPTED);
		
				// --- Wait for Tx completed        
				while(1)
				{
					u8_temp = can_get_status(&txmessage);
					if (u8_temp != CAN_STATUS_NOT_COMPLETED) break; // Out of while
				}	
				// --- Enable Rx
				while(can_cmd(&rxmessage) != CAN_CMD_ACCEPTED);			
				// Reset timer
				can_time_reset();
					
			}
			
		}		
		
		
// -------------------------------------------------------------------------------		
		

		
		if (bToggle)			//This is set to true only in the interrupt service routine at the bottom of the code
		{
			switch (dbState){
				case DBOOT:
					DbBOOThandler();
					break;
				case DADC:
					DbADChandler();
					break;
				case DTEXT:
					cursor = DbTEXThandler(textLine, cursor);
					break;
				case DCAN:
					pid = DbCAN();
					break;
				default:
					break;
			}
			bToggle = 0;			// clear the flag.
		}
		
		switch (dbState){
			case DBOOT:
			case DCAN:
				break;
			case DTEXT:
				lcdGotoXY(0, 1);     //Position the cursor on
				lcdPrintData(textLine, strlen(textLine)); //Display the text on the LCD
				break;
			case DADC:
				itoa(adcBuffer, text, 10);	//Convert the unsigned integer to an ascii string; look at 3.6 "The C programming language"
				lcdGotoXY(5, 1);     //Position the cursor on
				lcdPrintData("      ", 6); //Clear the lower part of the LCD
				lcdGotoXY(5, 1);     //Position the cursor on
				lcdPrintData(text, strlen(text)); //Display the text on the LCD
				break;
			default:
				lcdGotoXY(0, 1);     //Position the cursor on the first character of the first line
				lcdPrintData("You have a bug!", 15); //Inform of the problem
				break;
		}
	}
}


/* the functions below are Interrupt Service Routines, they are never called by software */

/** This function is executed every time Interrupt 6 is triggered. */
SIGNAL(SIG_INTERRUPT6)  //Execute the following code if an INT6 interrupt has been generated. It is kept short.
{
	bToggle = 1;	//Some push button has been pushed or released. Action needs to be taken.
	buttons = PINC; //Take a snapshot of the input register of Port C where the push buttons are connected.
}

ISR(ADC_vect){
	adc_value = ADCL;		//Load the low byte of the ADC result
	adc_value += (ADCH<<8); //shift the high byte by 8bits to put the high byte in the variable
}
