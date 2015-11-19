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
void Xmit(TransmitterData* TData) {
	enum XmitClockPhase Phase;
//Each 500 uS half bit period constitutes a separate clock "phase" for transmitter purposes.
	if (TData->Transmit_Clock_Phase == Low) {
		TData->Transmit_Clock_Phase = High;
        P2OUT |= BLUE_TEST_POINT;
	} else
		TData->Transmit_Clock_Phase = Low;
        P2OUT &= ~BLUE_TEST_POINT;
	Phase = TData->Transmit_Clock_Phase;
//Now do state machine
	switch (TData->Transmitter_State) {
	case StartBit:

		switch (Phase) {
		case Low:
            P1OUT |= TXMOD;
			break;
		case High:
            P1OUT &= ~TXMOD;
			TData->Transmitter_State = NormalXmit;
            break;
		}

		break;

	case NormalXmit:
		switch (Phase) {
		case Low:
			if (TData->Transmit_Data >> TData->Bits_Remaining) { // if current bit is high
				P1OUT &= ~TXMOD;	// this is MPEB
			} else {
				P1OUT |= TXMOD;
			}
			break;
		case High:
			if (TData->Transmit_Data >> TData->Bits_Remaining) { // if current bit is high
				P1OUT |= TXMOD;	// this is MPEB
			} else {
				P1OUT &= ~TXMOD;
                TData->Bits_Remaining--;    //decrement the number of bits being transmitted every 1ms
			}
			break;
		}

		break;
	case InterWord:
		switch (Phase) {
		case Low:

			break;
		case High:

			break;
		}

		break;
	default:
		InitRXVariables();		// reinitialize TransmissionData variables
		break;

	}
}

//Functions called via an  interrupt
//This is called every 500uS by the timer A0 interrupt function
// Do whatever needs to be done on a periodic basis for tx here:
void txinthandler(void) {
	_delay_cycles(INTERWORD_DELAY);	// interword timeout delay
	Xmit(&Xmit1);	// transmit data every 500us
}

