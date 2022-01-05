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
#ifndef IMICARRAY_H
#define IMICARRAY_H

#include "BatchQueue.hpp"
#include "ISensor.h"

class IMicArray : ISensor {
public:
    static const int GROUP_BY_SCAN_NUMBER = 1;
    static const int GROUP_BY_CHANNELS    = 2;

    virtual bool init() = 0;
    virtual bool stop() = 0;

    virtual int grouping()          = 0;
    virtual int channels()          = 0;
    virtual int channelSampleRate() = 0;
    virtual int channelReadRate()   = 0;

    virtual void read(void (*cb)(double *, int)) {}
    virtual void read(void (*cb)(int *, int)) {}
    virtual void read(void (*cb)(short *, int)) {}
    virtual void read(BatchQueue<short *> *queue) {}
};

#endif     /* IMICARRAY_H */
