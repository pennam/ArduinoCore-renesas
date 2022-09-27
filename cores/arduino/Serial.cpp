/*
  Serial.cpp - wrapper over mbed RawSerial
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2018-2019 Arduino SA

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#include "Arduino.h"
#include "Serial.h"
#include "IRQManager.h"


#ifdef Serial
#undef Serial
#endif

UART * UART::g_uarts[MAX_UARTS] = {nullptr};

void uart_callback(uart_callback_args_t *p_args)
{
    /* This callback function is not used but it is referenced into 
       FSP configuration so that (for the moment it is necessary to keep it) */
}

/* -------------------------------------------------------------------------- */
void UART::WrapperCallback(uart_callback_args_t *p_args) {
/* -------------------------------------------------------------------------- */  

  uint32_t channel = p_args->channel;
  
  UART *uart_ptr = UART::g_uarts[channel];

  if(uart_ptr == nullptr) {
    return;
  }
  

  switch (p_args->event){
      case UART_EVENT_RX_COMPLETE: // This is called when all the "expected" data are received
      case UART_EVENT_ERR_PARITY:
      case UART_EVENT_ERR_FRAMING:
      case UART_EVENT_ERR_OVERFLOW:
      {
          break;
      }
      case UART_EVENT_TX_COMPLETE:
      {
        if(uart_ptr->get_tx_tail_index() != uart_ptr->get_tx_head_index() ) {
          uart_ptr->inc_tx_tail_index();
          R_SCI_UART_Write (uart_ptr->get_ctrl(), uart_ptr->get_next_ptr_to_tx(), 1);
        }
        else {
          uart_ptr->set_tx_status(TX_STOPPED);
        }
        break;
      }
      case UART_EVENT_RX_CHAR:
      {
        if(uart_ptr->get_rx_head_index() == uart_ptr->get_prev_of_rx_tail()) {
          // drop receiving byte... not enough space in the buffer...
        }
        else {
          uart_ptr->put_in_rx_buffer(p_args->data);
          uart_ptr->inc_rx_head_index();
        }
        break;
      }
      case UART_EVENT_BREAK_DETECT:
      case UART_EVENT_TX_DATA_EMPTY:
      {
          break;
      }
  }

}


/* -------------------------------------------------------------------------- */
UART::UART(uint8_t _pin_tx, uint8_t _pin_rx) : 
  tx_pin(_pin_tx),
  rx_pin(_pin_rx),
  tx_st(TX_STOPPED), 
  init_ok(false) {
/* -------------------------------------------------------------------------- */    
  uart_cfg.txi_irq = FSP_INVALID_VECTOR;
  uart_cfg.tei_irq = FSP_INVALID_VECTOR;
  uart_cfg.rxi_irq = FSP_INVALID_VECTOR;
  uart_cfg.eri_irq = FSP_INVALID_VECTOR;
}




/* -------------------------------------------------------------------------- */
bool UART::setUpUartIrqs(uart_cfg_t &cfg) {
/* -------------------------------------------------------------------------- */  
  bool rv = false;

  if(channel == UART1_CHANNEL) {
    rv = IRQManager::getInstance().addPeripheral(IRQ_SCI_UART,&cfg);
  }
  
  return rv;
} 

/* -------------------------------------------------------------------------- */
size_t UART::write(uint8_t c) {
/* -------------------------------------------------------------------------- */  
  if(init_ok) {
    while(tx_head_index == previous(tx_tail_index, SERIAL_TX_BUFFER_SIZE)) { }
    inc(tx_head_index, SERIAL_TX_BUFFER_SIZE);
    tx_buffer[tx_head_index] = c;
    if(get_tx_status() == TX_STOPPED) {
      set_tx_status(TX_STARTED);
      inc(tx_tail_index, SERIAL_TX_BUFFER_SIZE);
      R_SCI_UART_Write (&uart_ctrl, tx_buffer + tx_tail_index, 1);
    }
    return 1;
  }
  else {
    return 0;
  }
}

/* -------------------------------------------------------------------------- */
UART::operator bool() {
/* -------------------------------------------------------------------------- */  
	return true;
}

