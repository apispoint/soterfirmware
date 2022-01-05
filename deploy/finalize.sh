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

_hlt=${1:-shutdown}
_dir=${2:-/tmp}

printf "\n\nCleaning source... "

sudo rm $_dir/*.c $_dir/*.cpp $_dir/*.h

printf "[ok]\n\n"

sh $_dir/secure.sh
sh $_dir/wait.sh

if [ $_hlt = "shutdown" ]; then
    sh $_dir/shutdown.sh
else
    sh $_dir/reboot.sh
fi