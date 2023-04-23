/* -*- C++ -*- */
/* 2023 Robotics @ Maryland
   Alexander Yelovich, alexyel84@gmail.com
*/

#ifndef PCA9685_H
#define PCA9685_H

#include "driver.h"

#define PCA9685_SLEEP 0x10

/* features this device should have
  - writing pwm to a specific channel
  - writing 0 to everything
*/

class PCA9685 : protected I2CDevice {
public:
    static const u8 PWM_LED[];

    PCA9685();
    ~PCA9685();
    int write_pwm(const int pwm, const int channel);
    
    
private:
    static const int pwm_freq_hz;
    static const int delay_percent;
    static const int PCA9685_ledn_prec;
    static const int paddr;
    static const int bus_num;
    
    bool pwm_to_ledn(u8 *bytes, const int pwm) const;
    bool  duty_to_ledn(u8 *bytes, const double percent) const;
    unsigned int duty_to_count(const double percent) const;
};

#endif
