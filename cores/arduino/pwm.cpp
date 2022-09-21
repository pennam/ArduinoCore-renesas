#include "Arduino.h"
#include "pwm.h"
#include "bsp_api.h"

extern const PinMuxCfg_t g_pin_cfg[];
extern const size_t g_pin_cfg_size;

PwmOut::PwmOut(pin_size_t pinNumber) : 
  _pin(pinNumber),
  _enabled(false)
{

}

PwmOut::~PwmOut() {
}




bool PwmOut::cfg_pin(int max_index) {
  /* verify index are good */
  if(_pin < 0 || _pin >= max_index) {
    return false;
  }
  /* getting configuration from table */
  const uint16_t *cfg = g_pin_cfg[_pin].list;
  uint16_t pin_cgf = getPinCfg(cfg, PIN_CFG_REQ_PWM,false);
  
  /* verify configuration are good */
  if(pin_cgf == 0) {
    return false;
  }
  
  timer_channel = GET_CHANNEL(pin_cgf);

  _is_gtp = IS_PIN_GTI_PWM(pin_cgf);

  _pwm_channel = IS_PWM_ON_A(pin_cgf) ? CHANNEL_A : CHANNEL_B;

  /* actually configuring PIN function */
  R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[_pin].pin, (uint32_t) (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));
  return true;

}

/* default begin function, starts the timers with default pwm configuration (490Hz and 50%) */
bool PwmOut::begin() {
  bool rv = true;
  int max_index = g_pin_cfg_size / sizeof(g_pin_cfg[0]);
  rv &= cfg_pin(max_index);
  
  if(rv) {
    /* extended PWM CFG*/
    timer_pwm_extended_cfg.trough_ipl               = (BSP_IRQ_DISABLED);
    timer_pwm_extended_cfg.trough_irq               = FSP_INVALID_VECTOR;
    timer_pwm_extended_cfg.poeg_link                = GPT_POEG_LINK_POEG0;
    timer_pwm_extended_cfg.output_disable           = GPT_OUTPUT_DISABLE_NONE;
    timer_pwm_extended_cfg.adc_trigger              = GPT_ADC_TRIGGER_NONE;
    timer_pwm_extended_cfg.dead_time_count_up       = 0;
    timer_pwm_extended_cfg.dead_time_count_down     = 0;
    timer_pwm_extended_cfg.adc_a_compare_match      = 0;
    timer_pwm_extended_cfg.adc_b_compare_match      = 0;
    timer_pwm_extended_cfg.interrupt_skip_source    = GPT_INTERRUPT_SKIP_SOURCE_NONE;
    timer_pwm_extended_cfg.interrupt_skip_count     = GPT_INTERRUPT_SKIP_COUNT_0;
    timer_pwm_extended_cfg.interrupt_skip_adc       = GPT_INTERRUPT_SKIP_ADC_NONE;
    timer_pwm_extended_cfg.gtioca_disable_setting   = GPT_GTIOC_DISABLE_PROHIBITED;
    timer_pwm_extended_cfg.gtiocb_disable_setting   = GPT_GTIOC_DISABLE_PROHIBITED;
    
    rv &= timer.begin_pwm(GPT_TIMER, timer_channel, &timer_pwm_extended_cfg, _pwm_channel);    
  }
  _enabled = rv;
  return rv;

}

bool begin_usec(uint32_t period_us, uint32_t pulse_us) {

}


