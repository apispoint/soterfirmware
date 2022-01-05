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
#ifndef MAG_LSM9DS1_H
#define MAG_LSM9DS1_H

#include "IMagnetometer.h"

class Mag_LSM9DS1 : public IMagnetometer {
public:
    Mag_LSM9DS1(short *offsets, int delay_ms = 1000);

    bool init();
    bool stop();

    double getX();
    double getY();
    double getZ();
    double getHeading();

    bool deviceOk();

private:
    void acquire();

    bool run;
    int fd;
    double x;
    double y;
    double z;
    double angle;
    int delay;
    short  *offset;
};

#endif     /* MAG_LSM9DS1_H */
