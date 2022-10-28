/*
 * Copyright (c) 2022 by Alexander Entinger <a.entinger@arduino.cc>
 * CAN library for Arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "CAN.h"

#include <IRQManager.h>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

using namespace arduino;

/**************************************************************************************
 * EXTERN GLOBAL CONSTANTS
 **************************************************************************************/

extern const PinMuxCfg_t g_pin_cfg[];
extern const size_t g_pin_cfg_size;

/**************************************************************************************
 * PROTOTYPE DEFINITIONS
 **************************************************************************************/

extern "C" void can_callback(can_callback_args_t *p_args);

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

ArduinoCAN::ArduinoCAN(int const can_tx_pin, int const can_rx_pin)
: _can_tx_pin{can_tx_pin}
, _can_rx_pin{can_rx_pin}
, _can_mtu_size{CanMtuSize::Classic}
, _open{nullptr}
, _close{nullptr}
, _write{nullptr}
, _read{nullptr}
, _info_get{nullptr}
, _mode_transition{nullptr}
, tx_complete{false}
, rx_complete{false}
, err_status{false}
, _can_bit_timing_cfg
{
  /* Actual bitrate: 250000 Hz. Actual Bit Time Ratio: 75 %. */
  .baud_rate_prescaler = 1 + 3 /* Division value of baud rate prescaler */,
  .time_segment_1 = 11,
  .time_segment_2 = 4,
  .synchronization_jump_width = 4
}
, _can_mailbox_mask
{
  0x1FFFFFFF,
  0x1FFFFFFF,
  0x1FFFFFFF,
  0x1FFFFFFF,
  0x1FFFFFFF,
  0x1FFFFFFF,
  0x1FFFFFFF,
  0x1FFFFFFF
}
, _can_mailbox
{
  { .mailbox_id =  0, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_TRANSMIT},
  { .mailbox_id =  1, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  2, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  3, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  4, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  5, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  6, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  7, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  8, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id =  9, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 10, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 11, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 12, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 13, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 14, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 15, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 16, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 17, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 18, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 19, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 20, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 21, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 22, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 23, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 24, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 25, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 26, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 27, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 28, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 29, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 30, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE },
  { .mailbox_id = 31, .id_mode = CAN_ID_MODE_EXTENDED, .frame_type = CAN_FRAME_TYPE_DATA, .mailbox_type = CAN_MAILBOX_RECEIVE }
}
, _can_fifo_int_cfg
{
  .fifo_int_mode = static_cast<e_can_fifo_interrupt_mode>(CAN_FIFO_INTERRUPT_MODE_RX_EVERY_FRAME | CAN_FIFO_INTERRUPT_MODE_TX_EVERY_FRAME),
  .tx_fifo_irq   = FSP_INVALID_VECTOR,
  .rx_fifo_irq   = FSP_INVALID_VECTOR,
}
, _can_rx_fifo_cfg
{
  .rx_fifo_mask1 = 0x1FFFFFFF,
  .rx_fifo_mask2 = 0x1FFFFFFF,
  .rx_fifo_id1 =
  {
      .mailbox_id              =  0,
      .id_mode                 =  CAN_ID_MODE_EXTENDED,
      .frame_type              =  CAN_FRAME_TYPE_DATA,
      .mailbox_type            =  CAN_MAILBOX_RECEIVE
  },
  .rx_fifo_id2 =
  {
      .mailbox_id              =  1,
      .id_mode                 =  CAN_ID_MODE_EXTENDED,
      .frame_type              =  CAN_FRAME_TYPE_DATA,
      .mailbox_type            =  CAN_MAILBOX_RECEIVE
  }
}
, _can_extended_cfg
{
  .clock_source   = CAN_CLOCK_SOURCE_CANMCLK,
  .p_mailbox_mask = _can_mailbox_mask,
  .p_mailbox      = _can_mailbox,
  .global_id_mode = CAN_GLOBAL_ID_MODE_EXTENDED,
  .mailbox_count  = CAN_NUM_OF_MAILBOXES,
  .message_mode   = CAN_MESSAGE_MODE_OVERWRITE,
  .p_fifo_int_cfg = &_can_fifo_int_cfg,
  .p_rx_fifo_cfg  = &_can_rx_fifo_cfg,
}
, _can_cfg
{
  .channel        = 0,
  .p_bit_timing   = &_can_bit_timing_cfg,
  .p_callback     = can_callback,
  .p_context      = nullptr,
  .p_extend       = &_can_extended_cfg,
  .ipl            = (12),
  .error_irq      = FSP_INVALID_VECTOR,
  .rx_irq         = FSP_INVALID_VECTOR,
  .tx_irq         = FSP_INVALID_VECTOR,
}
, _time_out{500}
, _rx_info
{
  []()
  {
    can_info_t rx_info_init;

    rx_info_init.error_code = 0;
    rx_info_init.error_count_receive = 0;
    rx_info_init.error_count_transmit = 0;
    rx_info_init.rx_fifo_status = 0;
    rx_info_init.rx_mb_status = 0;
    rx_info_init.status = 0;

    return rx_info_init;
  } ()
}
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

