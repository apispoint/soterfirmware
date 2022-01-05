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
# Build a production sensor image. Use a BBB Debian console image
#

printf "\n\n********************\nImaging Jump\n********************\n\n\n"

export BASE_IMAGE=$1
export YY=`date +"%y"`
export COMPANY="APIS Point, LLC."
export COMPANYACRO="AP"
export COPYRIGHT="2014-20$YY"
export PRODUCT_NAME="Soter"

# Required for am335x-pru-package Debian 9
echo "deb [arch=armhf] http://repos.rcn-ee.com/debian/ jessie main" >> /etc/apt/sources.list

apt-get -y update
#apt-get -y upgrade

# Update kernel and uEnv for capes (--bone-rt-channel or --bone-channel)
#sh /opt/scripts/tools/update_kernel.sh --lts-4_14 --bone-rt-channel

# Deployment packages

apt-get -y install curl
apt-get -y install libssl1.0-dev
apt-get -y install libcurl4-openssl-dev
apt-get -y install cpufrequtils
apt-get -y install ntp
apt-get -y install libjansson-dev
apt-get -y install jq
apt-get -y install busybox
apt-get -y install am335x-pru-package

curl -sL https://deb.nodesource.com/setup_10.x | sudo -E bash -
apt-get -y install nodejs
#npm install http-server -g

# Development packages

apt-get -y install g++
apt-get -y install build-essential
apt-get -y install i2c-tools

#
# Additional development ports BB-UART2, BB-SPIDEV1
#
#printf "\ncape_enable=bone_capemgr.enable_partno=BB-UART4,BB-ADC,uio_pruss_enable" >> /boot/uEnv.txt
#printf "\ncape_disable=bone_capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN" >> /boot/uEnv.txt
sed -i 's/^uboot_overlay_pru=/#uboot_overlay_pru=/gI' /boot/uEnv.txt
sed -i 's/^#uboot_overlay_pru=\/lib\/firmware\/AM335X-PRU-UIO/uboot_overlay_pru=\/lib\/firmware\/AM335X-PRU-UIO/gI' /boot/uEnv.txt
sed -i 's/^#disable_uboot_overlay_video=1/disable_uboot_overlay_video=1/gI' /boot/uEnv.txt
sed -i 's/^#disable_uboot_overlay_audio=1/disable_uboot_overlay_audio=1/gI' /boot/uEnv.txt

prussblacklist=/etc/modprobe.d/pruss-blacklist.conf
if [ -f $prussblacklist ]; then
    rm $prussblacklist
else
    printf "\nrm $prussblacklist [skipped]\n\n"
fi

apt-get -y clean

# Update banner information

printf "\n$COMPANY\nCopyright (c) $COPYRIGHT\n\n$PRODUCT_NAME Build $BASE_IMAGE\n" > /etc/issue
printf "\nYou are accessing an $COMPANY ($COMPANYACRO) Information System (IS) that is
provided for ${COMPANYACRO}-authorized use only.

By using this IS (which includes any device attached to this IS), you consent to
the following conditions:

  - $COMPANYACRO routinely intercepts and monitors communications on this IS for
    purposes including, but not limited to, penetration testing,
    COMSEC monitoring, network operations and defense,
    personnel misconduct (PM), law enforcement (LE), and
    counterintelligence (CI) investigations.

  - At any time, $COMPANYACRO may inspect and seize data stored on this IS.

  - Communications using, or data stored on, this IS are not private, are
    subject to routine monitoring, interception, and search, and may be
    disclosed or used for any ${COMPANYACRO}-authorized purpose.

  - This IS includes security measures (e.g., authentication and access
    controls) to protect $COMPANYACRO interests—not for your personal benefit or
    privacy.

  - Notwithstanding the above, using this IS does not constitute consent to PM,
    LE, or CI investigative searching or monitoring of the content of privileged
    communications, or work product, related to personal representation or
    services by attorneys, psychotherapists, or clergy, and their assistants.
    Such communications and work product are private and confidential.
    See User Agreement for details.\n\n" >> /etc/issue
cp /etc/issue /etc/issue.net
cp /etc/issue /etc/motd

# Update host information

echo "soter" > /etc/hostname
sed -i 's/\(beaglebone\|arm\)/soter/gI' /etc/hosts

#
# Default AP configuration
#
#sed -i 's/beaglebone/AP-Soter/gI' /etc/default/bb-wl18xx

#
# Static AP configuration
#
sed -i 's/USE_GENERATED_HOSTAPD=.*/USE_GENERATED_HOSTAPD=no/gI' /etc/default/bb-wl18xx
cp /tmp/hostapd*.conf /etc/hostapd.conf
sed -i 's/beaglebone/AP-Soter/gI' /etc/hostapd.conf
sed -i 's/AP-Soter-.\{4\}$/AP-Soter-####/gI' /etc/hostapd.conf

# Convert to 802.11n access point
echo "ieee80211n=1" >> /etc/hostapd.conf

# Overlay soter information

rm /ID.txt
rm /etc/dogtag
printf "APISPoint.com SO+ER OS $BASE_IMAGE" > /etc/soteros.id

printf "\n\nBase Image version $BASE_IMAGE\n\n"