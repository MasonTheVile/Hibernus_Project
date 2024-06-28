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

#include <msp430.h>
#include <stdio.h>
#include <string.h>

#include "hibernation.h"

#pragma SET_DATA_SECTION(".fram_vars")

unsigned long int *FRAM_write_ptr = (unsigned long int *) FRAM_START; //pointer that points the FRAM
unsigned long int *RAM_copy_ptr = (unsigned long int *) RAM_START; //pointer that points the RAM
unsigned long int *FLAG_interrupt = (unsigned long int *) INT; //Flag for Interrupt
unsigned long int *CC_Check = (unsigned long int *) CHECK; //Flag for Restoring

//These pointers and variable are used to set the PC
unsigned long int *FRAM_pc = (unsigned long int *) PC; //pointer for PC
unsigned long int* current_SP;

//This array is used to restore the state
unsigned int Registers[230];

unsigned int *Reg_copy_ptr;
unsigned int counter_check;

int pro;
int t;
int flag;

#pragma SET_DATA_SECTION()

//Voltage input from the External comparator
const unsigned int vol[256]= {
    1966, 1973, 1979, 1986, 1992, 1999, 2006, 2012, 2019, 2025, 2032, 2038, 2045, 2051, 2058, 2065, 2071, 2078, 2084, 2091,
    2097, 2104, 2110, 2117, 2124, 2130, 2137, 2143, 2150, 2156, 2163, 2169, 2176, 2183, 2189, 2196, 2202, 2209, 2215, 2222,
    2228, 2235, 2242, 2248, 2255, 2261, 2268, 2274, 2281, 2287, 2294, 2301, 2307, 2314, 2320, 2327, 2333, 2340, 2346, 2353,
    2360, 2366, 2373, 2379, 2386, 2392, 2399, 2405, 2412, 2419, 2425, 2432, 2438, 2445, 2451, 2458, 2464, 2471, 2478, 2484,
    2491, 2497, 2504, 2510, 2517, 2523, 2530, 2537, 2543, 2550, 2556, 2563, 2569, 2576, 2582, 2589, 2596, 2602, 2609, 2615,
    2622, 2628, 2635, 2641, 2648, 2655, 2661, 2668, 2674, 2681, 2687, 2694, 2700, 2707, 2714, 2720, 2727, 2733, 2740, 2746,
    2753, 2759, 2766, 2773, 2779, 2786, 2792, 2799, 2805, 2812, 2818, 2825, 2832, 2838, 2845, 2851, 2858, 2864, 2871, 2877,
    2884, 2891, 2897, 2904, 2910, 2917, 2923, 2930, 2936, 2943, 2950, 2956, 2963, 2969, 2976, 2982, 2989, 2995, 3002, 3009,
    3015, 3022, 3028, 3035, 3041, 3048, 3054, 3061, 3068, 3074, 3081, 3087, 3094, 3100, 3107, 3113, 3120, 3127, 3133, 3140,
    3146, 3153, 3159, 3166, 3172, 3179, 3186, 3192, 3199, 3205, 3212, 3218, 3225, 3231, 3238, 3245, 3251, 3258, 3264, 3271,
    3277, 3284, 3290, 3297, 3304, 3310, 3317, 3323, 3330, 3336, 3343, 3349, 3356, 3363, 3369, 3376, 3382, 3389, 3395, 3402,
    3408, 3415, 3422, 3428, 3435, 3441, 3448, 3454, 3461, 3467, 3474, 3481, 3487, 3494, 3500, 3507, 3513, 3520, 3526, 3533,
    3540, 3546, 3553, 3559, 3566, 3572, 3579, 3585, 3592, 3599, 3605, 3612, 3618, 3625, 3631, 3638 }; //Yes

