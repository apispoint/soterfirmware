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

#include <chrono>
#include <climits>
#include <string>
#include <thread>

#include "BatchQueue.hpp"

#include "IMicArray.h"
#include "AM335X_ADC_Sensor.h"
#include "GPS_UART.h"
#include "Mag_LSM9DS1.h"

#if REF_HW_VER==1
    #include "Tmp_TMP102.h"
#else
    #include "TmpBaro_MPL115A2.h"
#endif

#ifndef ADC_SPS_SCALAR
    #define ADC_SPS_SCALAR 0.8829
#endif

using namespace std;

BatchQueue<short *> read_queue(false);

inline uint64_t getTimeOfDay() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
}

void DoTask(IMicArray *dev, int rec_secs) {
    fprintf(stderr, "GO!");
    fflush(stderr);

    dev->read(&read_queue);

    sleep(rec_secs);

    dev->stop();
    fprintf(stderr, "\nStopped\n");
}

int main(int argc, char** argv) {
    if(argc != 5 && argc != 6) {
        printf("Usage: recgist <hid> <offset_x> <offset_y> <offset_z> [seconds]\n\n");
        return -1;
    }

    string HW_ID_STRING(argv[1]);

    short offsets[3] = {
        (short) atoi(argv[2]),
        (short) atoi(argv[3]),
        (short) atoi(argv[4]),
    };

    int rec_secs = argc == 6 ? atoi(argv[5]) : DELAY;

    Mag_LSM9DS1 mag(offsets);
    GPS_UART gps;

#if REF_HW_VER==1
    Tmp_TMP102 tmp;
#else
    TmpBaro_MPL115A2 tmp;
#endif

    mag.init();
    gps.init();
    tmp.init();

    fprintf(stderr, "Acquiring telemetry...");
    fflush(stderr);

    uint64_t dev_time = getTimeOfDay();

    sleep(5);

    double *gps_info = gps.getPosition();
    uint64_t gps_time = gps.getTime();

    double x = mag.getX();
    double y = mag.getY();
    double z = mag.getZ();
    double a = mag.getHeading();

    double t = tmp.getTemperature();

    mag.stop();
    gps.stop();
    tmp.stop();

    fprintf(stderr, "done");
    fflush(stderr);

    IMicArray *dev = new AM335X_ADC();

    int SPS_C = dev->channelSampleRate() * ADC_SPS_SCALAR;
    int CHANNELS = dev->channels();
    int DATA_LEN = CHANNELS * dev->channelReadRate();

    dev->init();

    fprintf(stderr, "\nReady...");
    fflush(stderr);

    std::thread thrd = std::thread(DoTask, dev, rec_secs);
    thrd.join();

    // Telemetry
    printf(
    "{"
    "\"id\":\"%s\","
    "\"channels\":%d,"
    "\"offset\":%d,"
    "\"spschannel\":%d,"
    "\"radius\":%.3f,"
    "\"time\":%lld,"

    "\"temperature\":%.2f,"

    "\"longitude\":%.6f,"
    "\"latitude\":%.6f,"
    "\"altitude\":%.2f,"
    "\"locationTime\":%lld,"

    "\"heading\":%.1f,"
    "\"magScalars\":[%.1f,%.1f,%.1f],"
    "\"magOffsets\":[%.1f,%.1f,%.1f],",

    HW_ID_STRING.c_str(), CHANNELS, GIST_LEAD_LEN, SPS_C, AF_RADIUS, dev_time,
    t,
    gps_info[0], gps_info[1], gps_info[2], gps_time,
    a, x, y, z, offsets[0], offsets[1], offsets[2]
    );
    fflush(stdout);

    // Data
    printf("\"data\":[");
    fflush(stdout);

    int size = read_queue.size();
    int samps = size * (DATA_LEN / CHANNELS);

    short** data_channel = new short*[CHANNELS];
    for(int i = 0; i < CHANNELS; i++)
        data_channel[i] = new short[samps];

    int c = 0;
    short *data;
    while((data = read_queue.request()) != NULL) {
        for(int i = 0; i < DATA_LEN; i++) {
            data_channel[i % CHANNELS][c] = data[i];
            c += (i + 1) % CHANNELS == 0 ? 1 : 0;
        }
        delete [] data;
    }

    for(int i = 0; i < CHANNELS; i++) {
        printf("\n[");
        for(int j = 0; j < samps; j++)
            printf("%d%s", data_channel[i][j], j < samps - 1 ? "," : "");
        printf("%s", i < CHANNELS - 1 ? "]," : "]");
        fflush(stdout);
    }

    printf("]}");
    fflush(stdout);

    return 0;
}
