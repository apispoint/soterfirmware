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

printf "\n\n********************\nCode Jump\n********************\n\n"

_USR=${1:-root}
_IPA=${2:-192.168.8.1}
_pub=${3:-~/.ssh/dev-soter/dev-soter-appmgr.pub}
_dir=${4:-/tmp}

#BASE_IMAGE=YYJJJ
REF_HW_VER=3

ADC_SPS_SCALAR=0.87480
AIN_CNT=4
CLK_DIV=3
AF_RADIUS=0.045
GIST_LEAD_LEN=8192
GIST_LEN=32768
STALTA_STA_LEN=16
STALTA_LTA_LEN=4096
STALTA_R_LIMIT=0.80
GIST_URI="https://api-soter.apispoint.com/gist"
CLIP_URI="https://api-soter.apispoint.com/clip"
CONF_URI="https://api-soter.apispoint.com/config"

AP_HOME=/opt/apispoint

FW_NAME=fw
CFG_NAME=config
SOTER_NAME=soter
PROVISION_NAME=provision
APPMGR_NAME=appmgr

BI_HOME=$AP_HOME/$BASE_IMAGE
FW_HOME=$AP_HOME/$FW_NAME
CFG_HOME=$AP_HOME/$CFG_NAME

AP_SOTER_HOME=$AP_HOME/$SOTER_NAME
AP_PROVISION_HOME=$AP_HOME/$PROVISION_NAME
AP_APPMGR_HOME=$AP_HOME/$APPMGR_NAME

SOTER_HOME=$BI_HOME/$SOTER_NAME
PROVISION_HOME=$BI_HOME/$PROVISION_NAME
APPMGR_HOME=$BI_HOME/$APPMGR_NAME

#
# Set Logical SYMLINK FW TO BASE_IMAGE Package
#

ssh $_USR@$_IPA \
" \
    rm -r $AP_HOME ; \
    mkdir -p $BI_HOME ; \
    mkdir -p $CFG_HOME ; \
    ln -s $BI_HOME $FW_HOME \
"

#
# Copy AppMgr Services
#

ssh $_USR@$_IPA \
" \
    rm -r $APPMGR_HOME ; \
    mkdir -p $APPMGR_HOME \
"

