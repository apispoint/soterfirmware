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
#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>

struct Configuration {
    Configuration(bool setDefaultDetector = false) {
        version        = 0;
        clipduration   = 10000;
        clipcount      = 6;
        amplitudelimit = 0.15;
        prohibit       = 0;
        mute           = 0;

        if(setDefaultDetector == true) {
            detectors_size = 1;
            detectors      = new int[detectors_size];
            std::fill_n(detectors, detectors_size, 0);
        }
    }

    ~Configuration() {
        delete [] detectors;
    }

    //
    // Given constants from config
    //
    int    version;
    int*   detectors;
    int    detectors_size;
    int    clipduration;
    int    clipcount;
    double amplitudelimit;
    int    prohibit;
    int    mute;

    //
    // Derived constants / variables
    //
    std::string config_str;
};

#endif     /* CONFIGURATION_HPP */
