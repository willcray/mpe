#ifndef RADIO_TRX_HEADER_BOARD_H
#define RADIO_TRX_HEADER_BOARD_H

#define TXMOD BIT5			// using P1.5 for transmit data
#define RXDATA_20 BIT0 	 	//todo check to see which interrupt is being called to determine
#define RXDATA_21 BIT1
							// if we want to use P2.0/P2.1 for receive data
#define CNTRL1 BIT6			// P1.6 for T/R control

// Test Points
#define WHITE_TEST_POINT BIT5 // P1.5 TX-DATA (TXMOD on TR1000) (U8)
#define YELLOW_TEST_POINT BIT0 // P2.0 / 2.1 RX-DATA (RXDATA on TR1000) (U9)
#define BLUE_TEST_POINT BIT2 // P2.2 (U12)
#define ORANGE_TEST_POINT BIT6  // P1.6 T/R (CNTRL1 on TR1000) (U10)
#define GREEN_TEST_POINT BIT5  // P2.5 (U11)
#define PURPLE_TEST_POINT BIT4 // P2.4 (U13)
#define BROWN_TEST_POINT BIT3 // P2.3 (U14)

#define BITS_IN_TRANSMISSION  32 // Needed by both TX and RX

int countOnes(long data);

#endif
