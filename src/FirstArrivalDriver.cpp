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
#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <thread>

#include <time.h>

#include "BaseStatistics_Clip.hpp"
#include "BatchQueue.hpp"
#include "Clip.hpp"
#include "Configuration.hpp"
#include "ClipService.hpp"
#include "GistService.hpp"
#include "ProvisionService.hpp"
#include "Gist.hpp"
#include "SlidingWindow.hpp"
#include "STALTA.hpp"
#include "Unbias.hpp"

#include "IGPS.h"
#include "IMagnetometer.h"
#include "ITemperature.h"
#include "IMicArray.h"

#define ADC_NORMALIZING_SCALAR 2048.0
#define POST_FA_POWER_BIN_SIZE 16     // Number of samples used to determine
                                      // the power following the FA detection

IGPS          *gps;
IMagnetometer *mag;
ITemperature  *tmp;
IMicArray     *dev;

volatile int SPS_C;

#ifdef SENSOR_NULL
    #include "Sensor_NULL.hpp"
#elif SENSOR_CSV
    #include "Sensor_CSV.hpp"
#else
    #include "AM335X_ADC_Sensor.h"
#endif

#ifdef GPS_TEST
    #include "GPSTest.hpp"
#else
    #include "GPS_UART.h"
#endif

#ifdef MAG_TEST
    #include "MagTest.hpp"
#else
    #include "Mag_LSM9DS1.h"
#endif

#ifdef TMP_TEST
    #include "TmpTest.hpp"
#else
    #include "TmpBaro_MPL115A2.h"
#endif

//
// Passed in arguments
//
string HW_ID_STRING (Gist<void*>::UNK_DEV);
string HW_TK_STRING ("");
short *mag_offsets = new short[3];

BatchQueue<Configuration *> confqueue(false);
BatchQueue<short *>         infilqueue;
BatchQueue<Gist<short> *>   afqueue;
BatchQueue<Clip<double> *>  cpqueue;
BatchQueue<Gist<short> *>   exfilqueue_gist;
BatchQueue<Clip<double> *>  exfilqueue_clip;

//
// Configuration
//
Configuration *config = NULL;
Configuration *tmpcon = NULL;

#if defined (DEBUGDETECT)
    int c = 1;
#endif

#ifdef DEBUGTELEMETRY
    int gist_seq = 1;
    int clip_seq = 1;
#endif

//
// Configuration callback
//
void configuration_cb(Configuration* c) {
    // Collapse configurations (aka Drain to latest)
#ifdef DEBUGCONFIG
    printf("\nConfig RECEIVED\n");
#endif
    while(confqueue.empty() == false)
        delete confqueue.request();
    confqueue.submit(c);
}

void applyConfiguration() {
    tmpcon = confqueue.request();
    if(tmpcon != NULL) {
        delete config;
        config = tmpcon;
    }
}

//
// Settling block
//
#ifndef SETTLE_MS
    #define SETTLE_MS 1000
#endif

volatile bool settle = false;

void settle_thread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(SETTLE_MS));
    settle = false;
}

//
// The ADC WATCHDOG softly resets the ADC subsystem until the ADC is operational
// and begins sequencing.
//
void watchdog_adc_reset() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    while(infilqueue.empty() == true) {
#ifdef DEBUGWATCHDOG
        printf("WATCHDOG_ADC_RESET\n");
#endif
        dev->stop();
        dev->init();
        dev->read(&infilqueue);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
#ifdef DEBUGWATCHDOG
    printf("WATCHDOG_ADC_COMPLETE\n");
#endif
}

//
// Clip control block
//
volatile bool SIGCLIP = false;

void clipcontrol_thread() {
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(config->clipduration));
        SIGCLIP = true;
    } while(true);
}

const int CLIP_TUPLE = 6;
void clip_thread() {
    const int DEV_AINCNT = dev->channels();

    Clip<double> *clip;

    ClipService<Clip<double> *> exfilClip(&exfilqueue_clip, CLIP_INGEST_URI);
    exfilClip.start();

    while(true) {
        clip = cpqueue.request();

        if(!config->mute) {
            clip->setLocation(
                    gps->getLongitude(),
                    gps->getLatitude(),
                    gps->getAltitude());

            clip->setTemperature(tmp->getTemperature());
            clip->setHeading(mag->getHeading());

            clip->setTuple(CLIP_TUPLE);

            clip->setChannels(DEV_AINCNT);
            clip->setID(HW_ID_STRING);
            clip->setTK(HW_TK_STRING);

#ifdef DEBUGTELEMETRY
            printf("%8s> Clip_PUSH: %d\n", "FAD", clip_seq);
            clip->setSequenceID(clip_seq);
            clip_seq++;
#endif

#ifndef DEBUGTUPLE
            exfilqueue_clip.submit(clip);
#else
            delete clip;
#endif
        }
        else
            delete clip;
    }
}

