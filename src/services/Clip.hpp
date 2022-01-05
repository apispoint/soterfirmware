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
#ifndef CLIP_HPP
#define CLIP_HPP

#include <string>

#include "Telemetry.hpp"

using namespace std;

template <class T> class Clip : public Telemetry<T> {
public:
    Clip();
    ~Clip();

    void setCount(size_t cnt);
    void setDuration(size_t duration);
    void setTuple(size_t tup);
    void setData(T* data, size_t len);

    size_t getCount();
    size_t getDuration();
    size_t getTuple();
    T* getData(size_t *len);

private:
    T* _data;
    size_t _len, _tup, _cnt, _duration;
};

template <class T> Clip<T>::Clip() : Telemetry<T>() {}

template <class T> Clip<T>::~Clip() {
    delete [] _data;
}

template <class T> void Clip<T>::setCount(size_t cnt) {
    _cnt = cnt;
}
template <class T> void Clip<T>::setDuration(size_t duration) {
    _duration = duration;
}
template <class T> void Clip<T>::setTuple(size_t tup) {
    _tup = tup;
}
template <class T> void Clip<T>::setData(T* data, size_t len) {
    _data = data;
    _len = len;
}

template <class T> size_t Clip<T>::getCount() {
    return _cnt;
}
template <class T> size_t Clip<T>::getDuration() {
    return _duration;
}
template <class T> size_t Clip<T>::getTuple() {
    return _tup;
}
template <class T> T* Clip<T>::getData(size_t *len) {
    *len = _len;
    return _data;
}

#endif     /* CLIP_HPP */
