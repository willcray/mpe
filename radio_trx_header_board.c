#include "radio_trx_header_board.h"

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
