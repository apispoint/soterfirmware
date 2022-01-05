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

#include "BatchQueue.hpp"
#include "IMicArray.h"
#include "AM335X_ADC_Sensor.h"

int main(int argc, char** argv) {
    int rec_secs = argc == 2 ? atoi(argv[1]) : DELAY;

    BatchQueue<short *> read_queue(false);
    IMicArray *dev = new AM335X_ADC();

    int CHANNELS = dev->channels();
    int DATA_LEN = CHANNELS * dev->channelReadRate();

    dev->init();
    dev->read(&read_queue);

    sleep(1);

    while(read_queue.empty() == true) {
        dev->stop();
        dev->init();
        dev->read(&read_queue);

        sleep(1);
    }

    fprintf(stderr, "\nReady...");
    fflush(stderr);

    fprintf(stderr, "GO!");
    fflush(stderr);

    dev->read(&read_queue);

    sleep(rec_secs);

    dev->stop();
    fprintf(stderr, "\nStopped\n");

    short *data;
    while((data = read_queue.request()) != NULL) {
        for(int i = 0; i < DATA_LEN; i++) {
            printf("%d", data[i]);
            if((i + 1) % CHANNELS == 0) {
                printf("\n");
                fflush(stdout);
            }
            else printf(",");
        }
    }

    return 0;
}
