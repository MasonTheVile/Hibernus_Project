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

//Interrupt and Restoring
#define INT 0xEDD8
#define CHECK 0xEDDC


//FRAM
#define FRAM_START 0xEDF0
#define FRAM_END 0xFF40

// RAM
#define RAM_START 0x1C00
#define RAM_END 0x2000



//Threshold
#define VMIN 2270
#define VMINN 2170

//PC
#define PC 0xEDF0

/*
//newley added
#define FRAM_START 0x4000
#define RAM_START 0x1C00
#define FRAM_END 40400
#define RAM_END 0x2C00
*/



// Function Declarations
void Hibernus(void);
void Hibernate (void);
void Restore(void);
void Save_RAM (void);
void Save_Register (void);
void Restore_Register (void);
void GPIO_configuration(void);
void Clock_configuration(void);
void Set_internal_comparator (void);

unsigned int get_stack_pointer(void);