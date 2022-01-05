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
#ifndef CLIP_SERVICE_HPP
#define CLIP_SERVICE_HPP

#include <thread>

#include "WebserviceHTTP.hpp"
#include "Clip.hpp"
#include "BatchQueue.hpp"

using namespace std;

template <class T> class ClipService : public WebserviceHTTP<T> {
public:
    ClipService(BatchQueue<T> *queue, string URL);

    void process(T clip);
};

template <class T> ClipService<T>::ClipService(BatchQueue<T> *queue, string URL) : WebserviceHTTP<T>(queue, URL) {}

template <class T> void ClipService<T>::process(T clip) {
    size_t datalen;
    double* data = clip->getData(&datalen);
    double* loc = clip->getLocation();

    datalen *= sizeof(*data);

    string params = "?";
    params +=
        "gps_lon="      + to_string(loc[0])                  + "&" +
        "gps_lat="      + to_string(loc[1])                  + "&" +
        "gps_alt="      + to_string(loc[2])                  + "&" +
        "gps_time="     + to_string(clip->getLocationTime()) + "&" +

        "mag_angle="    + to_string(clip->getHeading())      + "&" +
        "tmp_degree="   + to_string(clip->getTemperature())  + "&" +

        "clp_tuple="    + to_string(clip->getTuple())        + "&" +
        "clp_count="    + to_string(clip->getCount())        + "&" +
        "clp_duration=" + to_string(clip->getDuration())     + "&" +

        "dev_time="     + to_string(clip->getTime())         + "&" +
        "dev_channels=" + to_string(clip->getChannels())     + "&" +
        "dev_tk="       + clip->getTK()                      + "&" +

        "dev_id="       + clip->getID()
#ifdef DEBUGTELEMETRY
        + "$" + to_string(clip->getSequenceID())
#endif
        ;

    if(WebserviceHTTP<T>::post(params, (char*) data, datalen, clip)) {
#ifdef DEBUGTELEMETRY
        printf("%8s> Clip_SENT: %d\n", "ClipService", clip->getSequenceID());
#endif
        delete clip;
    }
#ifdef DEBUGTELEMETRY
    else
        printf("%8s> Clip_RTRY: %d\n", "ClipService", clip->getSequenceID());
#endif
}

#endif     /* CLIP_SERVICE_HPP */
