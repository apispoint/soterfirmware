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
#ifndef IMAGNETOMETER_H
#define IMAGNETOMETER_H

#include "ISensor.h"

class IMagnetometer : ISensor {
public:
    virtual bool init() = 0;
    virtual bool stop() = 0;

    virtual double getX()       = 0;
    virtual double getY()       = 0;
    virtual double getZ()       = 0;
    virtual double getHeading() = 0;

    virtual bool deviceOk() = 0;
};

#endif     /* IMAGNETOMETER_H */
