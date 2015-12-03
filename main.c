/*
 * Ben Browning and Will Cray
 */

//This code is a skeleton for a Manchester Phase Encoded Transceiver
//As configured, It may transmit any number of bits up to 32.
//A separate start bit is sent, with a rising edge at mid-bit.
//After this, normal transmission is sent
//This version defines a 1 as a rising edge at mid-bit
//Transmission rate is 1 mS per bit.
//There is an inter-transmission time of 50 mS, then repeat transmission
//A 500uS clock tick is interrupt is required.
// **************************************************************************************
// Includes Section
// ************************************************************************************
#include <msp430.h>
#include "tx.h"
#include "radio_trx_header_board.h"


#define TX_ENABLED      // Enable Transmit feature at compile time
// #define RX_ENABLED    // Enable Receive feature at compile time

#ifdef RX_ENABLED
#include "rx.h"
#endif

#ifdef TX_ENABLED
#include "tx.h"
#endif

// **************************************************************************************
// Constant Defines Section
// ************************************************************************************

// **************************************************************************************
// Function Prototypes Section
// ************************************************************************************

void InitTRXHardware(void); //This initializes all hardware subsystems, timer, ports etc.
//Defines for initializiation of various subsystems
//Clock System Initialization
void BCSplus_initial(void);
//Timer Initial..
void Timer0_A3_initial(void);
void Timer1_A3_initial(void);
//All hardware initializing is done here.

// **************************************************************************************

//Set up globals, including the transmitter structure.
//Comment Well

//Comment Well!
void InitTRXHardware(void) {

//Set up ports here :
#ifdef TX_ENABLED
	P1OUT &= ~TXMOD;		// set TXMOD low
	P1DIR |= TXMOD;		// TXDATA is an output
	P1OUT &= ~CNTRL1;		// send T/R low for transmit
	P1DIR |= CNTRL1;		// control T/R is an output
#endif

#ifdef RX_ENABLED
	P2DIR &= ~RXDATA_20;// RXDATA is an input
	P2DIR &= ~RXDATA_21;// RXDATA is an input

	P1OUT |= CNTRL1;// send T/R high for receive
	P1DIR |= CNTRL1;// control T/R is an output

	// set up timer a3 to get interrupted by P2.0 and P2.1
	P2SEL |= BIT0 + BIT1;
	P2SEL2 &= ~(BIT0 + BIT1);
#endif

	// set up test points
	// P2.2 (U12), P2.5 (U11), P2.4 (U13), P2.3 (U14)
	P2OUT &= ~(BLUE_TEST_POINT + GREEN_TEST_POINT + PURPLE_TEST_POINT
			+ BROWN_TEST_POINT);
	P2DIR |= (BLUE_TEST_POINT + GREEN_TEST_POINT + PURPLE_TEST_POINT
			+ BROWN_TEST_POINT);

// End of port setup/
	BCSplus_initial(); //get clock going - 8 mhz rate
	Timer0_A3_initial();
	Timer1_A3_initial();
}

void main(void) {
//Be sure to stop watchdog timer first!

	WDTCTL = WDTHOLD + WDTPW;

	InitTRXHardware();
#ifdef TX_ENABLED

    long messageToSend = 0x55378008; // can't have 1 in msb
    Xmit1.Message = messageToSend;
    int parityBit = countOnes(messageToSend) % 2;
    Xmit1.Transmit_Data = ((messageToSend << 1) | parityBit);
    Xmit1.Transmit_Data_Buffer = Xmit1.Transmit_Data;

    InitTXVariables();

    // InitTXVariables(0x10000000); // expect signal to look like 0010 0000 0000 0000 0000 0000 0000 0001 = 0x20000001
    // InitTXVariables(0x30000000); // expect signal to look like 0110 0000 0000 0000 0000 0000 0000 0000 = 0x60000000
    // InitTXVariables(0x0EABBEEF); // expect signal to look like 0001 1101 0101 0111 0111 1101 1101 1111 = 0x1D577DDF
    // expect signal to look like 1000 0000 0000 0000 0000 0000 0000 0001 = 0x80000001
#endif
#ifdef RX_ENABLED
	InitRXVariables();
#endif

	//Enable Global Interrupts after all intializing is done.
	_EINT();
	// how is this different from _BIS_SR(GIE) as seen in other labs?

	while (1) { //Main code loop here :
#ifdef RX_ENABLED
	rcv(); //Call the receiver
#endif
    }
}

// Interrupt Handlers
#pragma vector=TIMER0_A0_VECTOR
__interrupt void periodicTimerA0Interrupt(void) {
	/* Capture Compare Register 0 ISR Hook Function Name */
#ifdef TX_ENABLED
	txinthandler();
#endif
#ifdef RX_ENABLED
	rxinthandler();
#endif
	/* No change in operating mode on exit */
}
/*
 *  ======== Timer1_A3 Interrupt Service Routine ========
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void timerCaptureRisingInterrupt(void) {
	/* Capture Compare Register 0 ISR Hook Function Name */
#ifdef RX_ENABLED
	risingedge();
#endif
	/* No change in operating mode on exit */
}

