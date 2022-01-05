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
#ifndef STALTA_HPP
#define STALTA_HPP

#include <cmath>
#include <limits>

#include "SlidingWindow.hpp"

template <class T, size_t M, size_t N> class STALTA {
public:
    STALTA();

    void add(T* data, size_t len);
    double get_ratio();

private:
    SlidingWindow<T, M> sta;
    SlidingWindow<T, N> lta;

    T sum_of_sqs_sta;
    T sum_of_sqs_lta;
    T data_sq;
    double ratio, nan;
};

template <class T, size_t M, size_t N> STALTA<T, M, N>::STALTA() {
    ratio = sum_of_sqs_lta = sum_of_sqs_sta = 0.0;
    nan = std::numeric_limits<double>::quiet_NaN();
}

template <class T, size_t M, size_t N> void STALTA<T, M, N>::add(T* data, size_t len) {
    for(int i = 0; i < len; i++) {
        if(sta.size() == M)
            sum_of_sqs_sta -= sta.pop();
        if(lta.size() == N)
            sum_of_sqs_lta -= lta.pop();

        data_sq = data[i] * data[i];
        sta.push(&data_sq, 1);
        lta.push(&data_sq, 1);
        sum_of_sqs_sta += data_sq;
        sum_of_sqs_lta += data_sq;
        ratio = sqrt(sum_of_sqs_sta) / sqrt(sum_of_sqs_lta);
    }
}

template <class T, size_t M, size_t N> double STALTA<T, M, N>::get_ratio() {
    return lta.size() < N ? nan : ratio;
}

#endif     /* STALTA_HPP */
