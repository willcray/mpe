/*
 * Ben Browning and Will Cray
 */
/*
 * rx.c
*/
#include "rx.h"

void rxinthandler(void)
{
    // If you want something to happen for the receiver every 500us, you can do it here.

}


//This should be called Frequently from the main loop.
void rcv(void){
    int index ;
    PulseWidthStatus PWidth ;
    unsigned int CurrentTime ;
    Event_Que_Entry Current_Event ;
    CurrentTime = TA1R ; // Get the approximate current timestamp.
    if ((CurrentTime - Rcv1.LastEdgeTimeStamp) > MISSING_EDGE_TIMEOUT ){//Here we have had no transmissions in a while
        Rcv1.CurrentRcvState = Initial_Expect_Rising ;
    }
    index = GetEvent() ;
    if (index != -1 ) { //Here we have an edge to deal with, -1 indicates no event in queue
        Current_Event.Edge = Receiver_Events.Events[(unsigned int)index].Edge ;
        Current_Event.TimeStamp = Receiver_Events.Events[(unsigned int)index].TimeStamp ;
        //Now insert receiver state machine here
        Rcv1.LastEdgeTimeStamp = Current_Event.TimeStamp ; //This marks the last time that we got any time stamp at all
        switch (Rcv1.CurrentRcvState){
            // Ignore transitions until we get a rising one.
            case Initial_Expect_Rising :
                if (Current_Event.Edge == Rising) { //Leading edge of initial lead-in bit
                    Rcv1.CurrentRcvState = Initial_Expect_Falling ;
                    Rcv1.RisingEdgeTimeStamp = Current_Event.TimeStamp ;
                }
            break ;
            case Initial_Expect_Falling :
                if (Current_Event.Edge == Rising){
                    Rcv1.CurrentRcvState = Initial_Expect_Rising ; //Out of sequence start over
                }
                else {
                    Rcv1.FallingEdgeTimeStamp = Current_Event.TimeStamp ;  //Figure out when it happens
                    Rcv1.PulseWidth = Rcv1.FallingEdgeTimeStamp - Rcv1.RisingEdgeTimeStamp ; // And Test validity
                    PWidth = TestWidth(Rcv1.PulseWidth) ;
                    if (PWidth == Valid_FullBit) { //Here we have a valid full initial bit
                        Rcv1.CurrentRecvdData = 0 ; // Start all over for receiver
                        Rcv1.MidBitTimeStamp = Rcv1.FallingEdgeTimeStamp ; // By definition at mid bit....
                        Rcv1.BitsLeftToGet = BITS_IN_TRANSMISSION ;
                        Rcv1.CurrentRcvState = MidBit_Expect_Rising ; //Next bit is start of "real" data
                    }
                    else Rcv1.CurrentRcvState = Initial_Expect_Rising ; //Likely a noise pulse, start over
                }
            break ;
            case MidBit_Expect_Falling :
                if (Current_Event.Edge == Rising) { //Out of sequence - start over
                    Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                }
                else {
                    Rcv1.FallingEdgeTimeStamp = Current_Event.TimeStamp ;
                    Rcv1.PulseWidth = Rcv1.FallingEdgeTimeStamp - Rcv1.MidBitTimeStamp ; // Get width relative to last mid-bit
                    PWidth = TestWidth(Rcv1.PulseWidth) ;
                    if (PWidth == Valid_HalfBit) { //Here we have a half-bit, phasing transition
                        Rcv1.CurrentRcvState = MidBit_Expect_Rising ; // Got to expect a rising edge at mid-bit
                    }
                    else {
                        if (PWidth == Valid_FullBit) {    // Rising Edge at mid-bit , clock in a 1
                            Rcv1.CurrentRecvdData <<= 1 ; // Room for new bit
                            --Rcv1.BitsLeftToGet ;
                            if (Rcv1.BitsLeftToGet == 0){ //All done Start over
                                Rcv1.LastValidReceived = Rcv1.CurrentRecvdData ; //Buffer up last received value
                                Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                            }
                            else {
                                Rcv1.MidBitTimeStamp = Rcv1.FallingEdgeTimeStamp ; //New mark for mid-bit
                                Rcv1.CurrentRcvState = MidBit_Expect_Rising    ; //And Expect a rising edge
                            }
                        }
                        else{
                            Rcv1.CurrentRcvState = Initial_Expect_Rising ; // Bad pulse width
                        }
                    }
                }
            break ;
            //We arrived here from a valid mid-bit transition previously
            case MidBit_Expect_Rising :
                if (Current_Event.Edge == Falling) { //Out of sequence - start over
                    Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                }
                else {
                    Rcv1.RisingEdgeTimeStamp = Current_Event.TimeStamp ;
                    Rcv1.PulseWidth = Rcv1.RisingEdgeTimeStamp - Rcv1.MidBitTimeStamp ; // Get width relative to last mid-bit
                    PWidth = TestWidth(Rcv1.PulseWidth) ;
                    if (PWidth == Valid_HalfBit) { //Here we have a half-bit, phasing transition
                        Rcv1.CurrentRcvState = MidBit_Expect_Falling ; // Got to expect a falling edge at mid-bit
                    }
                    else {
                        if (PWidth == Valid_FullBit) {    // Rising Edge at mid-bit , clock in a 1
                            Rcv1.CurrentRecvdData <<= 1 ; // Room for new bit
                            Rcv1.CurrentRecvdData |= 0x01 ;
                            --Rcv1.BitsLeftToGet ;
                            if (Rcv1.BitsLeftToGet == 0){ //All done Start over
                                Rcv1.LastValidReceived = Rcv1.CurrentRecvdData ; //Buffer up last received value
                                Rcv1.CurrentRcvState = Initial_Expect_Rising ;
                            }
                            else {
                                Rcv1.MidBitTimeStamp = Rcv1.RisingEdgeTimeStamp ; //New mark for mid-bit
                                Rcv1.CurrentRcvState = MidBit_Expect_Falling    ; //And Expect a falling edge
                            }
                        }
                        else{
                            Rcv1.CurrentRcvState = Initial_Expect_Rising ; // Bad pulse width
                        }
                    }
                }
            break ;
            default:
                Rcv1.CurrentRcvState = Initial_Expect_Rising ;
            break ;
        }
    }

}

