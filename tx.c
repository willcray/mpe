/*
 * Ben Browning and Will Cray
 */

// **************************************************************************************
// Includes Section
// ************************************************************************************

#include <msp430.h>
#include "tx.h"

// **************************************************************************************

//This routine manages the actual transmitter and is called every 500uS by a periodic interrupt.
//Comment Well
void Xmit(TransmitterData* TData) {
    enum XmitClockPhase Phase;
//Each 500 uS half bit period constitutes a separate clock "phase" for transmitter purposes.
    if (TData->Transmit_Clock_Phase == Low){
        TData->Transmit_Clock_Phase = High ;
    }
    else TData->Transmit_Clock_Phase = Low ;
    Phase = TData->Transmit_Clock_Phase ;
//Now do state machine
    switch(TData->Transmitter_State){
        case StartBit :
            switch(Phase) {
                case Low :

                break ;
                case High :

                break ;
            }


        break ;

        case NormalXmit :
            switch(Phase) {
                case Low :

                break ;
                case High :

                break ;
            }

        break ;
        case InterWord :
            switch(Phase) {
                case Low :

                break ;
                case High :

                break ;
            }

        break ;
        default :
            TData->Transmitter_State = StartBit ;
//Other intitialization here.....
        break ;

    }
}

//Functions called via an  interrupt
//This is called every 500uS by the timer A0 interrupt function
void txinthandler(void) {
//Do whatever needs to be done on a periodic basis for tx here:
// update clock phase
// interword timeout

}







