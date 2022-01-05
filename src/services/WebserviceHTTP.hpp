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
#ifndef WEBSERVICEHTTP_HPP
#define WEBSERVICEHTTP_HPP

#include <cstring>
#include <string>
#include <curl/curl.h>

#include "CurlInit.hpp"

using namespace std;

template <class T> class WebserviceHTTP {
public:
    WebserviceHTTP(BatchQueue<T> *QUEUE, string URL);

    void start();
    void stop();
    bool post(string params, char* data, size_t len, T src, string mime = "application/octet-stream");

    virtual void process(T job) = 0;

private:
    BatchQueue<T> *queue;
    string url;
    volatile bool running;

    void processJob() {
        do {
            T job = queue->request();
            process(job);
        } while(running == true);
    }

    static size_t writefunc(void *ptr, size_t size, size_t nmemb, std::string *s) {
        return CURLE_WRITE_ERROR;
    }

    void retry(T job) {
#ifdef DEBUGTELEMETRY
        printf("%8s> HTTP_RTRY: %d\n", "ID", job->getSequenceID());
#endif
        std::this_thread::sleep_for(std::chrono::seconds(30));
        queue->submit(job);
    }

};

template <class T> WebserviceHTTP<T>::WebserviceHTTP(BatchQueue<T> *QUEUE, string URL) {
    CurlInit::init();
    url = URL;
    queue = QUEUE;
    running = false;
}

template <class T> void WebserviceHTTP<T>::start() {
    if(running == true)
        return;

    running = true;
    std::thread(&WebserviceHTTP::processJob, this).detach();
}

template <class T> void WebserviceHTTP<T>::stop() {
    if(running == false)
        return;

    running = false;
}

template <class T> bool WebserviceHTTP<T>::post(string params, char *data, size_t len, T src, string mime) {
    CURLcode code(CURLE_FAILED_INIT);
    CURL* curl = curl_easy_init();
    bool status = false;

    if(curl) {
        struct curl_slist* headers = NULL;
        string ct("Content-Type: " + mime);
        headers = curl_slist_append(headers, ct.c_str());

        if(CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_POST, 1L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, (url + params).c_str()))
        )
        {
            code = curl_easy_perform(curl);

            long http_code = 0;
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

            status =
                http_code >= 200 &&
                http_code < 300 &&
                code != CURLE_ABORTED_BY_CALLBACK &&
                (CURLE_OK == code);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    if(status == false)
        std::thread(&WebserviceHTTP::retry, this, src).detach();

    return status;
}

#endif     /* WEBSERVICEHTTP_HPP */
