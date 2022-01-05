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
#ifndef BATCH_QUEUE_HPP
#define BATCH_QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <deque>

template <class T> class BatchQueue {
public:
    BatchQueue(bool blocking = true);

    void submit(T job);
    T request();
    void clear();
    size_t size() const;
    bool empty() const;

private:
    bool _blocking;
    std::deque<T> queue;
    std::condition_variable cv;
    std::mutex mutex;
};

template <class T> BatchQueue<T>::BatchQueue(bool blocking) {
    _blocking = blocking;
}

template <class T> void BatchQueue<T>::submit(T job) {
    std::unique_lock<std::mutex> mlock(mutex);
    queue.push_back(job);
    mlock.unlock();
    cv.notify_one();
}

template <class T> T BatchQueue<T>::request() {
    std::unique_lock<std::mutex> mlock(mutex);

    if(queue.empty() == true && _blocking == false)
        return NULL;

    while(queue.empty() == true)
        cv.wait(mlock);

    auto job = queue.front();
    queue.pop_front();

    return job;
}

template <class T> void BatchQueue<T>::clear() {
    std::unique_lock<std::mutex> mlock(mutex);

    queue.clear();

    mlock.unlock();
    cv.notify_one();
}

template <class T> size_t BatchQueue<T>::size() const {
    return queue.size();
}

template <class T> bool BatchQueue<T>::empty() const {
    return queue.empty();
}

#endif     /* BATCH_QUEUE_HPP */
