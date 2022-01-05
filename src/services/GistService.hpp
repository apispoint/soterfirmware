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
#ifndef GIST_SERVICE_HPP
#define GIST_SERVICE_HPP

#include <arpa/inet.h>
#include <thread>

#include "WebserviceHTTP.hpp"
#include "Gist.hpp"
#include "BatchQueue.hpp"

using namespace std;

template <class T> class GistService : public WebserviceHTTP<T> {
public:
    GistService(BatchQueue<T> *queue, string URL);

    void process(T gist);
};

template <class T> GistService<T>::GistService(BatchQueue<T> *queue, string URL) : WebserviceHTTP<T>(queue, URL) {}

template <class T> void GistService<T>::process(T gist) {
    size_t datalen;
    short* data = gist->getData(&datalen);
    double* loc = gist->getLocation();
    short* mag_offsets = gist->getMagOffsets();

    //
    // Convert to network byte order
    //
    for(int i = 0; i < datalen; i++)
        data[i] = htons(data[i]);

    datalen *= sizeof(*data);

    string params = "?";
    params +=
        "gps_lon="      + to_string(loc[0])                  + "&" +
        "gps_lat="      + to_string(loc[1])                  + "&" +
        "gps_alt="      + to_string(loc[2])                  + "&" +
        "gps_time="     + to_string(gist->getLocationTime()) + "&" +

        "tmp_degree="   + to_string(gist->getTemperature())  + "&" +

        "mag_angle="    + to_string(gist->getHeading())      + "&" +
        "mag_x="        + to_string(gist->getMagX())         + "&" +
        "mag_y="        + to_string(gist->getMagY())         + "&" +
        "mag_z="        + to_string(gist->getMagZ())         + "&" +
        "mag_offset_x=" + to_string(mag_offsets[0])          + "&" +
        "mag_offset_y=" + to_string(mag_offsets[1])          + "&" +
        "mag_offset_z=" + to_string(mag_offsets[2])          + "&" +

        "dev_offset="   + to_string(gist->getOffset())       + "&" +
        "dev_radius="   + to_string(gist->getAFRadius())     + "&" +
        "dev_sps_c="    + to_string(gist->getSPS())          + "&" +
        "dev_time="     + to_string(gist->getTime())         + "&" +
        "dev_channels=" + to_string(gist->getChannels())     + "&" +
        "dev_tk="       + gist->getTK()                      + "&" +

        "dev_id="       + gist->getID()
#ifdef DEBUGTELEMETRY
        + "$" + to_string(gist->getSequenceID())
#endif
        ;

    if(WebserviceHTTP<T>::post(params, (char *) data, datalen, gist)) {
#ifdef DEBUGTELEMETRY
        printf("%8s> Gist_SENT: %d\n", "GistService", gist->getSequenceID());
#endif
        delete gist;
    }
#ifdef DEBUGTELEMETRY
    else
        printf("%8s> Gist_RTRY: %d\n", "GistService", gist->getSequenceID());
#endif
}

#endif     /* GIST_SERVICE_HPP */
