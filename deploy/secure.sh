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

_secure="${1:-secure}"

if [ $_secure = "unsecure" ]
  then
    printf "\nUnsecuring...\n\n"

    users="root debian"
    for user in $users
    do
        passwd -d $user
    done

    sed -i -e 's:#\?PermitRootLogin \(prohibit-password\|without-password\|no\):PermitRootLogin yes:g' /etc/ssh/sshd_config
    sed -i -e 's:#\?PermitEmptyPasswords no:PermitEmptyPasswords yes:g' /etc/ssh/sshd_config
    sed -i -e 's:UsePAM yes:UsePAM no:g' /etc/ssh/sshd_config

    #
    # For (manual) post-jump insecuring
    # sed -i -e 's:#\?PasswordAuthentication no:PasswordAuthentication yes:g' /etc/ssh/sshd_config
else
    printf "\nSecuring...\n\n"

    sed -i -e 's:PermitRootLogin yes:PermitRootLogin no:g' /etc/ssh/sshd_config
    sed -i -e 's:PermitEmptyPasswords yes:PermitEmptyPasswords no:g' /etc/ssh/sshd_config
    sed -i -e 's:#\?PasswordAuthentication yes:PasswordAuthentication no:g' /etc/ssh/sshd_config
    sed -i -e 's:UsePAM no:UsePAM yes:g' /etc/ssh/sshd_config
    sed -i -e 's:Subsystem sftp /usr/lib/openssh/sftp-server:#Subsystem sftp /usr/lib/openssh/sftp-server:g' /etc/ssh/sshd_config
fi

sudo -S /etc/init.d/ssh restart