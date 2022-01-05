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
#include <stdlib.h>
#include <unistd.h>

#include "Mag_LSM9DS1.h"

int main(int argc,char** argv)
{
    if(argc != 4) {
        printf("Usage: mag <offset_x> <offset_y> <offset_z>\n\n");
        return -1;
    }

    short offsets[3] = {
        (short) atoi(argv[1]),
        (short) atoi(argv[2]),
        (short) atoi(argv[3]),
    };

    Mag_LSM9DS1 mag(offsets, 75);
    bool ok = mag.init();

    if(!ok)
        return -1;

    sleep(5);

    double x = mag.getX();
    double y = mag.getY();
    double z = mag.getZ();
    double a = mag.getHeading();
    printf("x: %lf, y: %lf, z: %lf, angle: %lf\n", x, y, z, a);

    mag.stop();

    return 0;
}