//General Purpose registers addresses
const unsigned int gen[230]= {
      0x100, 0x102, 0x104, 0x120, 0x12a, 0x130, 0x140, 0x144, 0x146, 0x150, 0x152, 0x154, 0x156, 0x15c,
      0x160, 0x162, 0x164, 0x166, 0x168, 0x16a, 0x16c, 0x180, 0x182, 0x186, 0x188, 0x18a, 0x18c, 0x18e,
      0x198, 0x19a, 0x19c, 0x19e, 0x1b0, 0x200, 0x202, 0x204, 0x206, 0x20a, 0x20c, 0x20e, 0x216, 0x218,
      0x21a, 0x21c, 0x21e, 0x220, 0x222, 0x224, 0x226, 0x22a, 0x22c, 0x22e, 0x236, 0x238, 0x23a, 0x23c,
      0x23e, 0x320, 0x322, 0x324, 0x326, 0x32a, 0x32c, 0x336, 0x340, 0x342, 0x344, 0x346, 0x350, 0x352,
      0x354, 0x356, 0x360, 0x36e, 0x380, 0x382, 0x384, 0x386, 0x390, 0x392, 0x394, 0x396, 0x3a0, 0x3ae,
      0x3c0, 0x3c2, 0x3c4, 0x3c6, 0x3d0, 0x3d2, 0x3d4, 0x3d6, 0x3e0, 0x3ee, 0x400, 0x402, 0x404, 0x406,
      0x410, 0x412, 0x414, 0x416, 0x420, 0x42e, 0x440, 0x442, 0x444, 0x446, 0x450, 0x452, 0x454, 0x456,
      0x460, 0x46e, 0x4a0, 0x4a2, 0x4a8, 0x4aa, 0x4ac, 0x4ae, 0x4b0, 0x4b2, 0x4b4, 0x4b6, 0x4b8, 0x4ba,
      0x4bc, 0x4be, 0x4c0, 0x4c2, 0x4c4, 0x4c6, 0x4c8, 0x4ca, 0x4cc, 0x4ce, 0x4d0, 0x4d2, 0x4d4, 0x4d6,
      0x4d8, 0x4da, 0x4dc, 0x4de, 0x4e0, 0x4e2, 0x4e4, 0x4e6, 0x4e8, 0x4ea, 0x4ec, 0x500, 0x502, 0x504,
      0x506, 0x508, 0x50e, 0x510, 0x512, 0x516, 0x51a, 0x520, 0x522, 0x526, 0x52a, 0x530, 0x532, 0x536,
      0x53a, 0X5a0, 0x5a2, 0x5a4, 0x5a6, 0x5c0, 0x5c2, 0x5c6, 0x5c8, 0x5ca, 0x5cc, 0x5ce, 0x5d0, 0x5d2,
      0x5da, 0x5dc, 0x5de, 0x5e0, 0x5e2, 0x5e6, 0x5e8, 0x5ea, 0x5ec, 0x5ee, 0x5f0, 0x5f2, 0x5fa, 0x5fc,
      0x5fe, 0x640, 0x642, 0x646, 0x648, 0x64a, 0x64c, 0x64e, 0x654, 0x656, 0x658, 0x65a, 0x65c, 0x65e,
      0x660, 0x66a, 0x66c, 0x66e, 0x700, 0x702, 0x704, 0x706, 0x708, 0x70a, 0x712, 0x71a, 0x71c, 0x71e,
      0x8c0, 0x8c2, 0x8c4, 0x8c6, 0x8cc, 0x8ce}; //Yes

unsigned int i;

//hibernus
void Hibernus(void)
{

    //System_Init
    GPIO_configuration();
    Clock_configuration();

    //The internal comparator input is P1.1. The input must be Vcc/2.
    //A voltage divider with R1, R2=1Mohm can be used to have Vcc/2.

    Set_internal_comparator();
    
    //Restore procedure
    //Configuring the internal comparator interrupt
    CECTL1&= ~CEIES;
    CEINT &= ~(CEIFG + CEIIFG);
    CEINT|=CEIE;
    CECTL1|=CEON;
    __delay_cycles(400);

    *FLAG_interrupt=1;

    __bis_SR_register(LPM4_bits+GIE); // Enter LPM4 with inetrrupts enabled
    __no_operation();             // For debug 
    
    //the first time the system does not need a restore

    if ((*CC_Check!=0) && (*CC_Check!=1))
    {

        *CC_Check=0;

        *FLAG_interrupt=2;
        Set_internal_comparator();

        //Comparator setting for Hibernate
        CECTL1|=CEIES;
        CEINT &= ~(CEIFG + CEIIFG);
        CECTL1 |=CEON;
        CEINT |= CEIE;
        __delay_cycles(400);

        __bis_SR_register(GIE);        // Set interrupt
        __no_operation();

    }
    else
    {
        //If you have to restore a previous state

        //For debugging: Restore
        P1DIR |= BIT3;
        P1OUT |= BIT3;
        P2OUT &= ~BIT6;

        if (flag==100)
        {
            Restore();
        }
    }
    
}

void GPIO_configuration (void)
{

    //GPIO configuration
    P1OUT &= ~(0xFF);
    P2OUT &= ~(0xBF);
    P3OUT &= ~(0xFB);
    P4OUT &= ~(0xFF);
    PJOUT &= ~(0xFF); // Configure the pull-down resistor

    //TRIS REGISTERS
    P1DIR |= 0xFF;
    P2DIR |= 0x3F;
    P3DIR |= 0xFB;
    P4DIR |= 0xFF;
    PJDIR |= 0xFF; // // Direction = output

}


