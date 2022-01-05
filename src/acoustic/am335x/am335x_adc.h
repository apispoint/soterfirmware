//
//    Copyright (c) 2021 APIS Point, LLC.
//    All rights reserved.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef AM335X_ADC_H
#define AM335X_ADC_H

//
// ADC HOST COMMANDS + SEQUENCE CONST
//
#define ADC_HOST_CMD_UINT_CMD 0
#define ADC_HOST_CMD_BYTE_CMD 0

#define ADC_HOST_CMD_UINT_ACK 1
#define ADC_HOST_CMD_BYTE_ACK 4

#define ADC_HOST_SEQUENCE_FIFO0_VAL_UINT 2
#define ADC_HOST_SEQUENCE_FIFO0_VAL_BYTE 8

#define ADC_HOST_SEQUENCE_FIFO1_VAL_UINT 3
#define ADC_HOST_SEQUENCE_FIFO1_VAL_BYTE 12

#define ADC_HOST_CHANNEL_VAL_UINT 4
#define ADC_HOST_CHANNEL_VAL_BYTE 16

#define ADC_HOST_FIFO_THRESHOLD_VAL_UINT 5
#define ADC_HOST_FIFO_THRESHOLD_VAL_BYTE 20

//
// COMMANDS
//
#define ADC_HOST_CMD_DISABLE (0x0)
#define ADC_HOST_CMD_ENABLE  (0x1)
#define ADC_HOST_CMD_EXIT    (0x2)

//
// COMMAND ACKS
//
#define ADC_HOST_ACK_ENABLE (0x10)
#define ADC_HOST_ACK_EXIT   (0x11)

//
// See Device Tree
//
#define ADC_OFFSET (0x44E0D000)
#define ADC_LENGTH (0x1000)

//
// See TI AM335x TRM
//
#define ADC_IRQSTATUS_RAW  (0x24)
#define ADC_IRQSTATUS      (0x28)
#define ADC_IRQENABLE_SET  (0x2C)
#define ADC_IRQENABLE_CLR  (0x30)
#define ADC_CTRL           (0x40)
#define ADC_CLKDIV         (0x4C)
#define ADC_STEPENABLE     (0x54)
#define ADC_FIFO0COUNT     (0xE4)
#define ADC_FIFO0THRESHOLD (0xE8)
#define ADC_FIFO1COUNT     (0xF0)
#define ADC_FIFO1THRESHOLD (0xF4)

//
// Register Masks
//
#define ADC_FIFO_COUNT_MASK     (0x7F)
#define ADC_FIFO_DATA_MASK      (0xFFF)
#define ADC_FIFO_CHNL_MASK      (0xF0000)
#define ADC_FIFO_CHNL_DATA_MASK (0xF0FFF)

//
// Only 7 channels are available on the BB
//
#define ADC_STEPCONFIG1  (0x64)
#define ADC_STEPDELAY1   (0x68)
#define ADC_STEPCONFIG2  (0x6C)
#define ADC_STEPDELAY2   (0x70)
#define ADC_STEPCONFIG3  (0x74)
#define ADC_STEPDELAY3   (0x78)
#define ADC_STEPCONFIG4  (0x7C)
#define ADC_STEPDELAY4   (0x80)
#define ADC_STEPCONFIG5  (0x84)
#define ADC_STEPDELAY5   (0x88)
#define ADC_STEPCONFIG6  (0x8C)
#define ADC_STEPDELAY6   (0x90)
#define ADC_STEPCONFIG7  (0x94)
#define ADC_STEPDELAY7   (0x98)
#define ADC_STEPCONFIG8  (0x9C)
#define ADC_STEPDELAY8   (0xA0)
#define ADC_STEPCONFIG9  (0xA4)
#define ADC_STEPDELAY9   (0xA8)
#define ADC_STEPCONFIG10 (0xAC)
#define ADC_STEPDELAY10  (0xB0)
#define ADC_STEPCONFIG11 (0xB4)
#define ADC_STEPDELAY11  (0xB8)
#define ADC_STEPCONFIG12 (0xBC)
#define ADC_STEPDELAY12  (0xC0)
#define ADC_STEPCONFIG13 (0xC4)
#define ADC_STEPDELAY13  (0xC8)
#define ADC_STEPCONFIG14 (0xCC)
#define ADC_STEPDELAY14  (0xD0)
#define ADC_STEPCONFIG15 (0xD4)
#define ADC_STEPDELAY15  (0xD8)
#define ADC_STEPCONFIG16 (0xDC)
#define ADC_STEPDELAY16  (0xE0)

