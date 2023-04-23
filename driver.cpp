/* 2023 Robotics @ Maryland
   Alexander Yelovich, alexyel84@gmail.com
*/

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

extern "C" {
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <i2c/smbus.h>
}

#include "driver.h"

/*
  Need to somehow check if the functions have been queried,
  i2c_write should use the appropriate function (either SMBus
  or generic read/write)
 */

I2CDevice::I2CDevice(u16 paddr)
    : fd(-1), addr(paddr), func(I2CDevice::I2CFunc::NONE)
{

}

I2CDevice::~I2CDevice() {
    close_i2c();
}

int I2CDevice::open_i2c(int num) {
    std::stringstream s1;

    s1 << "/dev/i2c-" << num;

    const std::string i2c_adapter_str = std::string(s1.str());
    const char *i2c_adapter = i2c_adapter_str.c_str();

    LOG("<cpp-i2c>: Opening adapter: " << s1.str());
    if ((fd = open(i2c_adapter, O_NONBLOCK | O_RDWR)) < 0) {
        std::perror("<cpp-i2c>: error opening i2c bus");
        return -1;
    }

    if (!update_func()) {
        LOG("<cpp-i2c>: Failed updating func");
        return -1;
    }

    return 0;
}

int I2CDevice::close_i2c() {
    LOG("<cpp-i2c>: closing i2c bus");
    return close(fd);
}

int I2CDevice::write_byte(u8 reg, u8 data) {
    u8 buf[2];

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        std::perror("<cpp-i2c>: failure in setting IP address");
        return -1;
    }
    
    switch (func) {
    case I2CFunc::NONE:
        LOG("<cpp-i2c>: error querying I2C funcs");
        return -1;
        break;
    case I2CFunc::SMBUS:
        if (i2c_smbus_write_byte_data(fd, reg, data) < 0) {
            std::perror("<cpp-i2c>: failure writing SMBus communication");
            return -1;
        }
        break;
    case I2CFunc::NORMAL:
        buf[0] = reg;
        buf[1] = data;
        if (write(fd, buf, 2) < 0) {
            std::perror("<cpp-i2c>: failure writing normal I2C connection");
            return -1;
        }
        break;
    default:
        LOG("<cpp-i2c>: unknown I2C functionality error");
        return -1;
        break;
    }

    return 0;
}

int I2CDevice::read_byte(u8 reg) {
    u8 data;

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        std::perror("<cpp-i2c>: failure in setting IP address");
        return -1;
    }
    
    switch (func) {
    case I2CFunc::NONE:
        LOG("<cpp-i2c>: error querying I2C funcs");
        return -1;
        break;
    case I2CFunc::SMBUS:
        if ((data = i2c_smbus_read_byte_data(fd, reg)) < 0) {
            std::perror("<cpp-i2c>: failure writing SMBus communication");
            return -1;
        }
        break;
    case I2CFunc::NORMAL:
        if (read(fd, &data, 1) < 0) {
            std::perror("<cpp-i2c>: failure writing normal I2C connection");
            return -1;
        }
        break;
    default:
        LOG("<cpp-i2c>: unknown I2C functionality error");
        return -1;
        break;
    }

    return data;
}


bool I2CDevice::update_func() {
    unsigned long funcs;

    if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
        std::perror("<cpp-i2c>: failed querying funcs");
        return false;
    }

    // We only care about BYTE_DATA because that we are only
    // going to write one byte messages
    if (funcs & I2C_FUNC_SMBUS_BYTE_DATA) {
        LOG("<cpp-i2c>: messaging via smbus");
        func = I2CFunc::SMBUS;
    } else if (funcs & I2C_FUNC_I2C) {
        LOG("<cpp-i2c>: messaging via regular i2c read write");
        func = I2CFunc::NORMAL;
    }

    return true;
}