void Set_internal_comparator (void)
{

    // Configure CD0
    P1SEL1 |= BIT2;  //CD0
    P1SEL0 |= BIT2;   //P1.2

    CECTL0|=CEIPEN+CEIPSEL_1; //Channel selected for the V+
    CECTL2|=CERSEL;

    // Reference Voltage Generator
    CECTL2|=CEREFL_1; // Shared Ref. 1.2V from the shared reference ON: Comp. D Reference voltage level 3 : 1.5V /////////////////
    CECTL2|=CERS_2; // Comp. D Reference Source 2 : Shared Ref. CERS_2=10b

    CECTL1&= ~(CESHORT+CEEX+CEMRVS);

    CECTL2|=CEREF0_17; //Vref0=2.27V 
    CECTL2|=CEREF1_17; //Vref1=2.17V
        

    CECTL1|=CEF;
    CECTL1|=CEFDLY_0;

}

void Clock_configuration (void)
{

    //setting core frequency
    CSCTL0_H = 0xA5;                            // Unlock register
    CSCTL1 &= ~(DCORSEL);                       //Set max. DCO setting. 5.33MH in this case
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;              //DCO frequency select register 8MHz
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;          // Selects the ACLK, MCLK AND SMCLK sources register
                                                // Set ACLK = VLO; MCLK = DCO; SMCLK = DCO;
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;          // MCLK, SMCLK and ACLK source divider/prescaler register.
    CSCTL0_H = 0x01;                            // Lock Register
}

void Hibernate (void)
{

    //For Debugging: Hibernate
    P1DIR |= BIT7;
    P1OUT |= BIT7;
    P2OUT &= ~BIT6;

    *CC_Check=0;

    //copy Core registers to FRAM
    asm(" MOVA R1,&0xEDF4");
    asm(" MOVA R2,&0xEDF8");
    asm(" MOVA R3,&0xEDFC");
    asm(" MOVA R4,&0xEE00");
    asm(" MOVA R5,&0xEE04");
    asm(" MOVA R6,&0xEE08");
    asm(" MOVA R7,&0xEE0C");
    asm(" MOVA R8,&0xEE10");
    asm(" MOVA R9,&0xEE14");
    asm(" MOVA R10,&0xEE18");
    asm(" MOVA R11,&0xEE1C");
    asm(" MOVA R12,&0xEE20");
    asm(" MOVA R13,&0xEE24");
    asm(" MOVA R14,&0xEE28");
    asm(" MOVA R15,&0xEE2C");
    
    /*
    asm(" MOVA R1, &0x4004");
    asm(" MOVA R2, &0x4008");
    asm(" MOVA R3, &0x400C");
    asm(" MOVA R4, &0x4010");
    asm(" MOVA R5, &0x4014");
    asm(" MOVA R6, &0x4018");
    asm(" MOVA R7, &0x401C");
    asm(" MOVA R8, &0x4020");
    asm(" MOVA R9, &0x4024");
    asm(" MOVA R10, &0x4028");
    asm(" MOVA R11, &0x402C");
    asm(" MOVA R12, &0x4030");
    asm(" MOVA R13, &0x4034");
    asm(" MOVA R14, &0x4038");
    asm(" MOVA R14, &0x403C");
    */
    
    //PC
    current_SP = (unsigned long int *)get_stack_pointer(); 
    *FRAM_pc = *current_SP;

    // copy all the RAM onto the FRAM
    Save_RAM();

    // copy all the general registers onto the FRAM
    pro=0;
    Save_Register();

    flag=100;

    //For Debugging: Hibernate finished
    P1OUT &= ~BIT7;

    *CC_Check = 1;

}

void Restore (void)
{
    
    // Restore all Register and RAM Memory

    Restore_Register();
    
    // Restore RAM
    FRAM_write_ptr= (unsigned long int *) FRAM_START;

    //Leave the space for the core registers
    for ( i= 0; i<16; i++)
    {
        *FRAM_write_ptr++;
    }

    RAM_copy_ptr= (unsigned long int *) RAM_START;

    while(RAM_copy_ptr < (unsigned long int *) (RAM_END)) 
    {

        *RAM_copy_ptr++ = *FRAM_write_ptr++;

    }

    asm(" MOVA &0xEDF4,R1");
    asm(" MOVA &0xEDF8,R2");
    asm(" MOVA &0xEDFC,R3");
    asm(" MOVA &0xEE00,R4");
    asm(" MOVA &0xEE04,R5");
    asm(" MOVA &0xEE08,R6");
    asm(" MOVA &0xEE0C,R7");
    asm(" MOVA &0xEE10,R8");
    asm(" MOVA &0xEE14,R9");
    asm(" MOVA &0xEE18,R10");
    asm(" MOVA &0xEE1C,R11");
    asm(" MOVA &0xEE20,R12");
    asm(" MOVA &0xEE24,R13");
    asm(" MOVA &0xEE28,R14");
    asm(" MOVA &0xEE2C,R15");
    
    /*
    asm(" MOVA &0x4000,R1");
    asm(" MOVA &0x4004,R2");
    asm(" MOVA &0x4008,R3");
    asm(" MOVA &0x400C,R4");
    asm(" MOVA &0x4010,R5");
    asm(" MOVA &0x4014,R6");
    asm(" MOVA &0x4018,R7");
    asm(" MOVA &0x401C,R8");
    asm(" MOVA &0x4020,R9");
    asm(" MOVA &0x4024,R10");
    asm(" MOVA &0x4028,R11");
    asm(" MOVA &0x402C,R12");
    asm(" MOVA &0x4030,R13");
    asm(" MOVA &0x4034,R14");
    asm(" MOVA &0x4038,R15");
    */
    *current_SP=*FRAM_pc; 

    //Debugging: Restore finished
    P1OUT &= ~BIT3;
    
    *FLAG_interrupt=2;
    Set_internal_comparator();
    
    CECTL1|=CEIES;
    CEINT &= ~(CEIFG + CEIIFG);
    CECTL1 |=CEON;
    CEINT |= CEIE;
    
    __delay_cycles(400); // delay for the reference to settle

    
    *CC_Check=0;
    //pro=1; //issue here
    
    //For Debugging: System active
    P2DIR |= BIT6;
    P2OUT |= BIT6;

    __bis_SR_register(GIE); //inetrrupts enabled
    __no_operation();             // For debug
    
}