#define ADC_FIFO0DATA (0x100)
#define ADC_FIFO1DATA (0x200)

//
// USE TO CTRL ADC
//
#define ADC_CTRL_VAL_ENABLE  (0x1)
#define ADC_CTRL_VAL_DISABLE (0x0)

#define ADC_CTRL_VAL_STEPCONFIG_WRITE_PROTECT_OFF (0x1 << 2)
#define ADC_CTRL_VAL_STEPCONFIG_WRITE_PROTECT_ON  (0x0 << 2)

//
// USE TO ENABLE STEPx
//
#define ADC_SE_VAL_STEP1  (0x1 << 1)
#define ADC_SE_VAL_STEP2  (0x1 << 2)
#define ADC_SE_VAL_STEP3  (0x1 << 3)
#define ADC_SE_VAL_STEP4  (0x1 << 4)
#define ADC_SE_VAL_STEP5  (0x1 << 5)
#define ADC_SE_VAL_STEP6  (0x1 << 6)
#define ADC_SE_VAL_STEP7  (0x1 << 7)
#define ADC_SE_VAL_STEP8  (0x1 << 8)
#define ADC_SE_VAL_STEP9  (0x1 << 9)
#define ADC_SE_VAL_STEP10 (0x1 << 10)
#define ADC_SE_VAL_STEP11 (0x1 << 11)
#define ADC_SE_VAL_STEP12 (0x1 << 12)
#define ADC_SE_VAL_STEP13 (0x1 << 13)
#define ADC_SE_VAL_STEP14 (0x1 << 14)
#define ADC_SE_VAL_STEP15 (0x1 << 15)
#define ADC_SE_VAL_STEP16 (0x1 << 16)

//
// USE TO CONFIGURE STEPCONFIG
//
#define ADC_SC_VAL_FIFO0_THRESHOLD_IRQENABLE (0x1 << 2)
#define ADC_SC_VAL_FIFO1_THRESHOLD_IRQENABLE (0x1 << 5)

#define ADC_SC_VAL_FIFO_0 (0x0 << 26)
#define ADC_SC_VAL_FIFO_1 (0x1 << 26)

#define ADC_SC_VAL_AVG1  (0x0 << 2)
#define ADC_SC_VAL_AVG2  (0x1 << 2)
#define ADC_SC_VAL_AVG4  (0x2 << 2)
#define ADC_SC_VAL_AVG8  (0x3 << 2)
#define ADC_SC_VAL_AVG16 (0x4 << 2)

#define ADC_SC_VAL_SW_ONESHOT    (0x0)
#define ADC_SC_VAL_SW_CONTINUOUS (0x1)
#define ADC_SC_VAL_HW_ONESHOT    (0x2)
#define ADC_SC_VAL_HW_CONTINUOUS (0x3)

#define ADC_SC_VAL_AIN0 (0x0 << 19)
#define ADC_SC_VAL_AIN1 (0x1 << 19)
#define ADC_SC_VAL_AIN2 (0x2 << 19)
#define ADC_SC_VAL_AIN3 (0x3 << 19)
#define ADC_SC_VAL_AIN4 (0x4 << 19)
#define ADC_SC_VAL_AIN5 (0x5 << 19)
#define ADC_SC_VAL_AIN6 (0x6 << 19)

#define ADC_SC_VAL_CHANNEL_1 ADC_SC_VAL_AIN0
#define ADC_SC_VAL_CHANNEL_2 ADC_SC_VAL_AIN1
#define ADC_SC_VAL_CHANNEL_3 ADC_SC_VAL_AIN2
#define ADC_SC_VAL_CHANNEL_4 ADC_SC_VAL_AIN3
#define ADC_SC_VAL_CHANNEL_5 ADC_SC_VAL_AIN4
#define ADC_SC_VAL_CHANNEL_6 ADC_SC_VAL_AIN5
#define ADC_SC_VAL_CHANNEL_7 ADC_SC_VAL_AIN6

#endif     /* AM335X_ADC_H */
