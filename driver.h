/* -*- C++ -*- */
/* 2023 Robotics @ Maryland
   Alexander Yelovich, alexyel84@gmail.com
*/

#ifndef DRIVER_H
#define DRIVER_H

#include <cstdint>

// This should be removed in favor of defining at compile time
#define DEBUG

#if defined(DEBUG)
#define LOG(x) do { std::cerr << x << '\n'; } while (0)
#else
#define LOG(x)
#endif

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;

class I2CDevice {
public:
    enum I2CFunc {
        NONE,
        NORMAL,
        SMBUS
    };

    I2CDevice(u16 paddr);
    ~I2CDevice();
    int open_i2c(int num);
    int close_i2c();
    int write_byte(u8 reg, u8 data);
    int read_byte(u8 reg);

protected:
    int fd;
    u16 addr;
    
private:
    bool update_func();
    
    I2CFunc func;
};

#endif
