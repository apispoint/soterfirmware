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
#ifndef BASE_STATISTICS_HPP
#define BASE_STATISTICS_HPP

#include <cmath>

template <class T> class BaseStatistics {
public:
    BaseStatistics();

    void     add(T * data, size_t len);
    double   get_variance() const;
    double   get_std_dev() const;
    double   get_mean() const;
    uint64_t size() const;
    void     clear();

protected:
    uint64_t m_n;
    size_t i;
    double m_oldM, m_newM, m_oldS, m_newS;
};

template <class T> BaseStatistics<T>::BaseStatistics() {
    m_n = 0;
}

template <class T> void BaseStatistics<T>::add(T* data, size_t len) {
    for(i = 0; i < len; i++) {
        m_n++;

        //
        // TAOCP Volume 2, 3rd edition, p.232, D. Knuth
        // Welford's Method, 1962
        //

        if (m_n == 1) {
            m_oldM = m_newM = data[i];
            m_oldS = 0.0;
        } else {
            m_newM = m_oldM + (data[i] - m_oldM) / m_n;
            m_newS = m_oldS + (data[i] - m_oldM) * (data[i] - m_newM);

            // set up for next iteration
            m_oldM = m_newM;
            m_oldS = m_newS;
        }
    }
}

template <class T> double BaseStatistics<T>::get_variance() const {
    return (m_n > 1) ? m_newS / (m_n - 1) : 0.0;
}

template <class T> double BaseStatistics<T>::get_std_dev() const {
    return sqrt(get_variance());
}

template <class T> double BaseStatistics<T>::get_mean() const {
    return (m_n > 0) ? m_newM : 0.0;
}

template <class T> uint64_t BaseStatistics<T>::size() const {
    return m_n;
}

template <class T> void BaseStatistics<T>::clear() {
    m_n = 0;
}

#endif     /* BASE_STATISTICS_HPP */