void Save_RAM (void)
{

  FRAM_write_ptr = (unsigned long int *) FRAM_START;

  for ( i = 0; i < 16; i++)
  {
        *FRAM_write_ptr++;
  }


    // copy all RAM onto the FRAM
    RAM_copy_ptr = (unsigned long int *) RAM_START;

    while(RAM_copy_ptr <= (unsigned long int *) (RAM_END)) 
    {
        *FRAM_write_ptr++ =*RAM_copy_ptr++;
    }
}


void Save_Register (void)
{

    for(i = 0; i < 230; i++)
    {
        Reg_copy_ptr = (unsigned  int *)gen[i];
        Registers[i] = *Reg_copy_ptr;
    }
}


void Restore_Register (void)
{

    //unlock registers
    MPUCTL0_H = 0xA5;
    PMMCTL0_H =  0xA5;
    FRCTL0_H = 0xA5;
    CSCTL0_H = 0xA5;

    //Restore registers
    for(i=0;i<3;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }


    for(i=4;i<6;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }

    for(i=7;i<13;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }


    for(i=15;i<34;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }

    for(i=36;i<47;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }

    for(i=48;i<169;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }


    for(i=170;i<230;i++)
    {
        Reg_copy_ptr=(unsigned  int *)  gen[i];
        *Reg_copy_ptr=Registers[i];
    }

    MPUCTL0_H = 0x01;
    PMMCTL0_H = 0x01;
    FRCTL0_H = 0x01;
    CSCTL0_H = 0x01;
  
}


#pragma vector=COMP_E_VECTOR
__interrupt void COMP_E_ISR(void)
{

  P2DIR |= BIT0;
  P2OUT |= BIT0;


    CECTL1 &= ~(CEON); //disable comarator
    CEINT &= ~(CEIFG + CEIIFG + CEIE); //clean flags and disable iterrupt

    //Hibernate
    if (*FLAG_interrupt == 2)
    {

        CECTL1 &= ~(CEON); //disable comarator
        CEINT &= ~(CEIFG + CEIIFG + CEIE); //clean flags and disable iterrupt

        Hibernate();

        if(pro==0)
        {
            *FLAG_interrupt=4;
            Set_internal_comparator();

            CECTL2|=CEREF0_31;

            //Configuring the internal comparator interrupt
            CECTL1&= ~CEIES;
            CEINT &= ~(CEIFG + CEIIFG);
            CEINT|=CEIE;
            CECTL1|=CEON;
            __delay_cycles(400);

            P1DIR |= BIT2;
            P1OUT |= BIT2;

            __bis_SR_register(LPM4_bits+GIE); // Enter LPM4 with inetrrupts enabled
            __no_operation();             // For debug

        }

    }

    if (*FLAG_interrupt == 4)
    {
        P1OUT &= ~BIT2;

        *FLAG_interrupt=2;
        Set_internal_comparator();

        CECTL1|=CEIES;
        CEINT &= ~(CEIFG + CEIIFG);
        CECTL1 |=CEON;
        CEINT |= CEIE;
        __delay_cycles(400);

        //For Debugging: System active
        P2DIR |= BIT6;
        P2OUT |= BIT6;


        __bis_SR_register(GIE); // Enter LPM4 with inetrrupts enabled
        __no_operation();             // For debug
    }

    pro = 0;

    __bic_SR_register_on_exit(LPM4_bits); // Exit LPM4

    P2OUT &= ~BIT0;

}

unsigned int get_stack_pointer(void) 
{
    return __get_SP_register();
}