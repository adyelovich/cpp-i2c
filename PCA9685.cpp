/* 2023 Robotics @ Maryland
   Alexander Yelovich, alexyel84@gmail.com
*/

#include <iostream>

#include <cmath>
#include <cstdio> // for debugging

#include "driver.h"
#include "PCA9685.h"

const int PCA9685::pwm_freq_hz = 200;
const int PCA9685::delay_percent = 10;
const int PCA9685::PCA9685_ledn_prec = 4096;
const int PCA9685::paddr = 0x41;
const int PCA9685::bus_num = 1;
const u8 PCA9685::PWM_LED[16] = {0x06, 0x0A, 0x0E, 0x12,
                                 0x16, 0x1A, 0x1E, 0x22,
                                 0x26, 0x2A, 0x2E, 0x32,
                                 0x36, 0x3A, 0x3E, 0x42};

PCA9685::PCA9685() : I2CDevice::I2CDevice(PCA9685::paddr) {
    u8 data;
    
    // Somehow figure out which i2c bus we are working with
    // on the raspberry pi this is 1
    LOG("<PCA9685>: opening i2c");

    if (open_i2c(PCA9685::bus_num)) {
        std::cerr << "error opening i2c\n";
        // there was an error, don't do anything,
        // maybe throw exception here depending on what is idiomatic for ROS
        return;
    }

    if ((data = read_byte(0x00)) < 0) {
        // error, throw exception?
        std::cerr << "there was an error\n";
        return;
    }

    // if errors for later write bytes, consider throwing an exception
    
    if (data & PCA9685_SLEEP)
        write_byte(0x00, 0x00); // NOTE: we are turning off ALLCALL here

    // Make updates go into effect after writing all 4 LED registers
    // Also keep totem-pole structure (not sure what this means,
    // but it's the default so I'm keeping it this way)
    write_byte(0x01, 0x0C); 
}

PCA9685::~PCA9685() {
    // automatically calls parent's destructor
}

int PCA9685::write_pwm(const int pwm, const int channel) {
    u8 bytes[4];
    u32 ledn = pwm_to_ledn(bytes, pwm);

    for (int i = 0; i < 4; i++) {
        std::printf("writing byte 0x%02x to channel 0x%02x\n",
                    bytes[i],
                    channel + i);
                    
        write_byte(channel + i, bytes[i]);
    }
    
    return 1;
}

bool PCA9685::pwm_to_ledn(u8 *bytes, const int pwm) const {
    double period_us = 1000.0 * 1000 / pwm_freq_hz;
    double duty = (pwm / period_us) * 100;
    bool success = duty <= 100.0;
    std::printf("converting pwm %d to duty %lf%%\n", pwm, duty);

    if (!success) {
        // eventually implement functionality for this 
        LOG("<PCA9685>: WARNING: attempting to write duty above 100%");
    }

    return success ? duty_to_ledn(bytes, duty) : false;
}

bool PCA9685::duty_to_ledn(u8 *bytes, const double percent) const {
    unsigned int delay = duty_to_count(delay_percent);
    std::printf("converting duty %lf%% to count %u\n", percent, duty_to_count(percent));
    
    // First write the delay
    bytes[0] = (delay - 1) & 0xFF;
    bytes[1] = ((delay - 1) >> 8) & 0xFF;

    // Now write the given PWM
    unsigned int stop = delay + duty_to_count(percent);
    bytes[2] = (stop - 1) & 0xFF;
    bytes[3] = ((stop - 1) >> 8) & 0xFF;
    
    // insert a check for setting the MSBs
    if ((bytes[1] & 0xE0) || (bytes[3] & 0xE0)) {
        std::cerr << "Warning: writing the most significant bits in "
                  << "registers, this is undefined behavior\n";
        return false;
    }
    
    return true;
}

unsigned int PCA9685::duty_to_count(const double percent) const {
    return std::lround((PCA9685_ledn_prec * percent) / 100.0);
}

/*
u32 PCA9685::count_to_reg(unsigned int count) {
    u32 regs;

    if (count >= 0x1000) {
        LOG("<thrusters>: ERROR: attempting to set one of 4 MSBs in LEDn_H");
        return false;
    }
    
    regs = count
    buf[0] = (count & 0xFF);
    buf[1] = (count & 0xFF00) >> 8;

    return true;
}
*/
