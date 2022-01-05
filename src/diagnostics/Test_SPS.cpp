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
#include <iostream>

#include <time.h>
#include <unistd.h>

#include "BatchQueue.hpp"
#include "IMicArray.h"
#include "AM335X_ADC_Sensor.h"

#define SETTLE_TIME_SECS 2

using namespace std;

int main(int argc, char** argv) {
    int acq_period_secs = argc == 2 ? atoi(argv[1]) : SETTLE_TIME_SECS;

    BatchQueue<short *> read_queue;
    IMicArray *dev = new AM335X_ADC();

    long CHANNELS = dev->channels();
    long SPS_C    = dev->channelSampleRate();
    long DATA_LEN = dev->channelReadRate() * CHANNELS;
    long SPS_T    = SPS_C * CHANNELS;

    cout << "Theoretical:"             << endl;
    cout << "\tCHANNELS: " << CHANNELS << endl;
    cout << "\t   SPS_T: " <<    SPS_T << endl;
    cout << "\t   SPS_C: " <<    SPS_C << endl;

    dev->init();
    dev->read(&read_queue);

    sleep(1);

    while(read_queue.empty() == true) {
        dev->stop();
        dev->init();
        dev->read(&read_queue);

        sleep(1);
    }

    cout << "\nReady..." << flush;

    uint64_t init_sec = 0;
    uint64_t stop_sec = 0;
    uint64_t blocks = 0;

    struct timespec timer;

    cout << "Go!..." << flush;

    read_queue.clear();

    clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
    init_sec = timer.tv_sec + SETTLE_TIME_SECS;
    stop_sec = init_sec + acq_period_secs;

    do {
        read_queue.request();

        clock_gettime(CLOCK_MONOTONIC_RAW, &timer);

        if(timer.tv_sec >= init_sec)
            blocks++;

    } while(timer.tv_sec < stop_sec);

    dev->stop();
    cout << "Stopped\n\n" << flush;

    int sps_t  = (blocks * DATA_LEN) / (acq_period_secs + timer.tv_nsec * 1e-9);
    int sps_c  = (blocks * DATA_LEN) / (acq_period_secs + timer.tv_nsec * 1e-9) / CHANNELS;

    cout << "Actual:"                       << endl;
    cout << "\tBLOCKS: " << blocks          << endl;
    cout << "\tPERIOD: " << acq_period_secs << endl;
    cout << "\t SPS_T: " << sps_t           << endl;
    cout << "\t SPS_C: " << sps_c           << endl;

    return 0;
}
