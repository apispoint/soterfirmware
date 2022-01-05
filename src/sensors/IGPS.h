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
#ifndef IGPS_H
#define IGPS_H

#include <cstdint>

#include "ISensor.h"

class IGPS : ISensor {
public:
    virtual bool init() = 0;
    virtual bool stop() = 0;

    virtual double   getLongitude() = 0;
    virtual double   getLatitude()  = 0;
    virtual double   getAltitude()  = 0;
    virtual uint64_t getTime()      = 0;

    //
    // [lon, lat, alt]
    //
    double* getPosition() {
        double *pos = new double[3];
        pos[0] = getLongitude();
        pos[1] = getLatitude();
        pos[2] = getAltitude();
        return pos;
    }
};

#endif     /* IGPS_H */
