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
#ifndef GPS_UART_H
#define GPS_UART_H

#include "IGPS.h"

class GPS_UART : public IGPS {
public:
    GPS_UART();

    bool init();
    bool stop();

    double   getLongitude();
    double   getLatitude();
    double   getAltitude();
    uint64_t getTime();
    float    getDilution();
    int      getSatelliteCount();

private:
    void acquire();
    void noLockTelemetry();
    int checksum(const char *s);
    bool nmeaParse(const char *msg, int msglen, char *rawnmea, int *positions, char **buffers, int len);

    bool run;
    int tty_fd;
    double lon;
    double lat;
    double alt;
    uint64_t time;
    float dilution;
    int sCount;
};

#endif     /* GPS_UART_H */