void interferometry_thread() {
    const int DEV_AINCNT = dev->channels();

    Gist<short> *gist;
    short *data;
    size_t dataLen;

    GistService<Gist<short> *> exfilGist(&exfilqueue_gist, GIST_INGEST_URI);
    exfilGist.start();

    int i;

    while(true) {
        gist = afqueue.request();

        if(!config->mute) {
            data = gist->getData(&dataLen);

            gist->setLocation(
                    gps->getLongitude(),
                    gps->getLatitude(),
                    gps->getAltitude());

            gist->setTemperature(tmp->getTemperature());

            gist->setHeading(mag->getHeading());
            gist->setMagX(mag->getX());
            gist->setMagY(mag->getY());
            gist->setMagZ(mag->getZ());
            gist->setMagOffsets(mag_offsets);

            gist->setOffset(GIST_LEAD_LEN);
            gist->setAFRadius(AF_RADIUS);
            gist->setSPS(SPS_C);
            gist->setChannels(DEV_AINCNT);
            gist->setID(HW_ID_STRING);
            gist->setTK(HW_TK_STRING);

#ifdef DEBUGTELEMETRY
            printf("%8s> Gist_PUSH: %d\n", "FAD", gist_seq);
            gist->setSequenceID(gist_seq);
            gist_seq++;
#endif

            exfilqueue_gist.submit(gist);
        }
        else
            delete gist;
    }
}

inline uint64_t getTimeOfDay() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
}

