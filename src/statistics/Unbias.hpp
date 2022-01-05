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
#ifndef UNBIAS_HPP
#define UNBIAS_HPP

#include <cstddef>

template <class T > class Unbias {
public:
    Unbias(long shiftedfilter = -10000);

    void unbias(T* data, T* unbiased, size_t len);

private:
    bool initialize;
    long prev, shifted_filter, shiftedFCL;
};

template <class T> Unbias<T>::Unbias(long shiftedfilter) {
    initialize = true;
    shifted_filter = shiftedfilter;
    prev = 0;
}

template <class T> void Unbias<T>::unbias(T* data, T* unbiased, size_t len) {
    for(int i = 0; i < len; i++) {
        shiftedFCL = shifted_filter + ((data[i] - prev) << 8);
        shifted_filter = shiftedFCL - (shiftedFCL >> 8);
        unbiased[i] = (T) ((shifted_filter + 128) >> 8);
        prev = data[i];
    }
}

#endif     /* UNBIAS_HPP */
