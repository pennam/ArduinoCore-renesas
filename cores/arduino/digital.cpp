#include "Arduino.h"

void pinMode(pin_size_t pin, PinMode mode) {
	switch (mode) {
		case INPUT:
			R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_INPUT);
			break;
		case INPUT_PULLUP:
			R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_INPUT | IOPORT_CFG_PULLUP_ENABLE);
			break;
		case OUTPUT:
			R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_OUTPUT);
			break;
	}
}

void digitalWrite(pin_size_t pin, PinStatus val) {
	R_IOPORT_PinWrite(NULL, g_pin_cfg[pin].pin, val == LOW ? BSP_IO_LEVEL_LOW : BSP_IO_LEVEL_HIGH);
}

PinStatus digitalRead(pin_size_t pin) {
	bsp_io_level_t ret;
	R_IOPORT_PinRead(NULL, g_pin_cfg[pin].pin, &ret);
	return (ret == BSP_IO_LEVEL_LOW ? LOW : HIGH);
}