void processing_thread() {
    const int PEAK_SEED       = -1;
    const int DEV_AINCNT      = dev->channels();
    const int BUFFER_LEN      = DEV_AINCNT * dev->channelReadRate();
    const int SHIFT_COUNT     = (GIST_LEN - GIST_LEAD_LEN) * DEV_AINCNT;
    const int GIST_PACKET_LEN = GIST_LEN * DEV_AINCNT + DEV_AINCNT;

    int CLIP_PACKET_LEN = CLIP_TUPLE * config->clipcount * DEV_AINCNT;

    Unbias<short>                                  *unb    = new Unbias<short>[DEV_AINCNT];
    SlidingWindow<short, GIST_LEN>                 *trace  = new SlidingWindow<short, GIST_LEN>[DEV_AINCNT];
    STALTA<double, STALTA_STA_LEN, STALTA_LTA_LEN> *stalta = new STALTA<double, STALTA_STA_LEN, STALTA_LTA_LEN>[DEV_AINCNT];
    BaseStatistics_Clip<double>                    *clips  = new BaseStatistics_Clip<double>[DEV_AINCNT];

    short *data;
    short *gistData;

    short channel;
    int shift_pos, i, j;
    bool detected = false;
    uint64_t devDetectedTime;
    uint64_t gpsDetectedTime;
    Gist<short> *gist;

    int AINCNT_M_1 = DEV_AINCNT - 1;
    uint64_t devClipsTime = getTimeOfDay();
    uint64_t gpsClipsTime = gps->getTime();
    int clipsCount = 0;
    int clipsIndex;
    double *clipsData = new double[CLIP_PACKET_LEN];
    Clip<double> *clip;

    short unbData;
    double normData, unbData_d, ratioVal;
    double pwr_cumsum = 0.0;

    std::thread(clipcontrol_thread).detach();

    //
    // Dynamic SPS_C variables
    //
    uint64_t next_sec = 0;
    uint64_t time_nsec = 0;
    uint64_t blocks = 0;

    struct timespec timer;

    clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
    next_sec = timer.tv_sec++;

    infilqueue.clear();
    data = infilqueue.request();

    do {
        //
        // Dynamic SPS_C calculation
        //
        // BEGIN HEAD OF PROCESSING LOOP
        //
        blocks++;

        clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
        time_nsec = timer.tv_nsec; // After clock for increased accuracy

        if(next_sec < timer.tv_sec) {
            SPS_C = (blocks * BUFFER_LEN) / (1 + time_nsec * 1e-9) / DEV_AINCNT;
            next_sec++;
            blocks = 0;
#ifdef DEBUGSPS
            printf("SPS_C: %d\n", SPS_C);
#endif
        }
        //
        // END HEAD OF PROCESSING LOOP
        //

        for(i = 0; i < BUFFER_LEN; i++) {
            channel = i % DEV_AINCNT;

            //
            // Unbias and normalize the data between [ -1, 1 )
            //
            unb[channel].unbias(&data[i], &unbData, 1);

            unbData_d = unbData;
            normData = unbData_d / ADC_NORMALIZING_SCALAR;

            //
            // Add unbiased data to trace
            //
            trace[channel].push(&unbData, 1);

            //
            // Calculate First Arrival break point
            //
            stalta[channel].add(&unbData_d, 1);
            ratioVal = stalta[channel].get_ratio();

            //
            // Clip data
            //
            clips[channel].add(&normData, 1);
            if(channel == AINCNT_M_1 && SIGCLIP == true) {
                SIGCLIP = false;
                for(j = 0; j < DEV_AINCNT; j++) {
                    clipsIndex =
                                CLIP_TUPLE * j +
                                CLIP_TUPLE * clipsCount * DEV_AINCNT;

#ifndef DEBUGTUPLETEST
                    clipsData[clipsIndex + 0] = clips[j].get_min();
                    clipsData[clipsIndex + 1] = clips[j].get_max();
                    clipsData[clipsIndex + 2] = clips[j].get_mean();
                    clipsData[clipsIndex + 3] = clips[j].get_rms();
                    clipsData[clipsIndex + 4] = clips[j].get_crosses();
                    clipsData[clipsIndex + 5] = clips[j].get_variance();
#else
                    //
                    // 100 * (j+1) + 10 * clipsCount + {TUPLE_IDX};
                    //
                    clipsData[clipsIndex + 0] = 100 * (j+1) + 10 * clipsCount + 0;
                    clipsData[clipsIndex + 1] = 100 * (j+1) + 10 * clipsCount + 1;
                    clipsData[clipsIndex + 2] = 100 * (j+1) + 10 * clipsCount + 2;
                    clipsData[clipsIndex + 3] = 100 * (j+1) + 10 * clipsCount + 3;
                    clipsData[clipsIndex + 4] = 100 * (j+1) + 10 * clipsCount + 4;
                    clipsData[clipsIndex + 5] = 100 * (j+1) + 10 * clipsCount + 5;
#endif

#ifdef DEBUGTUPLE
                    printf("%f, %f, %f, %f, %f, %f, %f, %f, %f, %f%s",
                        clipsData[clipsIndex + 0],
                        clipsData[clipsIndex + 1],
                        clipsData[clipsIndex + 2],
                        clipsData[clipsIndex + 3],
                        clipsData[clipsIndex + 4],
                        clipsData[clipsIndex + 5],
                        0.0,//reserved
                        0.0,//reserved
                        0.0,//reserved
                        0.0,//reserved
                        (j == AINCNT_M_1) ? "\n\n" : "\n");
#endif

                    clips[j].clear();
                }
                clipsCount++;

                if(clipsCount == config->clipcount) {
                    //
                    // Construct a partial Clip
                    //
                    clip = new Clip<double>();
                    clip->setData(clipsData, CLIP_PACKET_LEN);

                    clip->setLocationTime(gpsClipsTime);

                    clip->setCount(config->clipcount);
                    clip->setDuration(config->clipduration);

                    clip->setTime(devClipsTime);

                    cpqueue.submit(clip);

                    //
                    // Apply configuration
                    //
                    applyConfiguration();

                    //
                    // Reset
                    //
                    CLIP_PACKET_LEN = CLIP_TUPLE * config->clipcount * DEV_AINCNT;
                    clipsData = new double[CLIP_PACKET_LEN];
                    clipsCount = 0;
                    gpsClipsTime = gps->getTime();
                    devClipsTime = getTimeOfDay();
                }
            }

#ifdef DEBUG
            printf("%1.4f%c", ratioVal, ((channel + 1) % DEV_AINCNT == 0 ? '\n' : ','));
#endif

            if(detected == false && settle == false && ratioVal >= STALTA_R_LIMIT) {
#ifdef DEBUGDETECT
                printf("\nDETECTED-FA >>> SAMPLE: %d CHANNEL: %d VALUE: %d >>> ", c, (channel + 1), data[i]);
#endif
                devDetectedTime = getTimeOfDay();
                gpsDetectedTime = gps->getTime();
                shift_pos = 0;
                pwr_cumsum = 0;
                detected = true;
            }

            if(detected == true) {
                shift_pos++;

                //
                // Stop processing the candidate FA if there is not enough
                // power after the first break
                //
                pwr_cumsum += abs(normData);
                if(shift_pos == POST_FA_POWER_BIN_SIZE && (pwr_cumsum / POST_FA_POWER_BIN_SIZE) < config->amplitudelimit) {
#ifdef DEBUGDETECT
                    printf("TRIGGERED - INSUFFICIENT POWER\n");
#endif
                    detected = false;
                }

                if(shift_pos == SHIFT_COUNT) {
#ifdef DEBUGDETECT
                    printf("TRIGGERED\n");
#endif
                    gistData = new short[GIST_PACKET_LEN];

                    for(j = 0; j < DEV_AINCNT; j++) {
                        trace[j].array(&gistData[DEV_AINCNT + (GIST_LEN * j)]);
                        gistData[j] = 0;//peakPos[j] - peakPos[0] - (j > 0 ? 1 : 0);
                    }

#ifdef DEBUGDATA
                    printf("LAGS: ");
                    for(j = 0; j < DEV_AINCNT; j++)
                        printf("%d%c", gistData[j], (j < (DEV_AINCNT - 1) ? ',' : '\n'));
                    for(int j = 0, k = DEV_AINCNT; j < DEV_AINCNT; j++) {
                        printf("channel: %d\n", j);
                        for(; k < GIST_LEN * (j + 1) + DEV_AINCNT; k++)
                            printf("%d%c", gistData[k], (k < (GIST_LEN * (j + 1) + DEV_AINCNT - 1) ? ',' : '\n'));
                    }
#endif

                    //
                    // Construct a partial Gist
                    //
                    gist = new Gist<short>();
                    gist->setData(gistData, GIST_PACKET_LEN);

                    gist->setLocationTime(gpsDetectedTime);

                    gist->setTime(devDetectedTime);

                    afqueue.submit(gist);
                    settle = true;
                    std::thread(settle_thread).detach();
                    detected = false;
                }
            }
#if defined (DEBUGDETECT)
            if((i + 1) % DEV_AINCNT == 0)
                c++;
#endif
        }

        delete [] data;
        data = infilqueue.request();
    } while(true);
}

