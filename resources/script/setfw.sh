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

AP_HOME={{AP_HOME}}
FW_HOME=$AP_HOME/{{FW_NAME}}

SOTER_NAME={{SOTER_NAME}}
PROVISION_NAME={{PROVISION_NAME}}
APPMGR_NAME={{APPMGR_NAME}}

AP_SOTER_HOME=$AP_HOME/$SOTER_NAME
AP_PROVISION_HOME=$AP_HOME/$PROVISION_NAME
AP_APPMGR_HOME=$AP_HOME/$APPMGR_NAME

SOTER_HOME=$FW_HOME/$SOTER_NAME
PROVISION_HOME=$FW_HOME/$PROVISION_NAME
APPMGR_HOME=$FW_HOME/$APPMGR_NAME

rm $AP_SOTER_HOME
rm $AP_PROVISION_HOME
rm $AP_APPMGR_HOME

ln -s $SOTER_HOME $AP_SOTER_HOME
ln -s $PROVISION_HOME $AP_PROVISION_HOME
ln -s $APPMGR_HOME $AP_APPMGR_HOME

if [ "$1" != "NOREBOOT" ]; then
    ( sleep 5 ; reboot ) &
fi

echo "Your device firmware was restored"
exit 0