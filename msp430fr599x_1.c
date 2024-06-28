/*
Hibernus for MSP430FR5739:

Hibernus: Software-based approach to intelligently hibernate and restore the system's state
in response to a power failure. This software exploits an external comparator.

Citation: If you are using this code for your research please cite:

[1]	D. Balsamo, A. S. Weddell, G. V. Merrett, B. M. Al-Hashimi, D. Brunelli and L. Benini,
	"Hibernus: Sustaining Computation During Intermittent Supply for Energy-Harvesting Systems,"
	in IEEE Embedded Systems Letters, vol. 7, no. 1, pp. 15-18, March 2015.

[2]	D. Balsamo; A. Weddell; A. Das; A. Arreola; D. Brunelli; B. Al-Hashimi; G. Merrett; L. Benini,
	"Hibernus++: A Self-Calibrating and Adaptive System for Transiently-Powered Embedded Devices,"
	in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems , vol.PP, no.99, pp.1-1

This research has been supported by the University of Southampton and the University of Bologna.
Copyright 2016, Domenico Balsamo, All rights reserved.
25/05/2016
*/


//Hibernus uses the internal comparator and the input is P1.1. The input must be Vcc/2.
//A voltage divider with R1, R2=1 Mohm can be used to have Vcc/2.

//The microcontroller input has to be connected to the output of the energy harvester
//through a diode, which prevents back-flow of charge to the harvester. See the reference [1] for the schematic.

#include <msp430.h> 
#include "hibernation.h"

#define LED1 BIT7  // P3.7
#define LED2 BIT6  // P3.6
#define LED3 BIT5  // P3.5
#define LED4 BIT4  // P3.4

/*
 * main.c
 */


//Main function for binary counter
int main(void) 
{
	//For Debugging: System active
	P2DIR |= BIT6;
	P2OUT |= BIT6;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Clear the LOCKLPM5 bit to enable GPIO
    PM5CTL0 &= ~LOCKLPM5;

    Hibernus();

    P3DIR |= (LED1 | LED2 | LED3 | LED4 );

    //Main code goes here
    //Example main code: Binary counter from 0-255

    //unsigned int i = 0;

    while(1)
    {
        /*
    	//Main function for binary counter
		for(i = 0; i < 256; i++)
        {
            //clear bits
            PJOUT &= ~(BIT0);
            PJOUT &= ~(BIT1);
            PJOUT &= ~(BIT2);
            PJOUT &= ~(BIT3);
            P3OUT &= ~(BIT4);
            P3OUT &= ~(BIT5);
            P3OUT &= ~(BIT6);
            P3OUT &= ~(BIT7);

            //setting count
            PJOUT |= (BIT0 & (i & 0x0001));
            PJOUT |= (BIT1 & (i & 0x0002));
            PJOUT |= (BIT2 & (i & 0x0004));
            PJOUT |= (BIT3 & (i  & 0x0008));
            P3OUT |= (BIT4 & (i & 0x0010));
            P3OUT |= (BIT5 & (i & 0x0020));
            P3OUT |= (BIT6 & (i & 0x0040));
            P3OUT |= (BIT7 & (i & 0x0080));

            __delay_cycles(1000000);
		}
        */
        
        //P3OUT &= ~(BIT4);
        //P3OUT &= ~(BIT5);
        //P3OUT &= ~(BIT6);
        //P3OUT &= ~(BIT7);

        P3OUT |= LED1;  // Set P3.7 high
       __delay_cycles(10000000);
        P3OUT &= ~LED1;  // Set P3.7 low
       __delay_cycles(10000000);
        
        // Blink LED on P3.6
        
        P3OUT |= LED2;  // Set P3.6 high
        __delay_cycles(10000000);
        P3OUT &= ~LED2;  // Set P3.6 low
       __delay_cycles(10000000);

        // Blink LED on P3.5
        P3OUT |= LED3;  // Set P3.5 high
        __delay_cycles(10000000);
        P3OUT &= ~LED3;  // Set P3.5 low
       __delay_cycles(10000000);
        
        // Blink LED on P3.4
        P3OUT |= LED4;  // Set P3.4 high
        __delay_cycles(10000000);
        P3OUT &= ~LED4;  // Set P3.4 low
        __delay_cycles(10000000);

    }
}


/*
#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    // Configure GPIO
    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output direction

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    while(1)
    {
        P1OUT ^= BIT0;                      // Toggle LED
        __delay_cycles(100000);
    }
}
*/
