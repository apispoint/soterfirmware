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

if [[ ! -z "$1" && ! -z "$2" && ! -z "$3" ]]
then
  rm /var/lib/connman/*.config > /dev/null 2>&1
else
  mv /var/lib/connman/soterwifi.config /var/lib/connman/soterwifi.config.back > /dev/null 2>&1
fi

#
# Ensure Soter Wi-Fi is auto-removed
#
count=0
while [ `ls /var/lib/connman | grep ^wifi > /dev/null 2>&1;echo $?` -eq 0 ] && [ $count -lt 5 ]; do
    sleep 1
    count=$((count+1))
done

rm -rf /var/lib/connman/wifi_* > /dev/null 2>&1

if [[ ! -z "$1" && ! -z "$2" && ! -z "$3" ]]
then
  printf "[service_soterwifi]\nType=wifi\nSSID=${1}\nSecurity=${2}\nPassphrase=${3}" > /var/lib/connman/soterwifi.config
else
  mv /var/lib/connman/soterwifi.config.back /var/lib/connman/soterwifi.config > /dev/null 2>&1
fi

#
# Ensure Soter Wi-Fi is readied
#
get_wificonfig() {
    local status=`ls /var/lib/connman | grep ^wifi`
    echo ${status:-1}
}

count=0
while : ; do
    wificonfig=$(get_wificonfig)
    [[ wificonfig -eq 1 && $count -lt 5 ]] || break
    sleep 1
    count=$((count+1))
done

#
# Ensure Soter Wi-Fi is readied
#
count=0
while [ `connmanctl scan wifi > /dev/null 2>&1; connmanctl services | grep "${wificonfig}" > /dev/null 2>&1; echo $?` -ne 0 ] && [ $count -lt 5 ]; do
    sleep 1
    count=$((count+1))
done

connmanctl connect ${wificonfig} > /dev/null 2>&1

#
# Ensure Soter Wi-Fi is online
#
count=0
while [ `curl -s http://192.168.8.1:3001/ | jq .ok` != "true" ] && [ $count -lt 5 ]; do
    sleep 1
    count=$((count+1))
done

[ `curl -s http://192.168.8.1:3001/ | jq .ok` != "false" ] && systemctl restart ntp
