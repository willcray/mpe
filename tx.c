/*
 * Ben Browning and Will Cray
 */

// **************************************************************************************
// Includes Section
// ************************************************************************************
#include <msp430.h>
#include "tx.h"
#include "radio_trx_header_board.h"

// **************************************************************************************

TransmitterData Xmit1 ;  //This declares an instance of the transmitter data structure.

//This routine manages the actual transmitter and is called every 500uS by a periodic interrupt.
//Comment Well
void Xmit(void) {
	enum XmitClockPhase Phase;
//Each 500 uS half bit period constitutes a separate clock "phase" for transmitter purposes.
	if (Xmit1.Transmit_Clock_Phase == Low) {
		Xmit1.Transmit_Clock_Phase = High;
        P2OUT |= BLUE_TEST_POINT;
	} else
		Xmit1.Transmit_Clock_Phase = Low;
        P2OUT &= ~BLUE_TEST_POINT;
	Phase = Xmit1.Transmit_Clock_Phase;
//Now do state machine
	switch (Xmit1.Transmitter_State) {
	case StartBit:

		switch (Phase) {
		case Low:
            P1OUT |= TXMOD;
			break;
		case High:
            P1OUT &= ~TXMOD;
			Xmit1.Transmitter_State = NormalXmit;
            break;
		}

		break;

	case NormalXmit:
		switch (Phase) {
		case Low:
			if (Xmit1.Transmit_Data >> Xmit1.Bits_Remaining) { // if current bit is high
				P1OUT &= ~TXMOD;	// this is MPEB
			} else {
				P1OUT |= TXMOD;
			}
			break;
		case High:
			if (Xmit1.Transmit_Data >> Xmit1.Bits_Remaining) { // if current bit is high
				P1OUT |= TXMOD;	// this is MPEB
			} else {
				P1OUT &= ~TXMOD;
                Xmit1.Bits_Remaining--;    //decrement the number of bits being transmitted every 1ms
			}
			break;
		}

        if (Xmit1.Bits_Remaining == 0) Xmit1.Transmitter_State = InterWord;

		break;
	case InterWord:
		switch (Phase) {
		case Low:

			break;
		case High:

			break;
		}
        _delay_cycles(INTERWORD_DELAY * 8000); // 50 ms / 8 MHz = 400,000;

		break;
	default:
		InitTXVariables();		// reinitialize TransmissionData variables
		break;

	}
}
void InitTXVariables(void) {
	//Here is an example:
	Xmit1.Bits_Remaining = BITS_IN_TRANSMISSION;
	Xmit1.Transmit_Data_Buffer = 0xAA55AA55;  //
	Xmit1.Transmit_Data = 0xAA55AA55; //This is just sample data, the final application Determines what is to be sent.
	Xmit1.Transmit_Clock_Phase = Low;
	Xmit1.Transmitter_State = StartBit;
	Xmit1.InterwordTimeout = INTERWORD_DELAY;
}

//Functions called via an  interrupt
//This is called every 500uS by the timer A0 interrupt function
// Do whatever needs to be done on a periodic basis for tx here:
void txinthandler(void) {
	Xmit();	// transmit data every 500us
}

