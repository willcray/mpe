#ifndef TX_H
#define TX_H

#include <msp430.h>
#include "radio_trx_header_board.h"

/*
 * Ben Browning and Will Cray
 */

// **************************************************************************************
// Constant Defines Section
// ************************************************************************************
#define SET_TX_DIR      P1DIR |= BIT5;    
#define TX_HIGH         P1OUT |= BIT5;
#define TX_LOW          P1OUT &= ~BIT5;

// This is effectively how long to wait between transmissions
// #define INTERWORD_DELAY       400000    //This is in units of cycles running at 8MHz
#define INTERWORD_DELAY 100 // 100 interrupts, each 500us each

// **************************************************************************************
// Type Declarations Section
// ************************************************************************************

enum Transmit_States {StartBit,NormalXmit,InterWord} ;

enum XmitClockPhase  {High,Low} ;

typedef struct {
    unsigned long        Transmit_Data         ;  //This is the data to actually be transmitted
    unsigned long        Transmit_Data_Buffer  ;  //This should be reloaded any time we wish to change what is transmitted.
    unsigned int         Bits_Remaining        ;  //This is the number of bits remaining in the current transmission
    enum XmitClockPhase  Transmit_Clock_Phase  ;  //This gets updated once every 1/2 bit period (500 uS in this case.)
    unsigned int         InterwordTimeout      ;  //This represents a "dead" period between successive transmissions
    enum Transmit_States Transmitter_State     ;  //This is the current state machine state for the transmitter
} TransmitterData ;


// **************************************************************************************
// Global Variables Section
// ************************************************************************************

extern TransmitterData Xmit1 ;  //This declares an instance of the transmitter data structure.

// **************************************************************************************
// Function Prototypes Section
// ************************************************************************************
void Xmit(void) ; //This routine is called every 500 uS by an interrupt handler.
void InitTXVariables(long data); //All Global Variables are set up by this
void txinthandler(void);
int countOnes(long data);

// ************************************************************************************************

#endif