scp -r ../appmgr/* $_USR@$_IPA:$APPMGR_HOME
scp $_pub $_USR@$_IPA:$APPMGR_HOME

#
# Copy Provisioning Services
#

ssh $_USR@$_IPA \
" \
    rm -r $PROVISION_HOME ; \
    mkdir -p $PROVISION_HOME \
"

scp -r ../provision/* $_USR@$_IPA:$PROVISION_HOME
scp ../resources/script/webapp.sh $_USR@$_IPA:$PROVISION_HOME
scp ../resources/script/httpd.conf $_USR@$_IPA:$PROVISION_HOME
scp ../resources/script/*service.sh $_USR@$_IPA:$PROVISION_HOME/services

#
# Copy code and build
#

scp \
    ../src/*.cpp \
    ../src/acoustic/am335x/*.h \
    ../src/acoustic/am335x/*.c \
    ../src/acoustic/am335x/pru/* \
    ../src/sensors/* \
    ../src/services/* \
    ../src/statistics/* \
$_USR@$_IPA:$_dir

ssh $_USR@$_IPA \
" \
    cd $_dir ; \
    rm -r $SOTER_HOME ; \
    mkdir -p $SOTER_HOME ; \
    pasm -b AM335X_ADC_asm.p asmADC ; \
    mv asmADC.bin $SOTER_HOME ; \
    arm-linux-gnueabihf-g++ -std=c++11 -pthread -fPIC -lprussdrv -shared -fpermissive -DAINCNT=$AIN_CNT -DCLKDIV=$CLK_DIV -o $SOTER_HOME/libADC_p.so AM335X_ADC_pru.cpp ; \
    arm-linux-gnueabihf-g++ -march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -ffast-math -O3 -std=c++11 -pthread -lcrypto -lcurl -fpermissive -lm -ljansson -DREF_HW_VER=$REF_HW_VER -DGIST_LEAD_LEN=$GIST_LEAD_LEN -DGIST_LEN=$GIST_LEN -DDURATION=0 -DSTALTA_R_LIMIT=$STALTA_R_LIMIT -DSTALTA_STA_LEN=$STALTA_STA_LEN -DSTALTA_LTA_LEN=$STALTA_LTA_LEN -DAF_RADIUS=$AF_RADIUS -DGIST_INGEST_URI=\"\\\"$GIST_URI\\\"\" -DCLIP_INGEST_URI=\"\\\"$CLIP_URI\\\"\" -DPROVISION_URI=\"\\\"$CONF_URI\\\"\" -L$SOTER_HOME -lADC_p tscadc.c GPS_UART.cpp Mag_LSM9DS1.cpp TmpBaro_MPL115A2.cpp FirstArrivalDriver.cpp -o $SOTER_HOME/soter \
"

#
# Copy test and build
#

scp \
    ../src/diagnostics/* \
$_USR@$_IPA:$_dir

DELAY=10
DURATION=3600

ssh $_USR@$_IPA \
" \
    cd $_dir ; \
    g++ -std=c++11 -pthread Mag_LSM9DS1.cpp magcal_lsm9ds1.cpp -o $SOTER_HOME/magcal ; \
    g++ -std=c++11 -pthread -DDEBUGNMEA GPS_UART.cpp gps_uart_test.cpp -o $SOTER_HOME/gps ; \
    g++ -std=c++11 -pthread TmpBaro_MPL115A2.cpp tmpbaro_mpl115a2_test.cpp -o $SOTER_HOME/tmp ; \
    g++ -std=c++11 -pthread -lm Mag_LSM9DS1.cpp mag_lsm9ds1_test.cpp -o $SOTER_HOME/mag ; \
    arm-linux-gnueabihf-g++ -march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -ffast-math -O3 -std=c++11 -pthread -fpermissive -lprussdrv -lrt -L$SOTER_HOME -lADC_p tscadc.c Test_SPS.cpp -o $SOTER_HOME/sps ; \
    arm-linux-gnueabihf-g++ -march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -ffast-math -O3 -std=c++11 -pthread -fpermissive -lprussdrv -L$SOTER_HOME -lADC_p tscadc.c Test_Rec.cpp -o $SOTER_HOME/rec -DDELAY=$DELAY ; \
    arm-linux-gnueabihf-g++ -march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -ffast-math -O3 -std=c++11 -pthread -fpermissive -lprussdrv -L$SOTER_HOME -lADC_p -ljansson -DADC_SPS_SCALAR=$ADC_SPS_SCALAR -DREF_HW_VER=$REF_HW_VER -DGIST_LEAD_LEN=$GIST_LEAD_LEN -DAF_RADIUS=$AF_RADIUS tscadc.c GPS_UART.cpp Mag_LSM9DS1.cpp TmpBaro_MPL115A2.cpp Test_RecJSON.cpp -o $SOTER_HOME/recgist -DDELAY=$DELAY ; \
    arm-linux-gnueabihf-g++ -march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -ffast-math -O3 -std=c++11 -pthread -lcrypto -lcurl -fpermissive -lm -ljansson -DREF_HW_VER=$REF_HW_VER -DGIST_LEAD_LEN=$GIST_LEAD_LEN -DGIST_LEN=$GIST_LEN -DDURATION=$DURATION -DSTALTA_R_LIMIT=$STALTA_R_LIMIT -DSTALTA_STA_LEN=$STALTA_STA_LEN -DSTALTA_LTA_LEN=$STALTA_LTA_LEN -DAF_RADIUS=$AF_RADIUS -DGIST_INGEST_URI=\"\\\"$GIST_URI\\\"\" -DCLIP_INGEST_URI=\"\\\"$CLIP_URI\\\"\" -DPROVISION_URI=\"\\\"$CONF_URI\\\"\" -L$SOTER_HOME -lADC_p tscadc.c GPS_UART.cpp Mag_LSM9DS1.cpp TmpBaro_MPL115A2.cpp FirstArrivalDriver.cpp -o $SOTER_HOME/fad -DDEBUGSPS -DDEBUGWATCHDOG -DDEBUGCONFIG -DDEBUGDETECT -DDEBUGTELEMETRY \
"

#
# Load ALL scripts
#

scp \
    ../resources/script/pinouts-cfg.sh \
    ../resources/script/hostapd-cfg.sh \
    ../resources/script/soter.sh \
    ../resources/script/module.sh \
    ../resources/script/fad.sh \
    ../resources/script/recgist.sh \
$_USR@$_IPA:$SOTER_HOME

#
# SYMLINK FW to AP_X_HOME and RO FW
#

scp \
    ../resources/script/setfw.sh \
$_USR@$_IPA:$FW_HOME

ssh $_USR@$_IPA \
" \
    sed -i 's#{{AP_HOME}}#$AP_HOME#g;\
            s#{{FW_NAME}}#$FW_NAME#g;\
            s#{{SOTER_NAME}}#$SOTER_NAME#g;\
            s#{{PROVISION_NAME}}#$PROVISION_NAME#g;\
            s#{{APPMGR_NAME}}#$APPMGR_NAME#g' $BI_HOME/setfw.sh ; \
    chattr -R +i $BI_HOME ; \
    chattr -R +i $FW_HOME ; \
    sh $FW_HOME/setfw.sh NOREBOOT \
"

#
# Enable ALL services
#

scp ../resources/systemd/* $_USR@$_IPA:/etc/systemd/system

ssh $_USR@$_IPA \
" \
    systemctl reenable pinouts-cfg.service ; \
    systemctl reenable hostapd-cfg.service ; \
    systemctl reenable apwifiservice.service ; \
    systemctl reenable devidservice.service ; \
    systemctl reenable magcalservice.service ; \
    systemctl reenable pingservice.service ; \
    systemctl reenable sensorservice.service ; \
    systemctl reenable signinservice.service ; \
    systemctl reenable wifiservice.service ; \
    systemctl reenable appmgr.service ; \
    systemctl reenable webapp.service ; \
    systemctl reenable soter.service \
"

#
# Inventory dump
#

printf "\n\n********************\nInventory Dump\n********************\n\n"

ssh $_USR@$_IPA \
" \
    ls $SOTER_HOME ; \
"