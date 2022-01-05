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

export BASE_IMAGE=`date +"%y%j"`

while getopts ":u:p:n:K:P:U:I:t:" opt; do
  case $opt in
    u) _usr="$OPTARG"
    ;;
    p) _pwd="$OPTARG"
    ;;
    n) _net="$OPTARG"
    ;;
    K) _key="$OPTARG"
    ;;
    P) _pem="$OPTARG"
    ;;
    A) _pub="$OPTARG"
    ;;
    U) _USR="$OPTARG"
    ;;
    I) _IPA="$OPTARG"
    ;;
    t) _tmp="$OPTARG"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    ;;
  esac
done

_usr=${_usr:-debian}
_pwd=${_pwd:-temppwd}
_net=${_net:-~/.ssh/dev-soter-wifi.config}
_key=${_key:-~/.ssh/dev-soter/dev-soter.pub}
_pem=${_pem:-~/.ssh/dev-soter/dev-soter}
_pub=${_pub:-~/.ssh/dev-soter/dev-soter-appmgr.pub}
_USR=${_USR:-root}
_IPA=${_IPA:-192.168.8.1}
_tmp=${_tmp:-/tmp}

# Delete stale host finger print

sed -i '' /"${_IPA}"/d ~/.ssh/known_hosts

# Seed build scripts in TMP

scp \
    finalize.sh \
    image_bbb.sh \
    reboot.sh \
    secure.sh \
    shutdown.sh \
    wait.sh \
$_usr@$_IPA:$_tmp

# Ready the SOM for jump (unsecure users, add sudoers, and seed public key)

ssh $_usr@$_IPA "echo $_pwd | sudo -S sh -c 'echo "debian ALL=NOPASSWD: ALL" >>/etc/sudoers'"
ssh -t $_usr@$_IPA "sudo sh $_tmp/secure.sh unsecure;sh $_tmp/wait.sh"

# Remove extraneous bin directory from debian user

ssh -t $_usr@$_IPA "rmdir ~/bin"

# ssh-copy-id needs to be done before removing passwords with secure.sh unsecure
#ssh-copy-id -f -i $_key $_usr@$_IPA
cat $_key | ssh $_usr@$_IPA 'umask 0077; mkdir .ssh; cat >> .ssh/authorized_keys && echo "Key copied"'

# Seed Wi-Fi configuration

scp $_net $_USR@$_IPA:/var/lib/connman
ssh $_USR@$_IPA "sleep 5;systemctl restart connman"
ssh $_USR@$_IPA "sh $_tmp/wait.sh wifi"

# Begin the Jump

ssh $_USR@$_IPA "sh $_tmp/image_bbb.sh $BASE_IMAGE"
sh codejump.sh $_USR $_IPA $_pub

# Clean up and signal device reboot

ssh $_USR@$_IPA "rm /var/lib/connman/${_net##*/}"
ssh -i $_pem $_USR@$_IPA "nohup sh -c 'sh $_tmp/finalize.sh' >/dev/null 2>&1 &"

printf "\n\nEt tu, Brute? Then fall Caesar!\n\n\n"