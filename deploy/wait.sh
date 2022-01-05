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

#
# Ensure Soter Wi-Fi or port is operational
#

_svc=${1:-port}
_prt=${2:-22}

_name=Port
_test="ss -tl4 '( sport = :${_prt} )' | grep LISTEN >/dev/null 2>&1"

if [ $_svc = "wifi" ]; then
    _name=Wi-Fi
    _test="ping -c 1 www.google.com >/dev/null 2>&1"
fi

printf "\n\nWaiting for $_name "

while : ; do
    sleep 1
    $(eval $_test)
    [ `echo $?` -ne 0 ] || break
    printf "."
done

printf " [ok]\n\n"