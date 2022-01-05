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
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "GPS_UART.h"

int main(int argc,char** argv)
{
    GPS_UART gps;
    bool ok = gps.init();

    if(!ok)
        return -1;

    sleep(5);

    double *gps_info = gps.getPosition();
    printf("lon: %lf, lat: %lf, alt: %lf\n", gps_info[0], gps_info[1], gps_info[2]);
    printf("time(GPS):  %lld\ntime(NULL): %ld\n", gps.getTime(), time(NULL));

    gps.stop();

    return 0;
}
