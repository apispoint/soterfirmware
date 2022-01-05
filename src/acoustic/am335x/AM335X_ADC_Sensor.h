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
#ifndef AM335X_ADC_SENSOR_H
#define AM335X_ADC_SENSOR_H

#include "am335x_adc.h"
#include "tscadc.h"

#include "IMicArray.h"
#include "BatchQueue.hpp"

class AM335X_ADC : public IMicArray {
public:
    AM335X_ADC();

    bool init();
    bool stop();

    int grouping();
    int channels();
    int channelSampleRate();
    int channelReadRate();

    void read(BatchQueue<short *> *queue);

private:
    void process();
    BatchQueue<short *> *batchqueue;

    //
    // FIFO_THRESHOLD must not exceed 64 which is the maximum number of
    // samples either FIFO0 or FIFO1 can contain
    //
    static const int FIFO_THRESHOLD = 64;
    int BUFFERLEN;

    //
    // Stopping limits
    //
    int attempt;
    const int retries = 3;

    int fd;
    void *map;
    unsigned int *cmdMem;

    enum state { created, inited, running, stopping, stopped };
    state pru_state = created;
};

#endif     /* AM335X_ADC_SENSOR_H */