bool ArduinoCAN::begin(CanMtuSize const can_mtu_size)
{
  bool init_ok = true;

  /* Configure the pins for CAN.
   */
  int const max_index = g_pin_cfg_size / sizeof(g_pin_cfg[0]);
  init_ok &= cfg_pins(max_index, _can_tx_pin, _can_rx_pin);

  /* Determine the right function pointers depending
   * on whether we have a CAN FD capable module or not.
   */
  if (can_mtu_size == CanMtuSize::Classic)
  {
    _open            = R_CAN_Open;
    _close           = R_CAN_Close;
    _write           = R_CAN_Write;
    _read            = R_CAN_Read;
    _info_get        = R_CAN_InfoGet;
    _mode_transition = R_CAN_ModeTransition;

    _can_mtu_size = CanMtuSize::Classic;
  }
#if IS_CAN_FD
  else
  {
    _open            = R_CANFD_Open;
    _close           = R_CANFD_Close;
    _write           = R_CANFD_Write;
    _read            = R_CANFD_Read;
    _info_get        = R_CANFD_InfoGet;
    _mode_transition = R_CANFD_ModeTransition;

    _can_mtu_size = CanMtuSize::FD;
  }
#endif
  else {
    init_ok = false;
  }

  /* Perform a sanity check if valid function pointers could
   * have been assigned.
   */
  if (!_open || !_close || !_write || !_read || !_info_get) {
    init_ok = false;
  }

  /* Configure the interrupts.
   */
  CanIrqReq_t irq_req
  {
    .ctrl = &_can_ctrl,
    .cfg = &_can_cfg,
  };
  init_ok &= IRQManager::getInstance().addPeripheral(IRQ_CAN, &irq_req);

//  pinMode(CAN_STDBY, OUTPUT);
//  digitalWrite(CAN_STDBY, LOW);

  if (_open(&_can_ctrl, &_can_cfg) != FSP_SUCCESS)
    init_ok = false;

  return init_ok;
}

void ArduinoCAN::end()
{
  _close(&_can_ctrl);
}

int ArduinoCAN::enableInternalLoopback()
{
#if IS_CAN_FD
  can_operation_mode_t const mode = CAN_OPERATION_MODE_GLOBAL_OPERATION;
#else
  can_operation_mode_t const mode = CAN_OPERATION_MODE_NORMAL;
#endif

  if(fsp_err_t const rc = _mode_transition(&_can_ctrl, mode, CAN_TEST_MODE_LOOPBACK_INTERNAL); rc != FSP_SUCCESS)
    return -rc;

  return 1;
}

int ArduinoCAN::disableInternalLoopback()
{
#if IS_CAN_FD
  can_operation_mode_t const mode = CAN_OPERATION_MODE_GLOBAL_OPERATION;
#else
  can_operation_mode_t const mode = CAN_OPERATION_MODE_NORMAL;
#endif

  if(fsp_err_t const rc = _mode_transition(&_can_ctrl, mode, CAN_TEST_MODE_DISABLED); rc != FSP_SUCCESS)
    return -rc;

  return 1;
}

int ArduinoCAN::write(CanMsg const & msg)
{
  can_frame_t can_msg = {
    /* id               = */ msg.id,
    /* id_mode          = */ CAN_ID_MODE_EXTENDED,
    /* type             = */ CAN_FRAME_TYPE_DATA,
    /* data_length_code = */ min(msg.data_length, CAN_DATA_BUFFER_LENGTH),
    /* options          = */ 0
  };

  memcpy(can_msg.data, msg.data, can_msg.data_length_code);

  if(fsp_err_t const rc = _write(&_can_ctrl, 0, &can_msg); rc != FSP_SUCCESS)
    return -rc;

  return 1;
}

