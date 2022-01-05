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

cd "$(dirname "$0")"

sh pinouts-cfg.sh

cpufreq-set -r -g performance
export AP_HOME=/opt/apispoint
export LD_LIBRARY_PATH=$AP_HOME/soter

#
# Ensure Soter Wi-Fi is online
#
while [ `curl -s http://192.168.8.1:3001/ | jq .ok` != "true" ]; do
    sleep 5
done

#
# Ensure NTP is synchronized
#
systemctl restart ntp

#
# Ensure Soter device is user / owner provisioned
#
while [ ! -f "${AP_HOME}/config/token.sub" ]; do
    sleep 5
done

#
# Check global prohibit (TRANQUILITY protocol)
#
prohibit_status() {
    local status=`curl -s https://api-soter.apispoint.com/config | jq '.prohibit' 2>/dev/null`
    echo ${status:-0}
}

while [ $(prohibit_status) -eq 1 ]; do
    sleep 60
done

dmp=`hexdump -e '8/1 "%c"' /sys/bus/i2c/devices/0-0050/eeprom -s 16 -n 12`
arr=`jq -r '.mag_offsets | map(tostring) | join(" ")' $AP_HOME/config/mag_calibration.json 2>/dev/null`
hid=${dmp:-"__UNKNOWN__"}
mag=${arr:-"0 0 0"}
sub=`cat $AP_HOME/config/token.sub`
nice -n -20 $AP_HOME/soter/soter $hid $mag $sub