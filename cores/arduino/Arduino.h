#ifndef __ARDUINO__H__
#define __ARDUINO__H__

#ifdef AZURE_RTOS_THREADX
#include "usb/SerialUSB.h"
#include "tx_api.h"
#endif
#include "api/ArduinoAPI.h"
#include "bsp_api.h"
#include "r_ioport.h"
#include "pwm.h"
#include "Serial.h"

#if defined(__cplusplus)

#undef F
// C++11 F replacement declaration
template <typename T1>
auto F(T1&& A)
  -> const arduino::__FlashStringHelper*
{
  return (const arduino::__FlashStringHelper*)A;
}

#endif //__cplusplus

#define interrupts()        __enable_irq()
#define noInterrupts()      __disable_irq()

// We provide analogReadResolution and analogWriteResolution APIs
int getAnalogReadResolution();
void analogReadResolution(int bits);
void analogWriteResolution(int bits);

typedef struct rtc_alarm_time
{
    bool       sec_match;              ///< Enable the alarm based on a match of the seconds field
    bool       min_match;              ///< Enable the alarm based on a match of the minutes field
    bool       hour_match;             ///< Enable the alarm based on a match of the hours field
    bool       mday_match;             ///< Enable the alarm based on a match of the days field
    bool       mon_match;              ///< Enable the alarm based on a match of the months field
    bool       year_match;             ///< Enable the alarm based on a match of the years field
    bool       dayofweek_match;        ///< Enable the alarm based on a match of the dayofweek field
} RtcAlarmSettings;

void setRtcTime(rtc_time_t time);
rtc_time_t getRtcTime();
void setRtcPeriodicInterrupt(rtc_periodic_irq_select_t period, void (* func)(rtc_callback_args_t *));
void setRtcAlarm(rtc_time_t time, RtcAlarmSettings time_match, void (* func)(rtc_callback_args_t *));

typedef struct _PinDescription
{
  bsp_io_port_pin_t name;
} PinDescription ;


typedef struct _AnalogPinDescription
{
  bsp_io_port_pin_t name;
  adc_channel_t ch;
} AnalogPinDescription;

typedef struct _AnalogOutPinDescription
{
  bsp_io_port_pin_t name;
  int ch;
} AnalogOutPinDescription;


typedef struct {
    gpt_instance_ctrl_t* gpt_ctrl;
    const timer_cfg_t* gpt_cfg;
    gpt_io_pin_t gpt_pin;
} pwmTable_t;

extern const PinDescription g_APinDescription[];
extern const AnalogPinDescription g_AAnalogPinDescription[];
extern const AnalogOutPinDescription g_AAnalogOutPinDescription[];
extern const pwmTable_t pwmTable[];

#define Serial1 _UART1_
#define Serial2 _UART2_
#define Serial3 _UART3_
#define Serial4 _UART4_
#define Serial5 _UART5_

#include "pins_arduino.h"

#endif //__ARDUINO__H__