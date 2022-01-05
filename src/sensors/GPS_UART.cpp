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
#include "GPS_UART.h"

#include <ctime>
#include <fcntl.h>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#include "BaseStatistics.hpp"

#ifndef GPS_DEV
    #define GPS_DEV "/dev/ttyS4"
#endif

#ifdef GPS_B4800
    #define GPS_BAUD B4800
#else
    #define GPS_BAUD B9600
#endif

#ifndef GPS_SATCOUNT_MINIMUM
    #define GPS_SATCOUNT_MINIMUM 3
#endif

GPS_UART::GPS_UART() {
    run = false;
    dilution = time = sCount = 0;
    lon = lat = alt = std::numeric_limits<double>::quiet_NaN();
}

void GPS_UART::noLockTelemetry() {
    time = 0;
    lon = lat = alt = std::numeric_limits<double>::quiet_NaN();
}

int GPS_UART::checksum(const char *s) {
    int c = 0;

    if(s[0] == '$') s++;

    while(*s && *s != '*')
        c ^= *s++;

    return c;
}

bool GPS_UART::nmeaParse(const char *msg, int msglen, char *rawnmea, int *positions, char **buffers, int len) {
    bool ok = false;

    if(strncmp(rawnmea, msg, msglen) != 0)
        return ok;

    if(strchr(rawnmea, '*') == NULL)
        return ok;

    char *s = rawnmea, *token, *nmea, *csum, isum[3];

    int index = 0;
    while( (token = strsep(&s, "*")) != NULL && index < 2 ) {
        if(index++ == 0) nmea = token;
        else             csum = token;
    }

    sprintf(isum, "%02X", checksum(nmea));
    ok = strncmp(isum, csum, 2) == 0;

    if(ok) {
        int pos = 0;
        index = 0;
        while(ok && pos < len && (buffers[pos] = strsep(&nmea, ",")) != NULL) {
            if(index == positions[pos]) {
                ok &= *buffers[pos] != '\0';
                pos++;
            }
            index++;
        }
    }

    return ok;
}

void GPS_UART::acquire() {
    int bufSize = 1;
    double _1_DIV_60 = 1.0 / 60.0;

    struct termios tio;
    bzero(&tio, sizeof(tio));

    tio.c_cflag = CS8 | CREAD | CLOCAL;
    cfsetospeed(&tio, GPS_BAUD);
    cfsetispeed(&tio, GPS_BAUD);

    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;

    tio.c_cc[VMIN] = bufSize;

    tcflush(tty_fd, TCIFLUSH);
    tcsetattr(tty_fd, TCSANOW, &tio);

    char nmea[83];
    int res, i = 0, j;

    float gtime;
    int date;
    int igtime;

    float decdeg[2];
    char hemi[2];

    int locked = 0;
    char status;

    double deg, elv;

    BaseStatistics<double> latlngalt[3];

    int gpgga_positions[] = {2, 3, 4, 5, 6, 7, 8, 9};
    int gpgga_poscnt = 8;
    char *gpgga_tokens[gpgga_poscnt];

    int gprmc_positions[] = {1, 2, 9};
    int gprmc_poscnt = 3;
    char *gprmc_tokens[gprmc_poscnt];

    do {
        res = read(tty_fd, &nmea[i++], bufSize);

        if(nmea[i-1] == '\n') {
            nmea[i] = '\0';
            i = 0;

#ifdef DEBUGNMEA
            printf("%s\n", nmea);
#endif

            //
            // Parse NMEA GPS GGA string
            //
            if(nmeaParse("$GPGGA", 6, nmea, gpgga_positions, gpgga_tokens, gpgga_poscnt)) {
                // See NMEA message for format
                decdeg[0] = atof(gpgga_tokens[0]);
                hemi[0] = gpgga_tokens[1][0];
                decdeg[1] = atof(gpgga_tokens[2]);
                hemi[2] = gpgga_tokens[3][0];
                locked = atoi(gpgga_tokens[4]);
                sCount = atoi(gpgga_tokens[5]);
                dilution = atof(gpgga_tokens[6]);
                elv = atof(gpgga_tokens[7]);

                if(
                     locked != 0
                  && sCount > GPS_SATCOUNT_MINIMUM) {
                    latlngalt[2].add(&elv, 1);
                    alt = latlngalt[2].get_mean();
                    for(j = 0; j < 2; j++) {
                        deg = (int) (decdeg[j] * 0.01);
                        deg += ((decdeg[j] - (deg * 100)) * _1_DIV_60);
                        if(hemi[j] == 'S' || hemi[j] == 'W')
                            deg *= -1;
                        latlngalt[j].add(&deg, 1);
                        if(j == 0) lat = latlngalt[j].get_mean();
                        else       lon = latlngalt[j].get_mean();
                    }
                }
                else
                    noLockTelemetry();
            }
            //
            // Parse NMEA GPS RMC string
            //
            if(nmeaParse("$GPRMC", 6, nmea, gprmc_positions, gprmc_tokens, gprmc_poscnt)) {
                // See NMEA message for format
                gtime = atof(gprmc_tokens[0]);
                status = gprmc_tokens[1][0];
                date = atoi(gprmc_tokens[2]);

                if(status == 'A' && locked != 0) {
                    igtime = (int) gtime;

                    struct tm t = {0};
                    // Year-1900, so DDMMYY (100 + YY) and (MM - 1)
                    t.tm_mday = date / 10000;
                    t.tm_mon = (date % 10000) / 100 - 1;
                    t.tm_year = 100 + (date % 100);

                    // hhmmss.sss
                    t.tm_hour = igtime / 10000;
                    t.tm_min  = (igtime % 10000) / 100;
                    t.tm_sec  = igtime % 100;
                    time = mktime(&t);
                    time = time * 1000 + (gtime - igtime) * 1000;
                }
                else
                    noLockTelemetry();
            }
        }
    } while(run);
}

bool GPS_UART::init() {
    if(run == true)
        return run;

    tty_fd = open(GPS_DEV, O_RDONLY | O_NOCTTY);
    if(tty_fd > -1) {
        run = true;
        std::thread(&GPS_UART::acquire, this).detach();
    }
    else
        fprintf(stderr, "Failed to open spatial device\n");

    return run;
}

bool GPS_UART::stop() {
    if(run == false)
        return !run;

    run = false;
    close(tty_fd);

    return !run;
}

double GPS_UART::getLongitude() {
    return lon;
}

double GPS_UART::getLatitude() {
    return lat;
}

double GPS_UART::getAltitude() {
    return alt;
}

uint64_t GPS_UART::getTime() {
    return time;
}

float GPS_UART::getDilution() {
    return dilution;
}

int GPS_UART::getSatelliteCount() {
    return sCount;
}
