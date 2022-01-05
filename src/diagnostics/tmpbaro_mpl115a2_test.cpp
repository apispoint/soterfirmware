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

#include "TmpBaro_MPL115A2.h"

int main(int argc,char** argv)
{
    TmpBaro_MPL115A2 tmpbaro;
    bool ok = tmpbaro.init();

    if(!ok)
        return -1;

    sleep(5);

    printf("TempC: %lf\n", tmpbaro.getTemperature());
    printf("kPa: %lf\n", tmpbaro.getPressure());

    tmpbaro.stop();

    return 0;
}
