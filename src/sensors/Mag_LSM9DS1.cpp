//
//    Copyright (c) 2021 APIS Point, LLC.
//    All rights reserved.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#include "Mag_LSM9DS1.h"

#include <chrono>
#include <cmath>
#include <fcntl.h>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>

#ifndef NOI2C
#include <linux/i2c-dev.h>
#endif

#include <sys/ioctl.h>

#ifndef MAG_DEV
    #define MAG_DEV "/dev/i2c-2"
#endif

#ifndef MAG_ADDR
    #define MAG_ADDR 0x1e
#endif

Mag_LSM9DS1::Mag_LSM9DS1(short *offsets, int delay_ms) {
    run = false;
    x = y = z = angle = std::numeric_limits<double>::quiet_NaN();
    offset = offsets;
    delay = delay_ms;
}

bool Mag_LSM9DS1::deviceOk() {
    unsigned char buf[6] = {0};

    unsigned char cmdbuf[2] = {0x0F, 0x00};
    int result = write(fd, cmdbuf, 1);

    if(result != 1 || read(fd, buf, 1) != 1 || buf[0] != 0x3D) {
        fprintf(stderr, "Failed to write to or identify magnetometer\n");
        return false;
    }

    return true;
}

void Mag_LSM9DS1::acquire() {
    unsigned char buf[6] = {0};

    unsigned char cmdbuf[2] = {0x0F, 0x00};
    int result = write(fd, cmdbuf, 1);

    if(result != 1 || read(fd, buf, 1) != 1 || buf[0] != 0x3D) {
        fprintf(stderr, "Failed to write to or identify magnetometer\n");
        return;
    }

    // CTRL_REG1
    // Enable temperature compensation, XY-axis Ultra-high performance, and 10Hz
    cmdbuf[0] = 0x20;
    cmdbuf[1] = 0xF0;
    write(fd, cmdbuf, 2);

    // CTRL_REG2
    // Enable continuous conversion mode
    cmdbuf[0] = 0x22;
    cmdbuf[1] = 0x00;
    write(fd, cmdbuf, 2);

    // CTRL_REG4
    // Enable Z-axis Ultra-high performance
    cmdbuf[0] = 0x23;
    cmdbuf[1] = 0x0C;
    write(fd, cmdbuf, 2);

    // CTRL_REG5
    // Update data registers only after read
    cmdbuf[0] = 0x24;
    cmdbuf[1] = 0x00;
    write(fd, cmdbuf, 2);

    double TO_360 = 360.0;
    double TO_DEG = 180.0 / M_PI;

    cmdbuf[0] = 0x28;
    do {
        write(fd, cmdbuf, 1);
        read(fd, buf, 6);

        x = (short) ((buf[1] << 8) | buf[0]) - offset[0];
        y = (short) ((buf[3] << 8) | buf[2]) - offset[1];
        z = (short) ((buf[5] << 8) | buf[4]) - offset[2];

        angle = -atan2(x, y) * TO_DEG;
        if(angle < 0) angle += TO_360;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    } while(run);
}

bool Mag_LSM9DS1::init() {
    if(run == true)
        return run;

    bool ok = true;

    #ifndef NOI2C
        if((fd = open(MAG_DEV, O_RDWR)) < 0) {
            fprintf(stderr, "Failed to open magnetometer device\n");
            ok = false;
        }
        if(ioctl(fd, I2C_SLAVE, MAG_ADDR) < 0) {
            fprintf(stderr, "Failed to acquire magnetometer on i2c bus\n");
            ok = false;
        }
        ok = deviceOk();

        run = ok;
        if(run)
            std::thread(&Mag_LSM9DS1::acquire, this).detach();
    #endif

    return run;
}

bool Mag_LSM9DS1::stop() {
    if(run == false)
        return !run;

    run = false;
    close(fd);

    return !run;
}

double Mag_LSM9DS1::getX() {
    return x;
}

double Mag_LSM9DS1::getY() {
    return y;
}

double Mag_LSM9DS1::getZ() {
    return z;
}

double Mag_LSM9DS1::getHeading() {
    return angle;
}
