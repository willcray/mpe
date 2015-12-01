/*
 * Ben Browning and Will Cray
 */

// **************************************************************************************
// Constant Declarations Section
// ************************************************************************************

//The following defines are for bit width definitions
//The underlying assumptions are"
//The capture timers are updated with a 1uS resolution
//A full bit time is 1 mS nominal
#define VALID_HALF_BIT_MIN    450
#define VALID_HALF_BIT_MAX    550
#define VALID_FULL_BIT_MIN    950
#define VALID_FULL_BIT_MAX   1050
#define MISSING_EDGE_TIMEOUT 1200

#define SIZE_OF_RCV_QUE        4 //Must be a power of 2!

extern int BITS_IN_TRANSMISSION;

//Receiver Definitions and declarations
enum Captured_Edge {Rising,Falling} ;  //these are the 2 types of edges in the received signal
typedef enum Captured_Edge EdgeType ;
typedef struct {
	EdgeType     Edge      ;    // Which type of edge was received
	unsigned int TimeStamp ;    // When we got it.
} Event_Que_Entry ;

typedef struct {
	Event_Que_Entry Events[SIZE_OF_RCV_QUE] ; //What each entry looks like
	unsigned int  QueSize ;                   //Current size of queue
	unsigned int  Get_Index ;                 //Where we get data from
	unsigned int  Put_index ;                 //Where we put new data
} Event_Queue ;

enum Que_Errors {No_Error,Que_Full,Que_Empty} ;
typedef enum Que_Errors QueReturnErrors       ;

//The following typedefs are for the receiver section
typedef enum Rcv_States {Initial_Expect_Rising,Initial_Expect_Falling,MidBit_Expect_Rising,MidBit_Expect_Falling} ReceiverStates ;
typedef struct {
	ReceiverStates  CurrentRcvState      ;   // State for state machine implementation
	unsigned int	RisingEdgeTimeStamp  ;   // Time stamp at leading edge of signal
	unsigned int    FallingEdgeTimeStamp ;   // TIme stamp for falling edge
	unsigned int    PulseWidth           ;   // Difference in time between edges
	unsigned int    MidBitTimeStamp      ;   // Time Stamp of last valid mid-bit transition
	unsigned int    LastEdgeTimeStamp    ;   // When the last edge occured, regardless of polarity
	unsigned long   CurrentRecvdData     ;   // Data that is being shifted in
	unsigned long   LastValidReceived    ;   // Last complete valid word.
	unsigned int    BitsLeftToGet        ;   // Number of bits to go in reception.
}ManchesterReceiver;


typedef enum PulseWidths {Invalid_Width,Valid_HalfBit,Valid_FullBit} PulseWidthStatus ;

// **************************************************************************************
//Global Variables Section
// ************************************************************************************

//Receiver Global Variables
ManchesterReceiver Rcv1 ;
Event_Queue Receiver_Events ;

// ************************************************************************************************

// ********************************************************************************************
// Function prototype declarations Section

//Receiver Queue functions:
//This function places a new event in the event queue structure.
//The rising and falling edge detectors should call this handler with the appropriate info!

//This is called from within the main loop to see if there are any events on the que, i.e. captured edges.
//Note that it disables interrupts to ensure that data is not overwritten by an interrupter
//If no event is pending, it returns a -1, otherwise it returns an index into the queue that corresponds to the oldest
//event.
QueReturnErrors InsertEvent(EdgeType DetectedEdge, unsigned int CapturedTime);
int GetEvent(void);

//This functions tests a current pulse width and determines if it is a valid width
PulseWidthStatus TestWidth(unsigned int CurrentPulse);

void rcv(void) ;

void risingedge(void);

void fallingedge(void);

void rxinthandler(void);


// ************************************************************************************************
