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
#ifndef TMPBARO_MPL115A2
#define TMPBARO_MPL115A2

#include "ITemperature.h"
#include "IPressure.h"

class TmpBaro_MPL115A2 : public ITemperature, public IPressure {
public:
    TmpBaro_MPL115A2();

    bool init();
    bool stop();

    double getTemperature();
    double getPressure();

private:
    void acquire();

    bool run;
    int fd;
    double temp;
    double pressure;
    float A0, B1, B2, C12;
};

#endif     /* TMPBARO_MPL115A2 */
