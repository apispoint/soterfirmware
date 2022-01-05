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

AP_HOME=/opt/apispoint
UPD_URL="https://soter-update.apispoint.com"

echo "Checking for updates..."

CURR_SW=`curl --connect-timeout 5 -s 192.168.8.1:3004 | jq -r ".sw" 2>/dev/null`

if [ $? -ne 0 ] || [ "$CURR_SW" = "BAD" ]
then
  echo "Not updated: Invalid introspection"
  exit 1
fi

_PKG=`curl -s $UPD_URL/MANIFEST.json | jq -r --arg CURR_SW $CURR_SW '.[$CURR_SW]' 2>/dev/null`

if [ $? -ne 0 ] || [ -z "$_PKG" ] || [ "$_PKG" = "null" ]
then
  echo "Your device is up to date - $CURR_SW"
  exit 0
fi

echo "Downloading..."

SIG_B64=/tmp/$_PKG.sig.b64
SIG_BIN=/tmp/$_PKG.sig.bin
BIN_LOC=/tmp/$_PKG.tar.gz

curl -o $SIG_B64 $UPD_URL/$_PKG.sig.b64 2>/dev/null
SIG_CURL_OK=$?

curl -o $BIN_LOC $UPD_URL/$_PKG.tar.gz 2>/dev/null

if [ $? -ne 0 ] || [ $SIG_CURL_OK -ne 0 ]
then
  echo "Not updated: Invalid get"
  exit 3
fi

echo "Verifying..."

openssl base64 -d -in $SIG_B64 -out $SIG_BIN 2>/dev/null
openssl dgst -sha256 -verify $AP_HOME/appmgr/dev-soter-appmgr.pub -signature $SIG_BIN $BIN_LOC > /dev/null 2>&1
VERIFIED_OK=$?

if [ $VERIFIED_OK -ne 0 ]
then
  echo "Not updated: Invalid signature"
  exit 4
fi

echo "Installing..."

rm -rf $AP_HOME/${_PKG}

tar -C $AP_HOME -xzf $BIN_LOC 2>/dev/null

if [ $? -ne 0 ]
then
  echo "Not updated: Invalid extract"
  exit 5
fi

echo "Extras..."

INSTALL_SCRIPT=$AP_HOME/${_PKG}/install.sh
if [ -f $INSTALL_SCRIPT ]; then
    . $INSTALL_SCRIPT
fi

echo "Finalizing..."

rm -rf $AP_HOME/soter
rm -rf $AP_HOME/provision
rm -rf $AP_HOME/appmgr

ln -s $AP_HOME/${_PKG}/soter $AP_HOME/soter
ln -s $AP_HOME/${_PKG}/provision $AP_HOME/provision
ln -s $AP_HOME/${_PKG}/appmgr $AP_HOME/appmgr

BUILDS=(`ls -d -1 /opt/apispoint/[1-9]*`)
IFS=$'\n' SORTED_BUILDS=($(sort <<<"${BUILDS[*]}")); unset IFS
SORT_BUILDS_LEN=${#SORTED_BUILDS[@]}

if [ $SORT_BUILDS_LEN -ne 2 ]
then
  for (( i=1; i<$SORT_BUILDS_LEN-1; i++ ))
  do
    rm -rf $SORTED_BUILDS[$i]
  done
fi

( sleep 5 ; reboot ) &

echo "Your device was updated successfully"
exit 0
