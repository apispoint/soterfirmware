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
#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <cstdint>
#include <string>

using namespace std;

template <class T> class Telemetry {
public:
    static const string UNK_DEV;

    Telemetry();
    ~Telemetry();

    void setTemperature(double temp);
    void setLocation(double lon, double lat, double amsl);
    void setLocationTime(uint64_t time);
    void setTime(uint64_t t);
    void setID(string id);
    void setTK(string sub);
    void setChannels(size_t channels);
    void setHeading(double heading);

    double getTemperature();
    double* getLocation();
    uint64_t getLocationTime();
    uint64_t getTime();
    string getID();
    string getTK();
    size_t getChannels();
    double getHeading();

#ifdef DEBUGTELEMETRY
    int getSequenceID();
    void setSequenceID(int seq);
#endif

private:
    double _temp;
    double* _loc;
    uint64_t _gpstime;
    uint64_t _t;
    string _id, _sub;
    size_t _channels;
    double _heading;

#ifdef DEBUGTELEMETRY
    int _seq;
#endif

};

template <class T> const string Telemetry<T>::UNK_DEV = "__UNKNOWN__";

template <class T> Telemetry<T>::Telemetry() {
    _loc = new double[3];
    fill_n(_loc, 3, 0);
    _t = time(NULL);
    _id = UNK_DEV;
}

template <class T> Telemetry<T>::~Telemetry() {
    delete [] _loc;
}

template <class T> void Telemetry<T>::setTemperature(double temp) {
    _temp = temp;
}

template <class T> void Telemetry<T>::setLocation(double lon, double lat, double amsl) {
    _loc[0] = lon;
    _loc[1] = lat;
    _loc[2] = amsl;
}

template <class T> void Telemetry<T>::setLocationTime(uint64_t time) {
    _gpstime = time;
}

template <class T> void Telemetry<T>::setTime(uint64_t t) {
    _t = t;
}

template <class T> void Telemetry<T>::setID(string id) {
    _id = id;
}

template <class T> void Telemetry<T>::setTK(string sub) {
    _sub = sub;
}

template <class T> void Telemetry<T>::setChannels(size_t channels) {
    _channels = channels;
}

template <class T> void Telemetry<T>::setHeading(double heading) {
    _heading = heading;
}

template <class T> double Telemetry<T>::getTemperature() {
    return _temp;
}

template <class T> double* Telemetry<T>::getLocation() {
    return _loc;
}

template <class T> uint64_t Telemetry<T>::getLocationTime() {
    return _gpstime;
}

template <class T> uint64_t Telemetry<T>::getTime() {
    return _t;
}

template <class T> string Telemetry<T>::getID() {
    return _id;
}

template <class T> string Telemetry<T>::getTK() {
    return _sub;
}

template <class T> size_t Telemetry<T>::getChannels() {
    return _channels;
}

template <class T> double Telemetry<T>::getHeading() {
    return _heading;
}

#ifdef DEBUGTELEMETRY
    template <class T> int Telemetry<T>::getSequenceID() {
        return _seq;
    }
    template <class T> void Telemetry<T>::setSequenceID(int seq) {
        _seq = seq;
    }
#endif

#endif     /* TELEMETRY_HPP */
