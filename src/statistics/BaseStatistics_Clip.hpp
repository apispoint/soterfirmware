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
#ifndef BASE_STATISTICS_CLIP_HPP
#define BASE_STATISTICS_CLIP_HPP

#include <algorithm>
#include <cmath>
#include <limits>

#include "BaseStatistics.hpp"

template <class T> class BaseStatistics_Clip : public BaseStatistics<T> {
public:
    BaseStatistics_Clip(/*T maxValue = 1*/);

    void add(T* data, size_t len);

    T get_min();
    T get_max();
    double get_rms();
    size_t get_crosses();
    void clear();

protected:
    int i;
    size_t n, zcr_cnt;
    T min, max, prev, sumsqs;
};

template <class T> BaseStatistics_Clip<T>::BaseStatistics_Clip(/*T maxValue*/) : BaseStatistics<T>() {
    zcr_cnt = prev = n = sumsqs = 0;
    min =  std::numeric_limits<T>::max();
    max = -std::numeric_limits<T>::max();
}

template <class T> void BaseStatistics_Clip<T>::add(T* data, size_t len) {
    for(i = 0; i < len; i++) {
        BaseStatistics<T>::add(&data[i], 1);

        if(data[i] * prev < 0 && n)
            zcr_cnt++;
        prev = data[i];

        n++;
        sumsqs += data[i] * data[i];

        if(data[i] < min) min = data[i];
        if(data[i] > max) max = data[i];
    }
}

template <class T> T BaseStatistics_Clip<T>::get_min() {
    return min;
}

template <class T> T BaseStatistics_Clip<T>::get_max() {
    return max;
}
template <class T> double BaseStatistics_Clip<T>::get_rms() {
    return sqrt((double) (sumsqs / n));
}

template <class T> size_t BaseStatistics_Clip<T>::get_crosses() {
    return zcr_cnt;
}

template <class T> void BaseStatistics_Clip<T>::clear() {
    BaseStatistics<T>::clear();
    zcr_cnt = prev = n = sumsqs = 0;
    min =  std::numeric_limits<T>::max();
    max = -std::numeric_limits<T>::max();
}

#endif     /* BASE_STATISTICS_CLIP_HPP */