void sequencing_thread() {
    if(DURATION <= 0)
        while(!config->prohibit)
            std::this_thread::sleep_for(std::chrono::seconds(60));
    else
        std::this_thread::sleep_for(std::chrono::seconds(DURATION));
}

int main(int argc, char **argv) {
    if(argc == 6) {
        HW_ID_STRING.assign(argv[1]);
        mag_offsets[0] = (short) atoi(argv[2]);
        mag_offsets[1] = (short) atoi(argv[3]);
        mag_offsets[2] = (short) atoi(argv[4]);
        HW_TK_STRING.assign(argv[5]);
    }

    ProvisionService provision(PROVISION_URI, HW_ID_STRING);
    provision.getConfiguration(&configuration_cb);

#ifdef SENSOR_NULL
    dev = new Sensor_NULL();
#elif SENSOR_CSV
    dev = new Sensor_CSV();
#else
    dev = new AM335X_ADC();
#endif

#ifdef GPS_TEST
    gps = new GPSTest();
#else
    gps = new GPS_UART();
#endif

#ifdef MAG_TEST
    mag = new MagTest();
#else
    mag = new Mag_LSM9DS1(mag_offsets);
#endif

#ifdef TMP_TEST
    tmp = new TmpTest();
#else
    #if REF_HW_VER==1
        tmp = new Tmp_TMP102();
    #else
        tmp = new TmpBaro_MPL115A2();
    #endif
#endif

    gps->init();
    mag->init();
    tmp->init();
    dev->init();
    dev->read(&infilqueue);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        applyConfiguration();
    } while(config == NULL);

#ifdef DEBUGCONFIG
    printf("\nConfig Given:\n\tversion: %d\n\tclipduration: %d\n\tclipcount: %d\n\tamplitudelimit: %f\n\tprohibit: %d\n",
        config->version, config->clipduration, config->clipcount, config->amplitudelimit, config->prohibit);
#endif

    //
    // Interferometry thread
    //
    std::thread(interferometry_thread).detach();

    //
    // Clip thread
    //
    std::thread(clip_thread).detach();

    //
    // ADC Watchdog Reset (blocking call)
    //
    watchdog_adc_reset();

    //
    // Processing thread
    //
    std::thread prc_thrd(processing_thread);
    sched_param sch;
    sch.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(prc_thrd.native_handle(), SCHED_FIFO, &sch);
    prc_thrd.detach();

    //
    // Sequencing thread
    //
    std::thread seq_thrd = std::thread(sequencing_thread);
    seq_thrd.join();

    gps->stop();
    mag->stop();
    tmp->stop();
    dev->stop();

    return EXIT_SUCCESS;
}