/* begin "standard", period and pulse when raw is false are supposed to be expressed in usec */
/* -------------------------------------------------------------------------- */
bool PwmOut::begin(uint32_t period_width, uint32_t pulse_width, bool raw /*= false */, timer_source_div_t sd /*= TIMER_SOURCE_DIV_1*/) {
/* -------------------------------------------------------------------------- */
  bool rv = true;
  int max_index = g_pin_cfg_size / sizeof(g_pin_cfg[0]);
  rv &= cfg_pin(max_index);
  
  if(rv) {
    /* extended PWM CFG*/
    timer_pwm_extended_cfg.trough_ipl               = (BSP_IRQ_DISABLED);
    timer_pwm_extended_cfg.trough_irq               = FSP_INVALID_VECTOR;
    timer_pwm_extended_cfg.poeg_link                = GPT_POEG_LINK_POEG0;
    timer_pwm_extended_cfg.output_disable           = GPT_OUTPUT_DISABLE_NONE;
    timer_pwm_extended_cfg.adc_trigger              = GPT_ADC_TRIGGER_NONE;
    timer_pwm_extended_cfg.dead_time_count_up       = 0;
    timer_pwm_extended_cfg.dead_time_count_down     = 0;
    timer_pwm_extended_cfg.adc_a_compare_match      = 0;
    timer_pwm_extended_cfg.adc_b_compare_match      = 0;
    timer_pwm_extended_cfg.interrupt_skip_source    = GPT_INTERRUPT_SKIP_SOURCE_NONE;
    timer_pwm_extended_cfg.interrupt_skip_count     = GPT_INTERRUPT_SKIP_COUNT_0;
    timer_pwm_extended_cfg.interrupt_skip_adc       = GPT_INTERRUPT_SKIP_ADC_NONE;
    timer_pwm_extended_cfg.gtioca_disable_setting   = GPT_GTIOC_DISABLE_PROHIBITED;
    timer_pwm_extended_cfg.gtiocb_disable_setting   = GPT_GTIOC_DISABLE_PROHIBITED;
    
    
    if(raw) {
      rv &= timer.begin(TIMER_MODE_PWM, (_is_gtp) ? GPT_TIMER : AGT_TIMER , timer_channel,  period_width, pulse_width, sd);
    }
    else {
      /* NOTE: I suppose period and pulse are expressed in ms */
      float freq_hz = (1000000.0 / (float)period_width);
      float duty_perc = ((float)pulse_width * 100.0 / (float)period_width);
      rv &= timer.begin(TIMER_MODE_PWM, (_is_gtp) ? GPT_TIMER : AGT_TIMER , timer_channel, freq_hz, duty_perc);
    }

    timer.set_pwm_extended_cfg(&timer_pwm_extended_cfg);
    
    if(_pwm_channel == CHANNEL_A) {
      timer.enable_gtioc_CH_A();
    } 
    else {
      timer.enable_gtioc_CH_B();
    }
  }

  if(rv) {
    fsp_err_t err = R_GPT_Open(timer.get_ctrl_ptr(),timer.get_cfg_ptr());
    if ((err != FSP_ERR_ALREADY_OPEN) && (err != FSP_SUCCESS)) {
      return false;
    }
  }
  
  if (R_GPT_Start(timer.get_ctrl_ptr()) != FSP_SUCCESS) {
    return false;
  }

  _enabled = true;
  return rv;
}

bool PwmOut::period(int ms) {
  return timer.set_period_ms((double)ms);
}

bool PwmOut::pulseWidth(int ms) {
  if(_pwm_channel == CHANNEL_A) {
    return timer.set_pulse_ms((double)ms,GPT_IO_PIN_GTIOCA);
  }
  else {
    return timer.set_pulse_ms((double)ms,GPT_IO_PIN_GTIOCB);
  }  
}

bool PwmOut::period_us(int us) {
  return timer.set_period_us((double)us);
}

bool PwmOut::pulseWidth_us(int us) {
  if(_pwm_channel == CHANNEL_A) {
    return timer.set_pulse_us((double)us,GPT_IO_PIN_GTIOCA);
  }
  else {
    return timer.set_pulse_us((double)us,GPT_IO_PIN_GTIOCB);
  }  
  
}

bool PwmOut::period_raw(int period) {
  if (R_GPT_PeriodSet(timer.get_ctrl_ptr(), period) != FSP_SUCCESS) {
    return false;
  }
  return true;
}

bool PwmOut::pulseWidth_raw(int pulse) {
  if(_pwm_channel == CHANNEL_A) {
    if (R_GPT_DutyCycleSet(timer.get_ctrl_ptr(), pulse, GPT_IO_PIN_GTIOCA) != FSP_SUCCESS) {
      return false;
    }
  }
  else {
    if (R_GPT_DutyCycleSet(timer.get_ctrl_ptr(), pulse, GPT_IO_PIN_GTIOCB) != FSP_SUCCESS) {
      return false;
    }
  }  
  return true;
}

bool PwmOut::pulse_perc(float duty) {
  timer_info_t info;
  R_GPT_InfoGet(timer.get_ctrl_ptr(), &info);
  float period = (float)info.period_counts;
  float pulse = period * duty / 100.0;
  pulseWidth_raw((int)pulse);
}


void PwmOut::suspend() {
  if (_enabled) {
    R_GPT_Stop(timer.get_ctrl_ptr());
    _enabled = false;
  }
}

void PwmOut::resume() {
  if (!_enabled) {
    R_GPT_Start(timer.get_ctrl_ptr());
    _enabled = true;
  }
}

void PwmOut::end() {
  R_GPT_Close(timer.get_ctrl_ptr());
  _enabled = false;
}