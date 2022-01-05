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
#include <cmath>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <thread>

#include "Mag_LSM9DS1.h"

volatile sig_atomic_t stop = 0;

void inthand(int signum) {
    stop = 1;
}

int main(int argc, char** argv)
{
    bool dataout = argc > 1;

    signal(SIGINT, inthand);

    short min[3] = {SHRT_MAX};
    short max[3] = {SHRT_MIN};
    short xyz[3] = {0};

    short offsets[3] = {0, 0, 0};

    Mag_LSM9DS1 mag(offsets, 75);
    bool ok = mag.init();

    if(!ok)
        return -1;

    int i;
    do {
        xyz[0] = (short) mag.getX();
        xyz[1] = (short) mag.getY();
        xyz[2] = (short) mag.getZ();

        if(dataout) {
            printf("%d,%d,%d\n", xyz[0], xyz[1], xyz[2]);
            fflush(stdout);
        }

        for(i = 0; i < 3; i++) {
            if(xyz[i] < min[i]) min[i] = xyz[i];
            if(xyz[i] > max[i]) max[i] = xyz[i];
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while(!stop);

    mag.stop();

    if(!dataout) {
        printf("Calibration complete....\n");
        printf("Calculating offsets...\n");
    }

    short center[3] = {0};
    for(i = 0; i < 3; i++)
        center[i] = (short) round((max[i] + min[i]) / 2.0);

    if(dataout) {
        printf(">%d,%d,%d\n", center[0], center[1], center[2]);
        fflush(stdout);
    }

    if(!dataout)
        printf("Writing 'mag_calibration.json' with file truncation enabled....\n");

    FILE *fp = fopen("/opt/apispoint/config/mag_calibration.json", "w");
    fprintf(fp, "{\"mag_offsets\":[%d,%d,%d]}", center[0], center[1], center[2]);
    fclose(fp);

    if(!dataout)
        printf("Done and done.\n");

    return 0;
}
