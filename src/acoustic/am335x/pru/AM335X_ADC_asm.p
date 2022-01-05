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
.origin 0
.entrypoint ADC_ACQUISITION

#include "am335x_adc.h"

#define PRU0_ARM_INTERRUPT 19

#define CONST_PRUCFG       C4
#define CONST_PRUSHAREDRAM C28

// Address for the Constant table Programmable Pointer Register 0 (CTPPR_0)
#define CTPPR_0 0x22028

ADC_ACQUISITION:

    // Enable OCP master port
    LBCO r0, CONST_PRUCFG, 4, 4
    CLR  r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
    MOV  r0, 0x00000120
    MOV  r1, CTPPR_0
    SBBO r0, r1, 0, 4

    //
    // Parameters from user land
    //
    LBCO r20, CONST_PRUSHAREDRAM, ADC_HOST_SEQUENCE_FIFO0_VAL_BYTE, 4
    LBCO r21, CONST_PRUSHAREDRAM, ADC_HOST_SEQUENCE_FIFO1_VAL_BYTE, 4
    LBCO r22, CONST_PRUSHAREDRAM, ADC_HOST_CHANNEL_VAL_BYTE, 4
    LBCO r23, CONST_PRUSHAREDRAM, ADC_HOST_FIFO_THRESHOLD_VAL_BYTE, 4

    MOV r0, r20
    MOV r1, ADC_OFFSET | ADC_STEPENABLE
    MOV r2, ADC_OFFSET | ADC_FIFO0COUNT
    MOV r4, ADC_HOST_ACK_ENABLE

    MOV r5, 0 // Current FIFO [0, 1]
    MOV r6, 0 // Next FIFO count check (r6 + r22)

CMDLOOP:

    //
    // Await HOST commands from user land
    //
    LBCO r15, CONST_PRUSHAREDRAM, ADC_HOST_CMD_BYTE_CMD, 4
    QBEQ CMDLOOP, r15, ADC_HOST_CMD_DISABLE
    QBEQ EXIT, r15, ADC_HOST_CMD_EXIT
    SBCO r4, CONST_PRUSHAREDRAM, ADC_HOST_CMD_BYTE_ACK, 4 // Signal ENABLE ACK

DRAINZERO:
    LBBO r10, r2, 0, 4
    QBNE DRAINZERO, r10, 0

    MOV r6, 0

SEQUENCE:
    ADD r6, r6, r22

    //
    // Enable ADC steps for sequencing
    //
    SBBO r0, r1, 0, 4

FIFOCHECK:
    //
    // Wait until all steps are sequenced
    //
    LBBO r10, r2, 0, 4
    QBNE FIFOCHECK, r10, r6
    QBNE SEQUENCE, r10, r23

    MOV r31.b0, PRU0_ARM_INTERRUPT + 16

    QBEQ FILLFIFO1, r5, 0

    MOV r5, 0
    MOV r0, r20
    MOV r2, ADC_OFFSET | ADC_FIFO0COUNT
    JMP CMDLOOP

FILLFIFO1:
    MOV r5, 1
    MOV r0, r21
    MOV r2, ADC_OFFSET | ADC_FIFO1COUNT
    JMP CMDLOOP

EXIT:
    MOV r0, ADC_HOST_ACK_EXIT
    SBCO r0, CONST_PRUSHAREDRAM, ADC_HOST_CMD_BYTE_ACK, 4

    // Signal host that sequencing is complete
    MOV r31.b0, PRU0_ARM_INTERRUPT + 16
    HALT
