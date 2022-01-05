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
#ifndef PROVISION_SERVICE_HPP
#define PROVISION_SERVICE_HPP

#include <chrono>
#include <curl/curl.h>
#include <fstream>
#include <jansson.h>
#include <iostream>
#include <string>
#include <thread>

#include "Configuration.hpp"
#include "CurlInit.hpp"

#ifndef PROV_DELAY_MS
    #define PROV_DELAY_MS 60000
#endif

#ifndef PROV_FILENAME
    #define PROV_FILENAME ".conf.json"
#endif

using namespace std;

class ProvisionService {
public:
    ProvisionService(string URL, string devid);

    void getConfiguration(void (*cb)(Configuration*));

private:
    string url;
    bool running;

    void (*callback)(Configuration*);
    void pollConfig();

    static void writefunc(void *ptr, size_t size, size_t nmemb, std::string *s) {
        s->append((const char *) ptr);
    }
    Configuration* curlConfiguration();

    static void writeConfig(string config) {
        if(!config.empty()) {
            ofstream out(PROV_FILENAME);
            out << config;
            out.close();
        }
    }

    static Configuration* readConfig() {
        Configuration *config = NULL;

        ifstream infile(PROV_FILENAME);
        if(infile.is_open()) {
            string json, line;

            while(getline(infile, line))
                json.append(line);
            infile.close();

            // Create configuration
            config = createConfiguration(json);
        }

        if(config == NULL) {
            config = new Configuration(true);
        }

        return config;
    }

    static Configuration* createConfiguration(string str) {
        if(str.empty())
            return NULL;

        json_t *root;
        json_error_t error;
        root = json_loads(str.c_str(), 0, &error);

        Configuration *config = NULL;

        if(root) {
            json_t
                *version,
                *detectors,
                *clipduration,
                *clipcount,
                *amplitudelimit,
                *prohibit,
                *mute,
                *value;

            version        = json_object_get(root, "version");
            detectors      = json_object_get(root, "detectors");
            clipduration   = json_object_get(root, "clipduration");
            clipcount      = json_object_get(root, "clipcount");
            amplitudelimit = json_object_get(root, "amplitudelimit");
            prohibit       = json_object_get(root, "prohibit");
            mute           = json_object_get(root, "mute");

            int i;
            config = new Configuration();
            config->detectors_size = json_array_size(detectors);
            config->detectors = new int[config->detectors_size];
            json_array_foreach(detectors, i, value)
                config->detectors[i] = json_integer_value(value);

            config->version        = json_integer_value(version);
            config->clipduration   = json_integer_value(clipduration);
            config->clipcount      = json_integer_value(clipcount);
            config->amplitudelimit = json_real_value(amplitudelimit);
            config->prohibit       = json_integer_value(prohibit);
            config->mute           = json_integer_value(mute);

            config->config_str.append(str);
        }

        json_decref(root);

        return config;
    }
};

ProvisionService::ProvisionService(string URL, string devid) {
    CurlInit::init();
    url = URL.append("/").append(devid);
    running = false;
}

void ProvisionService::getConfiguration(void (*cb)(Configuration*)) {
    if(running)
        return;

    callback = cb;
    std::thread(&ProvisionService::pollConfig, this).detach();

    running = true;
}

Configuration* ProvisionService::curlConfiguration() {
    CURL *curl;

    Configuration *config = NULL;

    curl = curl_easy_init();
    if(curl) {
        string str;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
        //
        // TODO Update to verify HOST/PEER
        //
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        /*CURLcode code = */curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        config = createConfiguration(str);
    }

    return config;
}

void ProvisionService::pollConfig() {
    Configuration *config = readConfig();
    int version = config->version;
    callback(config);

    while(running) {
        config = curlConfiguration();

        if(config != NULL &&
                  ((config->version > 0 && config->version > version)
               ||  (config->version < 1 && config->version < version))
          ) {
            version = config->version;
            writeConfig(config->config_str);
            callback(config);
        }
        else
            delete config;

        std::this_thread::sleep_for(std::chrono::milliseconds(PROV_DELAY_MS));
    }
}

#endif     /* PROVISION_SERVICE_HPP */