/* -------------------------------------------------------------------------- */
bool  UART::cfg_pins(int max_index) {
/* -------------------------------------------------------------------------- */  
  /* verify index are good */
  if(tx_pin < 0 || rx_pin < 0 || tx_pin >= max_index || rx_pin >= max_index) {
    return false;
  }
  /* getting configuration from table */
  const uint16_t *cfg = g_pin_cfg[tx_pin].list;
  uint16_t cfg_tx = getPinCfg(cfg, PIN_CFG_REQ_UART_TX,false);
  cfg = g_pin_cfg[rx_pin].list;
  uint16_t cfg_rx = getPinCfg(cfg, PIN_CFG_REQ_UART_RX,false);
  /* verify configuration are good */
  if(cfg_tx == 0 || cfg_rx == 0 ) {
    return false;
  }
  /* verify channel are the same for both pins */
  if(GET_CHANNEL(cfg_tx) != GET_CHANNEL(cfg_rx) ) {
    return false;
  }
  /* verify channel does not exceed max possible uart channels */
  if(channel >= MAX_UARTS) {
    return false;
  }
  /* setting channel */
  channel = GET_CHANNEL(cfg_tx);
  
  /* actually configuring PIN function */
  ioport_peripheral_t ioport_tx = USE_SCI_EVEN_CFG(cfg_tx) ? IOPORT_PERIPHERAL_SCI0_2_4_6_8 : IOPORT_PERIPHERAL_SCI1_3_5_7_9;
  ioport_peripheral_t ioport_rx = USE_SCI_EVEN_CFG(cfg_rx) ? IOPORT_PERIPHERAL_SCI0_2_4_6_8 : IOPORT_PERIPHERAL_SCI1_3_5_7_9;
  
  R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[tx_pin].pin, (uint32_t) (IOPORT_CFG_PERIPHERAL_PIN | ioport_tx));
  R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[rx_pin].pin, (uint32_t) (IOPORT_CFG_PERIPHERAL_PIN | ioport_rx));

  return true;
}

/* -------------------------------------------------------------------------- */
void UART::begin(unsigned long baudrate, uint16_t config) {
/* -------------------------------------------------------------------------- */  
  init_ok = true;
  int max_index = g_pin_cfg_size / sizeof(g_pin_cfg[0]);

  init_ok &= cfg_pins(max_index);
  
  if(init_ok) {
    UART::g_uarts[channel]        = this;

    uart_baud.semr_baudrate_bits_b.abcse          = 0;
    uart_baud.semr_baudrate_bits_b.abcs           = 0;
    uart_baud.semr_baudrate_bits_b.bgdm           = 1;
    uart_baud.cks                                 = 0;
    uart_baud.brr                                 = 25;
    uart_baud.mddr                                = (uint8_t) 256;
    uart_baud.semr_baudrate_bits_b.brme           = false;

    uart_cfg_extend.clock                         = SCI_UART_CLOCK_INT;
    uart_cfg_extend.rx_edge_start                 = SCI_UART_START_BIT_FALLING_EDGE;
    uart_cfg_extend.noise_cancel                  = SCI_UART_NOISE_CANCELLATION_DISABLE;
    uart_cfg_extend.rx_fifo_trigger               = SCI_UART_RX_FIFO_TRIGGER_MAX;
    uart_cfg_extend.rx_fifo_trigger               = SCI_UART_RX_FIFO_TRIGGER_MAX;
    uart_cfg_extend.p_baud_setting                = &uart_baud;
    uart_cfg_extend.flow_control                  = SCI_UART_FLOW_CONTROL_RTS;
    uart_cfg_extend.flow_control_pin              = (bsp_io_port_pin_t) UINT16_MAX;
    uart_cfg_extend.rs485_setting.enable          = SCI_UART_RS485_DISABLE;
    uart_cfg_extend.rs485_setting.polarity        = SCI_UART_RS485_DE_POLARITY_HIGH;
    uart_cfg_extend.rs485_setting.de_control_pin  = (bsp_io_port_pin_t) UINT16_MAX;
    
    uart_cfg.channel                              = channel; 
    uart_cfg.p_context                            = NULL;
    uart_cfg.p_extend                             = &uart_cfg_extend;
    uart_cfg.p_transfer_tx                        = NULL;
    uart_cfg.p_transfer_rx                        = NULL;
  
    switch(config){
      case SERIAL_8N1:
          uart_cfg.data_bits = UART_DATA_BITS_8;
          uart_cfg.parity = UART_PARITY_OFF;
          uart_cfg.stop_bits = UART_STOP_BITS_1;
          break;
      case SERIAL_8N2:
          uart_cfg.data_bits = UART_DATA_BITS_8;
          uart_cfg.parity = UART_PARITY_OFF;
          uart_cfg.stop_bits = UART_STOP_BITS_2;
          break;
      case SERIAL_8E1:
          uart_cfg.data_bits = UART_DATA_BITS_8;
          uart_cfg.parity = UART_PARITY_EVEN;
          uart_cfg.stop_bits = UART_STOP_BITS_1;
          break;
      case SERIAL_8E2:
          uart_cfg.data_bits = UART_DATA_BITS_8;
          uart_cfg.parity = UART_PARITY_EVEN;
          uart_cfg.stop_bits = UART_STOP_BITS_2;
          break;
      case SERIAL_8O1:
          uart_cfg.data_bits = UART_DATA_BITS_8;
          uart_cfg.parity = UART_PARITY_ODD;
          uart_cfg.stop_bits = UART_STOP_BITS_1;
          break;
      case SERIAL_8O2:
          uart_cfg.data_bits = UART_DATA_BITS_8;
          uart_cfg.parity = UART_PARITY_ODD;
          uart_cfg.stop_bits = UART_STOP_BITS_2;
          break;
    }
    
    uart_cfg.p_callback = UART::WrapperCallback;
  }
  else {
    return;
  }

  init_ok &= setUpUartIrqs(uart_cfg);
  
  fsp_err_t err;
  const bool bit_mod = true;
  const uint32_t err_rate = 5;

  err = R_SCI_UART_BaudCalculate(baudrate, bit_mod, err_rate, &uart_baud);
  err = R_SCI_UART_Open (&uart_ctrl, &uart_cfg);
  if(err != FSP_SUCCESS) while(1);
  err = R_SCI_UART_BaudSet(&uart_ctrl, (void *) &uart_baud);
  if(err != FSP_SUCCESS) while(1);
  err = R_SCI_UART_Read(&uart_ctrl, rx_buffer, 0);
  if(err != FSP_SUCCESS) while(1);
}

