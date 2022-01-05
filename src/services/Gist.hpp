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
#ifndef GIST_HPP
#define GIST_HPP

#include <string>

#include "Telemetry.hpp"

using namespace std;

template <class T> class Gist : public Telemetry<T> {
public:
    Gist();
    ~Gist();

    void setData(T* data, size_t len);
    void setOffset(size_t offset);
    void setMagX(double mx);
    void setMagY(double my);
    void setMagZ(double mz);
    void setMagOffsets(short* magOffsets);
    void setAFRadius(double radius);
    void setSPS(int samplesPerSecond);

    T* getData(size_t *len);
    size_t getOffset();
    double getMagX();
    double getMagY();
    double getMagZ();
    short* getMagOffsets();
    double getAFRadius();
    int getSPS();

private:
    T* _data;
    size_t _len;
    double _magx, _magy, _magz;
    short* _magOffsets;
    double _radius;
    int _sps;
    size_t _offset;
};

template <class T> Gist<T>::Gist() : Telemetry<T>() {
    _len = 0;
    _radius = 0;
    _sps = 0;
}

template <class T> Gist<T>::~Gist() {
    delete [] _data;
}

template <class T> void Gist<T>::setData(T* data, size_t len) {
    _data = data;
    _len = len;
}

template <class T> void Gist<T>::setOffset(size_t offset) {
    _offset = offset;
}

template <class T> void Gist<T>::setMagX(double mx) {
    _magx = mx;
}

template <class T> void Gist<T>::setMagY(double my) {
    _magy = my;
}

template <class T> void Gist<T>::setMagZ(double mz) {
    _magz = mz;
}

template <class T> void Gist<T>::setMagOffsets(short* magOffsets) {
    _magOffsets = magOffsets;
}

template <class T> void Gist<T>::setAFRadius(double radius) {
    _radius = radius;
}

template <class T> void Gist<T>::setSPS(int sps) {
    _sps = sps;
}

template <class T> T* Gist<T>::getData(size_t *len) {
    *len = _len;
    return _data;
}

template <class T> size_t Gist<T>::getOffset() {
    return _offset;
}

template <class T> double Gist<T>::getMagX() {
    return _magx;
}

template <class T> double Gist<T>::getMagY() {
    return _magy;
}

template <class T> double Gist<T>::getMagZ() {
    return _magz;
}

template <class T> short* Gist<T>::getMagOffsets() {
    return _magOffsets;
}

template <class T> double Gist<T>::getAFRadius() {
    return _radius;
}

template <class T> int Gist<T>::getSPS() {
    return _sps;
}

#endif     /* GIST_HPP */
