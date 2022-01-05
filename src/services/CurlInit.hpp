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
#ifndef CURL_INIT_HPP
#define CURL_INIT_HPP

#include <mutex>
#include <string>
#include <thread>
#include <curl/curl.h>
#include <openssl/err.h>

using namespace std;

struct CurlInit {

    static CurlInit* init();
    void close();

private:
    static std::mutex* aMutex;
    static CurlInit* instance;

    CurlInit(){};
    CurlInit(const CurlInit& src){};
    CurlInit& operator=(const CurlInit& rhs){};

    static void locking_function(int mode, int n, const char* file, int line)
    {
        if(mode & CRYPTO_LOCK)
            CurlInit::aMutex[n].lock();
        else
            CurlInit::aMutex[n].unlock();
    }

    static unsigned long id_function()
    {
        return (unsigned long) std::hash<std::thread::id>() (std::this_thread::get_id());
    }

    int thread_setup();
    int thread_cleanup();
};

CurlInit* CurlInit::instance = NULL;
std::mutex* CurlInit::aMutex = NULL;

CurlInit* CurlInit::init() {
    if(!CurlInit::instance) {
        CurlInit::instance = new CurlInit();
        curl_global_init(CURL_GLOBAL_ALL);
        CurlInit::instance->thread_setup();
    }

    return CurlInit::instance;
}

void CurlInit::close() {
    CurlInit::instance->thread_cleanup();
}

int CurlInit::thread_setup()
{
    CurlInit::aMutex = new std::mutex[CRYPTO_num_locks()];
    if(!CurlInit::aMutex)
        return 0;

    CRYPTO_set_id_callback(CurlInit::id_function);
    CRYPTO_set_locking_callback(CurlInit::locking_function);

    return 1;
}

int CurlInit::thread_cleanup()
{
    if(!CurlInit::aMutex)
        return 0;

    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);

    curl_global_cleanup();

    delete[] CurlInit::aMutex;
    CurlInit::aMutex = NULL;
    delete CurlInit::instance;
    CurlInit::instance = NULL;

    return 1;
}

#endif     /* CURL_INIT_HPP */
