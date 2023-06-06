#include "SFU.h"
#include "CodeFlashBlockDevice.h"

const unsigned char SFU[0x20000] __attribute__ ((section(".second_stage_ota"), used)) = {
	#include "c33.h"
};

CodeFlashBlockDevice& bd = CodeFlashBlockDevice::getInstance();
