#include "Serial.h"

#if SERIAL_HOWMANY > 0
UART _UART1_(UART1_TX_PIN, UART1_RX_PIN);
#endif

#if SERIAL_HOWMANY > 1
UART _UART2_(UART2_TX_PIN, UART2_RX_PIN);
#endif

#if SERIAL_HOWMANY > 2
UART _UART3_(UART3_TX_PIN, UART3_RX_PIN);
#endif

#if SERIAL_HOWMANY > 3
UART _UART4_(UART4_TX_PIN, UART4_RX_PIN);
#endif

#if SERIAL_HOWMANY > 4
UART _UART5_(UART5_TX_PIN, UART5_RX_PIN);
#endif