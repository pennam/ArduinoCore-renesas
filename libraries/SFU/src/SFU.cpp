#include "SFU.h"

const unsigned char SFU[0x10000] __attribute__ ((section(".second_stage_ota"), used)) = {
	#include "c33.h"
};