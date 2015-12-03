/*
 * Ben Browning and Will Cray
 */

// **************************************************************************************
// Includes Section
// ************************************************************************************
#include "tx.h"

// **************************************************************************************

TransmitterData Xmit1 ;  //This declares an instance of the transmitter data structure.
int k = 0;

//This routine manages the actual transmitter and is called every 500uS by a periodic interrupt.
//Comment Well
void Xmit(void) {
//Each 500 uS half bit period constitutes a separate clock "phase" for transmitter purposes.
	if (Xmit1.Transmit_Clock_Phase == Low) {
		Xmit1.Transmit_Clock_Phase = High;
        P2OUT |= BLUE_TEST_POINT;
	} else
    {
		Xmit1.Transmit_Clock_Phase = Low;
        P2OUT &= ~BLUE_TEST_POINT;
    }
//Now do state machine
	switch (Xmit1.Transmitter_State) {
		case StartBit:
			P2OUT ^= GREEN_TEST_POINT;
			switch (Xmit1.Transmit_Clock_Phase) {
				case Low:
				break;
				case High:
					// clock just rose, and we're trying to send a 1, therefore set TXMOD high
					if(k == 0){
						P1OUT |= TXMOD;	// this is MPEB
						k++;
					}else{
						P1OUT &= ~TXMOD;
						Xmit1.Transmitter_State = NormalXmit;
						k = 0;
					}
				break;
			}
		break;

		case NormalXmit:
			switch (Xmit1.Transmit_Clock_Phase) {
			case Low:
				Xmit1.Bits_Remaining--;    //decrement the number of bits being transmitted every 1ms

				// mid bit transition stuff if necessary
				if ((Xmit1.Transmit_Data >> Xmit1.Bits_Remaining) & 0x1) { // if current bit is high
					// clock just fell, and we're trying to send a 1, therefore set TXMOD low so it can go high on clock rise
					P1OUT &= ~TXMOD;	// this is MPEB
				} else {
					// clock just fell, and we're trying to send a 0, therefore set TXMOD high so it can go low on clock rise
					P1OUT |= TXMOD;
				}
			break;
			
			case High:
				if ((Xmit1.Transmit_Data >> Xmit1.Bits_Remaining) & 0x1) { // if current bit is high
					// clock just rose, and we're trying to send a 1, therefore set TXMOD high
					P1OUT |= TXMOD;	// this is MPEB
				} else {
					// clock just rose, and we're trying to send a 0, therefore set TXMOD low
					P1OUT &= ~TXMOD;
				}
			break;
			}

	        if (Xmit1.Bits_Remaining == -1){
	        	Xmit1.Transmitter_State = InterWord;
	        }

		break;

		case InterWord:
			P1OUT &= ~TXMOD;	// send the data low during interword
			Xmit1.InterwordTimeout--;
			if (Xmit1.InterwordTimeout == 0){
				InitTXVariables(Xmit1.Transmit_Data);      // reinitialize TransmissionData variables
			}

		break;

		default:
	        Xmit1.Transmitter_State = StartBit;
	        // other initialization here
		break;

		
	}
}
void InitTXVariables(long data) {
	if (data >> 31 & 0x1) {
		// BAD SEND, MSB is 1 aka the long won't fit with a parity bit at the end
	} else {
		int parityBit = countOnes(data) % 2;

		Xmit1.Transmit_Data = (data << 1) | parityBit;
		Xmit1.Transmit_Data_Buffer = Xmit1.Transmit_Data;
	}

	//Here is an example:
	Xmit1.Bits_Remaining = BITS_IN_TRANSMISSION;
	Xmit1.Transmit_Clock_Phase = Low;
	Xmit1.Transmitter_State = StartBit;
	Xmit1.InterwordTimeout = INTERWORD_DELAY;
}

int countOnes(long data) {
	int count = 0;
	while (data)
	{
		if (data & 0x1)
			count++;
		data >>= 1;
	}
	return count;
}

//Functions called via an  interrupt
//This is called every 500uS by the timer A0 interrupt function
// Do whatever needs to be done on a periodic basis for tx here:
void txinthandler(void) {
	Xmit();	// transmit data every 500us
}

