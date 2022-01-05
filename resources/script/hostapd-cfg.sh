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

mac_address="/proc/device-tree/ocp/ethernet@4a100000/slave@4a100200/mac-address"
cpsw_0_mac=$(hexdump -v -e '1/1 "%02X" ":"' ${mac_address} | sed 's/.$//')
mac_0_prefix=$(echo ${cpsw_0_mac} | cut -c 1-14)
cpsw_0_6=$(echo ${cpsw_0_mac} | awk -F ':' '{print $6}')
cpsw_res=$(echo "obase=16;ibase=16;$cpsw_0_6 + 103" | bc)
cpsw_3_mac=${mac_0_prefix}:$(echo ${cpsw_res} | cut -c 2-3)

ssid_append=$(echo $cpsw_3_mac | cut -b 13-17 | sed 's/://g')
sed -i "s/AP-Soter-.\{4\}$/AP-Soter-${ssid_append}/gI" /etc/hostapd.conf