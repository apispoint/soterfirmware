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
#include "TmpBaro_MPL115A2.h"

#include <chrono>
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

#ifndef TMPBARO_DEV
    #define TMPBARO_DEV "/dev/i2c-1"
#endif

#ifndef TMPBARO_ADDR
    #define TMPBARO_ADDR 0x60
#endif

TmpBaro_MPL115A2::TmpBaro_MPL115A2() {
    run = false;
    temp = std::numeric_limits<double>::quiet_NaN();
    pressure = std::numeric_limits<double>::quiet_NaN();
}

void TmpBaro_MPL115A2::acquire() {
    unsigned char config[2] = {0x12, 0x00};
    unsigned char data[8] = {0};
    unsigned char reg[1] = {0x00};

    int pres, ctmp;
    float pComp;

    do {
        write(fd, config, 2);

        // Per the datasheet tc (conversion time) is 3ms max
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        write(fd, reg, 1);
        if(read(fd, data, 4) == 4) {
            // Convert the data to 10-bits
            pres = (data[0] << 8 | data[1]) >> 6;
            ctmp = (data[2] << 8 | data[3]) >> 6;

            // Calculate pressure compensation
            pComp = A0 + (B1 + C12 * ctmp) * pres + B2 * ctmp;

            // Convert the data
            pressure = (65.0 / 1023.0) * pComp + 50.0;
            temp = (ctmp - 498.0) / -5.35 + 25.0;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while(run);
}

bool TmpBaro_MPL115A2::init() {
    if(run == true)
        return true;

    bool ok = true;
    #ifndef NOI2C
        if((fd = open(TMPBARO_DEV, O_RDWR)) < 0) {
            fprintf(stderr, "Failed to open temperature-pressure device\n");
            ok = false;
        }
        if(ioctl(fd, I2C_SLAVE, TMPBARO_ADDR) < 0) {
            fprintf(stderr, "Failed to acquire temperature-pressure on i2c bus\n");
            ok = false;
        }
        unsigned char reg[1] = {0};

        if(write(fd, reg, 1) == 1) {
            char data[8] = {0};
            if(read(fd, data, 8) == 8) {
                A0  = (data[0] << 8  | data[1])       / 8.0;
                B1  = (data[2] << 8  | data[3])       / 8192.0;
                B2  = (data[4] << 8  | data[5])       / 16384.0;
                C12 = ((data[6] << 8 | data[7]) >> 2) / 4194304.0;
            }
            else {
                fprintf(stderr, "Failed to acquire temperature-pressure initialize data\n");
                ok = false;
            }
        }
        else {
            fprintf(stderr, "Failed to acquire temperature-pressure initialization register\n");
            ok =false;
        }

        run = ok;
        if(run)
            std::thread(&TmpBaro_MPL115A2::acquire, this).detach();
    #endif

    return run;
}

bool TmpBaro_MPL115A2::stop() {
    if(run == false)
        return !run;

    run = false;
    close(fd);

    return !run;
}

double TmpBaro_MPL115A2::getTemperature() {
    return temp;
}

double TmpBaro_MPL115A2::getPressure() {
    return pressure;
}