uint8_t ArduinoCAN::read(CanMsg & msg)
{
  uint32_t rx_status = 0;
  while(!rx_status && (_time_out--)) {
    // Get the status information for CAN transmission
    if (_info_get(&_can_ctrl, &_rx_info) != FSP_SUCCESS) {
    _time_out = 500;
      return 0;
    }
    rx_status = _rx_info.rx_mb_status;
  }
  _time_out = 500;

  if (rx_status) {
    /* Read the input frame received */
    can_frame_t can_msg;
    if (_read(&_can_ctrl, 0, &can_msg) != FSP_SUCCESS) {
      return 0;
    }
    msg.id = can_msg.id;
    msg.data_length = can_msg.data_length_code;
    memcpy((uint8_t*)&msg.data[0], (uint8_t*)&can_msg.data[0], can_msg.data_length_code);
    return 1;
  }
  return 0;
}

/**************************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************************************/

bool ArduinoCAN::cfg_pins(int const max_index, int const can_tx_pin, int const can_rx_pin)
{
  /* Verify if indices are good. */
  if (can_tx_pin < 0 || can_rx_pin < 0 || can_tx_pin >= max_index || can_rx_pin >= max_index) {
    return false;
  }

  /* Getting configuration from table. */
  const uint16_t * cfg = nullptr;
  cfg = g_pin_cfg[can_tx_pin].list;
  uint16_t cfg_can_tx = getPinCfg(cfg, PIN_CFG_REQ_CAN_TX, /* prefer_sci = */ false);
  cfg = g_pin_cfg[can_rx_pin].list;
  uint16_t cfg_can_rx = getPinCfg(cfg, PIN_CFG_REQ_CAN_RX, /* prefer_sci = */ false);

  /* Verify if configurations are good. */
  if (cfg_can_tx == 0 || cfg_can_rx == 0) {
    return false;
  }

  /* Verify if channel is the same for all pins. */
  uint32_t const ch_can_tx = GET_CHANNEL(cfg_can_tx);
  uint32_t const ch_can_rx = GET_CHANNEL(cfg_can_rx);
  if (ch_can_tx != ch_can_rx) {
    return false;
  }

  /* Actually configure pin functions. */
  R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[can_tx_pin].pin, (uint32_t) (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_CAN));
  R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[can_rx_pin].pin, (uint32_t) (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_CAN));

  return true;
}

/**************************************************************************************
 * CALLBACKS FOR FSP FRAMEWORK
 **************************************************************************************/

extern "C" void can_callback(can_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case CAN_EVENT_TX_COMPLETE:
        {
            CAN.tx_complete = true;        //set flag bit
            break;
        }
        case CAN_EVENT_RX_COMPLETE: // Currently driver don't support this. This is unreachable code for now.
        {
            CAN.rx_complete = true;
            break;
        }
        case CAN_EVENT_ERR_WARNING:             //error warning event
        case CAN_EVENT_ERR_PASSIVE:             //error passive event
        case CAN_EVENT_ERR_BUS_OFF:             //error Bus Off event
        case CAN_EVENT_BUS_RECOVERY:            //Bus recovery error event
        case CAN_EVENT_MAILBOX_MESSAGE_LOST:    //overwrite/overrun error event
        case CAN_EVENT_ERR_BUS_LOCK:            // Bus lock detected (32 consecutive dominant bits).
        case CAN_EVENT_ERR_CHANNEL:             // Channel error has occurred.
        case CAN_EVENT_TX_ABORTED:              // Transmit abort event.
        case CAN_EVENT_ERR_GLOBAL:              // Global error has occurred.
        case CAN_EVENT_TX_FIFO_EMPTY:           // Transmit FIFO is empty.
        {
            CAN.err_status = true;          //set flag bit
            break;
        }
    }
}

/**************************************************************************************
 * OBJECT INSTANTIATION
 **************************************************************************************/

#if CAN_HOWMANY > 0
ArduinoCAN CAN(PIN_CAN0_TX, PIN_CAN0_RX);
#endif

#if CAN_HOWMANY > 1
ArduinoCAN CAN1(PIN_CAN1_TX, PIN_CAN1_RX);
#endif
