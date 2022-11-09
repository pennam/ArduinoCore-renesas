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
 * NAMESPACE
 **************************************************************************************/

namespace arduino
{

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

template <size_t const RING_BUFFER_SIZE>
CanMsgRingbuffer<RING_BUFFER_SIZE>::CanMsgRingbuffer()
: _head{0}
, _tail{0}
, _num_elems{0}
{
}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

template <size_t const RING_BUFFER_SIZE>
void CanMsgRingbuffer<RING_BUFFER_SIZE>::enqueue(CanMsg const & msg)
{
  if (isFull())
    return;

  _buf[_head] = msg;
  _head = next(_head);
  _num_elems++;
}

template <size_t const RING_BUFFER_SIZE>
CanMsg CanMsgRingbuffer<RING_BUFFER_SIZE>::dequeue()
{
  if (isEmpty())
    return CanMsg();

  CanMsg const msg = _buf[_tail];
  _tail = next(_tail);
  _num_elems--;

  return msg;
}

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* arduino */