//This function places a new event in the event queue structure.
//The rising and falling edge detectors should call this handler with the appropriate info!
QueReturnErrors InsertEvent(EdgeType DetectedEdge, unsigned int CapturedTime){
    QueReturnErrors rval ;
    unsigned int putindex ;
    rval = No_Error ;
    if (Receiver_Events.QueSize == 4) {//Here Que is already full
        rval = Que_Full ;
    }
    else { //Here we can insert a new event
        ++Receiver_Events.QueSize ;
        putindex = Receiver_Events.Put_index ;
        Receiver_Events.Events[putindex].Edge = DetectedEdge ;
        Receiver_Events.Events[putindex].TimeStamp = CapturedTime ;
        ++putindex ;
        putindex &= SIZE_OF_RCV_QUE-1 ; //Note the constant must always be a power of 2!
        Receiver_Events.Put_index = putindex ;
    }
    return rval ;
}

//This is called from within the main loop to see if there are any events on the que, i.e. captured edges.
//Note that it disables interrupts to ensure that data is not overwritten by an interrupter
//if the return value is negative, then the queue is empty, else it returns an index to the oldest event in the
//queue
int GetEvent(void){

    int rval ;
    unsigned int getindex ;
    rval = -1 ;
    asm("  PUSH.B SR") ;

    _DINT() ;
    if (Receiver_Events.QueSize == 0) { //Nothing to be had!
        rval = -1 ;
    }
    else {
        getindex = Receiver_Events.Get_Index ;
        rval = (int)(getindex) ;
        ++getindex ;
        getindex &= SIZE_OF_RCV_QUE-1 ; //Note the constant must always be a power of 2!
        Receiver_Events.Get_Index = getindex ;
        --Receiver_Events.QueSize ;
    }
    asm("  POP.B SR");
    return rval ;
}

//This functions tests a current pulse width and determines if it is a valid width
PulseWidthStatus TestWidth(unsigned int CurrentPulse){
    PulseWidthStatus rval ;
    rval = Invalid_Width ;
    if ((CurrentPulse >= VALID_HALF_BIT_MIN) && (CurrentPulse <= VALID_HALF_BIT_MAX)) {
        rval = Valid_HalfBit ;
    }
    else {
        if ((CurrentPulse >= VALID_FULL_BIT_MIN) && (CurrentPulse <= VALID_FULL_BIT_MAX)){
            rval = Valid_FullBit ;
        }
    }
    return rval ;
}


//This called by the capture routine on the rising edge of the input signal
void risingedge(void) {
    InsertEvent(Rising, TA1CCR0) ;    //Insert this event into event Queue
}

void fallingedge(void){
    InsertEvent(Falling, TA1CCR1) ; //Insert this event into event Queue.
}

