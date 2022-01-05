#!/bin/bash

#
#    Copyright (c) 2021 APIS Point, LLC.
#    All rights reserved.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, version 3 of the License.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

sh pinouts-cfg.sh

cpufreq-set -r -g performance
export AP_HOME=/opt/apispoint
export LD_LIBRARY_PATH=$AP_HOME/soter

dmp=`hexdump -e '8/1 "%c"' /sys/bus/i2c/devices/0-0050/eeprom -s 16 -n 12`
arr=`jq -r '.mag_offsets | map(tostring) | join(" ")' $AP_HOME/config/mag_calibration.json 2>/dev/null`
hid=${dmp:-"__UNKNOWN__"}
mag=${arr:-"0 0 0"}
out=`date '+%y%m%d_%H%M%S'`
nice -n -20 $AP_HOME/soter/recgist REC-$hid $mag $1 > ${out}_${hid}.rec