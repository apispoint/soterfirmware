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
#ifndef SLIDING_WINDOW_HPP
#define SLIDING_WINDOW_HPP

#include <cstddef>
#include <cstring>

template <class T, size_t N> class SlidingWindow {
public:
    SlidingWindow();

    void push(T* data, size_t len);
    T pop();
    void clear();
    T front() const;
    size_t size() const;
    bool empty() const;
    size_t array(T *array);

protected:
    size_t sizeoft;
    size_t _head;
    size_t _tail;
    size_t _size;
    T queue[N];
    T headpop;
};

template <class T, size_t N> SlidingWindow<T, N>::SlidingWindow() {
    sizeoft = sizeof(T);
    _size = _head = _tail = 0;
}

template <class T, size_t N> void SlidingWindow<T, N>::push(T* data, size_t len) {
    for(int i = 0; i < len; i++) {
        if(_size == N)
            pop();
        queue[_tail] = data[i];
        _tail = (_tail + 1) % N;
        _size++;
    }
}

template <class T, size_t N> T SlidingWindow<T, N>::pop() {
    headpop = queue[_head];
    if(_size > 0) {
        _head = (_head + 1) % N;
        _size--;
    }
    return headpop;
}

template <class T, size_t N> T SlidingWindow<T, N>::front() const {
    return queue[_head];
}

template <class T, size_t N> size_t SlidingWindow<T, N>::size() const {
    return _size;
}

template <class T, size_t N> bool SlidingWindow<T, N>::empty() const {
    return _size == 0;
}

template <class T, size_t N> void SlidingWindow<T, N>::clear() {
    _size = _head = _tail = 0;
}

template <class T, size_t N> size_t SlidingWindow<T, N>::array(T *array) {
    if(_size == 0) {
        return 0;
    }

    if(_head < _tail) {
        for(int i = _tail - 1; i > _head; i--)
            array[i] = queue[i];
    }
    else {
        memcpy(&array[0],         &queue[_tail], sizeoft * (N - _tail));
        memcpy(&array[N - _tail], &queue[0],     sizeoft * _tail);
    }

    return _size;
}

#endif     /* SLIDING_WINDOW_HPP */