/*
 *  ======== Timer1_A3 Interrupt Service Routine ========
 */
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timerCaptureFallingInterrupt(void) {
	switch (__even_in_range(TA1IV, TA1IV_TAIFG)) // Efficient switch-implementation
	{
	case TA1IV_TACCR1:
		/* Capture Compare Register 1 ISR Hook Function Name */
#ifdef RX_ENABLED
		fallingedge();
#endif
		/* No change in operating mode on exit */
		break;
	case TA1IV_TACCR2:
		break;
	case TA1IV_TAIFG:
		break;
	}
}

/*
 *  ======== BCSplus_init ========
 *  Initialize MSP430 Basic Clock System
 */
void BCSplus_initial(void) {
	/*
	 * Basic Clock System Control 2
	 *
	 * SELM_0 -- DCOCLK
	 * DIVM_0 -- Divide by 1
	 * ~SELS -- DCOCLK
	 * DIVS_0 -- Divide by 1
	 * ~DCOR -- DCO uses internal resistor
	 *
	 * Note: ~<BIT> indicates that <BIT> has value zero
	 */
	BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;

	if (CALBC1_8MHZ != 0xFF) {
		/* Adjust this accordingly to your VCC rise time */
		__delay_cycles(100000);

		// Follow recommended flow. First, clear all DCOx and MODx bits. Then
		// apply new RSELx values. Finally, apply new DCOx and MODx bit values.
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_8MHZ; /* Set DCO to 8MHz */
		DCOCTL = CALDCO_8MHZ;
	}

	/*
	 * Basic Clock System Control 1
	 *
	 * XT2OFF -- Disable XT2CLK
	 * ~XTS -- Low Frequency
	 * DIVA_0 -- Divide by 1
	 *
	 * Note: ~XTS indicates that XTS has value zero
	 */
	BCSCTL1 |= XT2OFF + DIVA_0;

	/*
	 * Basic Clock System Control 3
	 *
	 * XT2S_0 -- 0.4 - 1 MHz
	 * LFXT1S_2 -- If XTS = 0, XT1 = VLOCLK ; If XTS = 1, XT1 = 3 - 16-MHz crystal or resonator
	 * XCAP_1 -- ~6 pF
	 */
	BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1;
}

// *****************************************************************************************

/*
 *  ======== Timer0_A3_init ========
 *  Initialize MSP430 Timer0_A3 timer
 */
void Timer0_A3_initial(void) {
	/*
	 * TA0CCTL0, Capture/Compare Control Register 0
	 *
	 * CM_0 -- No Capture
	 * CCIS_0 -- CCIxA
	 * ~SCS -- Asynchronous Capture
	 * ~SCCI -- Latched capture signal (read)
	 * ~CAP -- Compare mode
	 * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
	 *
	 * Note: ~<BIT> indicates that <BIT> has value zero
	 */
	TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_0 + CCIE;

	/* TA0CCR0, Timer_A Capture/Compare Register 0 */
	TA0CCR0 = 499;

	/*
	 * TA0CTL, Timer_A3 Control Register
	 *
	 * TASSEL_2 -- SMCLK
	 * ID_3 -- Divider - /8
	 * MC_1 -- Up Mode
	 */
	TA0CTL = TASSEL_2 + ID_3 + MC_1;
}

/*
 *  ======== Timer1_A3_init ========
 *  Initialize MSP430 Timer1_A3 timer
 */
void Timer1_A3_initial(void) {
	/*
	 * TA1CCTL0, Capture/Compare Control Register 0
	 *
	 * CM_1 -- Rising Edge
	 * CCIS_0 -- CCIxA
	 * SCS -- Sychronous Capture
	 * ~SCCI -- Latched capture signal (read)
	 * CAP -- Capture mode
	 * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
	 *
	 * Note: ~SCCI indicates that SCCI has value zero
	 */
	TA1CCTL0 = CM_1 + CCIS_0 + SCS + CAP + OUTMOD_0 + CCIE;

	/*
	 * TA1CCTL1, Capture/Compare Control Register 1
	 *
	 * CM_2 -- Falling Edge
	 * CCIS_0 -- CCIxA
	 * SCS -- Sychronous Capture
	 * ~SCCI -- Latched capture signal (read)
	 * CAP -- Capture mode
	 * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
	 *
	 * Note: ~SCCI indicates that SCCI has value zero
	 */
	TA1CCTL1 = CM_2 + CCIS_0 + SCS + CAP + OUTMOD_0 + CCIE;

	/*
	 * TA1CCTL2, Capture/Compare Control Register 2
	 *
	 * CM_2 -- Falling Edge
	 * CCIS_0 -- CCIxA
	 * SCS -- Sychronous Capture
	 * ~SCCI -- Latched capture signal (read)
	 * CAP -- Capture mode
	 * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
	 *
	 * Note: ~SCCI indicates that SCCI has value zero
	 */
	TA1CCTL2 = CM_2 + CCIS_0 + SCS + CAP + OUTMOD_0;

	/*
	 * TA1CTL, Timer_A3 Control Register
	 *
	 * TASSEL_2 -- SMCLK
	 * ID_3 -- Divider - /8
	 * MC_2 -- Continuous Mode
	 */
	TA1CTL = TASSEL_2 + ID_3 + MC_2;
}