/* -------------------------------------------------------------------------- */
void UART::begin(unsigned long baudrate) {
/* -------------------------------------------------------------------------- */  
  begin(baudrate, SERIAL_8N1);
}

/* -------------------------------------------------------------------------- */
void UART::end() {
/* -------------------------------------------------------------------------- */  
  rx_head_index = 0;
  rx_tail_index = 0;
  tx_head_index = -1;
  tx_tail_index = -1;
  R_SCI_UART_Close (&uart_ctrl);
}



void UART::setPins(int tx, int rx, int rts, int cts)
{
  setPins(digitalPinToBspPin(tx), digitalPinToBspPin(rx),
          digitalPinToBspPin(rts), digitalPinToBspPin(cts));
}

void UART::setPins(bsp_io_port_pin_t tx, bsp_io_port_pin_t rx,
                      bsp_io_port_pin_t rts, bsp_io_port_pin_t cts)
{
  pinPeripheral(tx, (uint32_t) IOPORT_CFG_PERIPHERAL_PIN | getPinConfig(tx));
  pinPeripheral(rx, (uint32_t) IOPORT_CFG_PERIPHERAL_PIN | getPinConfig(rx));
  if (rts != (bsp_io_port_pin_t)0) {
    pinPeripheral(rts, (uint32_t) IOPORT_CFG_PERIPHERAL_PIN | getPinConfig(rts));
    pinPeripheral(cts, (uint32_t) IOPORT_CFG_PERIPHERAL_PIN | getPinConfig(cts));
  }
}

/* -------------------------------------------------------------------------- */
int UART::available() {
/* -------------------------------------------------------------------------- */  
  return ((unsigned int)(SERIAL_RX_BUFFER_SIZE + rx_head_index - rx_tail_index)) % SERIAL_RX_BUFFER_SIZE;
}

/* -------------------------------------------------------------------------- */
int UART::peek() {
/* -------------------------------------------------------------------------- */  
  if(rx_head_index == rx_tail_index) {
    return -1;
  }
  else {
    unsigned char c = rx_buffer[rx_tail_index];
    return c;
  }
}

/* -------------------------------------------------------------------------- */
int UART::read() {
/* -------------------------------------------------------------------------- */  
  if(rx_head_index == rx_tail_index) {
    return -1;
  }
  else {
    unsigned char c = rx_buffer[rx_tail_index];
    inc_rx_tail_index();
    return c;
  }
}

/* -------------------------------------------------------------------------- */
void UART::flush() {
/* -------------------------------------------------------------------------- */  
  while (tx_st == TX_STARTED){};
}