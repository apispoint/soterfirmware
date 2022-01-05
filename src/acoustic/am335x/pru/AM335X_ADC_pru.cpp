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
#include <chrono>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <thread>

#include "AM335X_ADC_Sensor.h"

#define PRU_NUM 0
#define OFFSET_SHAREDRAM 0x800

#if AINCNT != 7
    #define AINCNT 4
#endif

/*
#define ADC_MAX_VALUE 4095.0 // 12 bit ADC or 2^12 for [0, 4096)
#define ADC_REF_VOLTS 1.8    // Reference voltage
#define ADC_FACTOR    ADC_REF_VOLTS / ADC_MAX_VALUE

inline double valueToVolts(int value) {
    // D = Vin * (2^n - 1) / Vref
    // Where:
    //    D    = Digital value
    //    Vin  = Input voltage
    //    n    = Number of bits
    //    Vref = Reference voltage
    return value * ADC_FACTOR;
}
*/

AM335X_ADC::AM335X_ADC() {
    BUFFERLEN = (FIFO_THRESHOLD / AINCNT) * AINCNT;
    pru_state = created;
}

int AM335X_ADC::grouping() { return IMicArray::GROUP_BY_SCAN_NUMBER; }
int AM335X_ADC::channels() { return AINCNT; }

//
// CLK    := 24000000      ; 24mhz
// CLKDIV := [0, CLK) + 1  ; +1 is implicit
// CLKACQ := 15            ; cycles
// CHNCNT := [1, 7]
//
// SPS   := CLK / CLKDIV / CLKACQ
// SPS_C := SPS / CHNCNT
//
int AM335X_ADC::channelSampleRate() { return 24000000 / (CLKDIV + 1) / 15 / AINCNT; }
int AM335X_ADC::channelReadRate() { return BUFFERLEN / AINCNT; }

bool AM335X_ADC::init() {
    if(pru_state != created)
        return false;

    map = adc_open(&fd);

    //
    // Configure the ADC
    //
    adc_config(ADC_CTRL, ADC_CTRL_VAL_DISABLE | ADC_CTRL_VAL_STEPCONFIG_WRITE_PROTECT_OFF, map);

    adc_config(ADC_STEPCONFIG1, ADC_SC_VAL_AIN0 | ADC_SC_VAL_FIFO_0, map);
    adc_config(ADC_STEPCONFIG2, ADC_SC_VAL_AIN1 | ADC_SC_VAL_FIFO_0, map);
    adc_config(ADC_STEPCONFIG3, ADC_SC_VAL_AIN2 | ADC_SC_VAL_FIFO_0, map);
    adc_config(ADC_STEPCONFIG4, ADC_SC_VAL_AIN3 | ADC_SC_VAL_FIFO_0, map);
#if AINCNT == 7
    adc_config(ADC_STEPCONFIG5, ADC_SC_VAL_AIN4 | ADC_SC_VAL_FIFO_0, map);
    adc_config(ADC_STEPCONFIG6, ADC_SC_VAL_AIN5 | ADC_SC_VAL_FIFO_0, map);
    adc_config(ADC_STEPCONFIG7, ADC_SC_VAL_AIN6 | ADC_SC_VAL_FIFO_0, map);
#endif

    adc_config(ADC_STEPCONFIG8,  ADC_SC_VAL_AIN0 | ADC_SC_VAL_FIFO_1, map);
    adc_config(ADC_STEPCONFIG9,  ADC_SC_VAL_AIN1 | ADC_SC_VAL_FIFO_1, map);
    adc_config(ADC_STEPCONFIG10, ADC_SC_VAL_AIN2 | ADC_SC_VAL_FIFO_1, map);
    adc_config(ADC_STEPCONFIG11, ADC_SC_VAL_AIN3 | ADC_SC_VAL_FIFO_1, map);
#if AINCNT == 7
    adc_config(ADC_STEPCONFIG12, ADC_SC_VAL_AIN4 | ADC_SC_VAL_FIFO_1, map);
    adc_config(ADC_STEPCONFIG13, ADC_SC_VAL_AIN5 | ADC_SC_VAL_FIFO_1, map);
    adc_config(ADC_STEPCONFIG14, ADC_SC_VAL_AIN6 | ADC_SC_VAL_FIFO_1, map);
#endif

    //
    // Initialize the steps to fire as quickly as possible, 0 delay
    //
    adc_config(ADC_STEPDELAY1,  0, map);
    adc_config(ADC_STEPDELAY2,  0, map);
    adc_config(ADC_STEPDELAY3,  0, map);
    adc_config(ADC_STEPDELAY4,  0, map);
#if AINCNT == 7
    adc_config(ADC_STEPDELAY5,  0, map);
    adc_config(ADC_STEPDELAY6,  0, map);
    adc_config(ADC_STEPDELAY7,  0, map);
#endif
    adc_config(ADC_STEPDELAY8,  0, map);
    adc_config(ADC_STEPDELAY9,  0, map);
    adc_config(ADC_STEPDELAY10, 0, map);
    adc_config(ADC_STEPDELAY11, 0, map);
#if AINCNT == 7
    adc_config(ADC_STEPDELAY12, 0, map);
    adc_config(ADC_STEPDELAY13, 0, map);
    adc_config(ADC_STEPDELAY14, 0, map);
#endif

    adc_config(ADC_CLKDIV, CLKDIV, map);

    adc_config(ADC_CTRL, ADC_CTRL_VAL_ENABLE | ADC_CTRL_VAL_STEPCONFIG_WRITE_PROTECT_ON, map);

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    /* Initialize the PRU */
    prussdrv_init();

    /* Open PRU Interrupt */
    if(prussdrv_open(PRU_EVTOUT_0)) {
        fprintf(stderr, "prussdrv_open open failed\n");
        return false;
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);
/*
    prussdrv_pru_disable(PRU_NUM);
    prussdrv_pru_enable(PRU_NUM);
    prussdrv_pru_reset(PRU_NUM);
*/
    void *sharedCmdMem;
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedCmdMem);
    cmdMem = (unsigned int *) sharedCmdMem;

    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_CMD_UINT_CMD] = ADC_HOST_CMD_DISABLE;

    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_CHANNEL_VAL_UINT]        = AINCNT;
    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_FIFO_THRESHOLD_VAL_UINT] = BUFFERLEN;
    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_SEQUENCE_FIFO0_VAL_UINT] =
                            ADC_SE_VAL_STEP1
                          | ADC_SE_VAL_STEP2
                          | ADC_SE_VAL_STEP3
                          | ADC_SE_VAL_STEP4
#if AINCNT == 7
                          | ADC_SE_VAL_STEP5
                          | ADC_SE_VAL_STEP6
                          | ADC_SE_VAL_STEP7
#endif
        ;

    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_SEQUENCE_FIFO1_VAL_UINT] =
                            ADC_SE_VAL_STEP8 
                          | ADC_SE_VAL_STEP9
                          | ADC_SE_VAL_STEP10
                          | ADC_SE_VAL_STEP11
#if AINCNT == 7
                          | ADC_SE_VAL_STEP12
                          | ADC_SE_VAL_STEP13
                          | ADC_SE_VAL_STEP14
#endif
        ;

    // Execute program
    prussdrv_exec_program(PRU_NUM, "./asmADC.bin");

    pru_state = inited;

    return true;
}

bool AM335X_ADC::stop() {
    if(pru_state != running)
        return false;

    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_CMD_UINT_CMD] = ADC_HOST_CMD_EXIT;
    pru_state = stopping;

    attempt = 0;
    while(pru_state == stopping && attempt++ < retries)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    prussdrv_pru_disable(PRU_NUM);
    prussdrv_exit();

    adc_close(fd, map);

    pru_state = created;

    return true;
}

void AM335X_ADC::process() {
    unsigned int *_fifo0      = (map + ADC_FIFO0DATA);
    unsigned int *_fifo0count = (map + ADC_FIFO0COUNT);
    unsigned int *_fifo1      = (map + ADC_FIFO1DATA);
    unsigned int *_fifo1count = (map + ADC_FIFO1COUNT);

    short *data;
    int i, samp;

    cmdMem[OFFSET_SHAREDRAM + ADC_HOST_CMD_UINT_CMD] = ADC_HOST_CMD_ENABLE;

    do {
        prussdrv_pru_wait_event(PRU_EVTOUT_0);
        prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

        //
        // Drain ADC_FIFO0
        //
        if((*_fifo0count & ADC_FIFO_COUNT_MASK) == BUFFERLEN) {
            data = new short[BUFFERLEN];
            for(samp = BUFFERLEN, i = 0; i < BUFFERLEN; i++, samp--) {
                data[i] = (*_fifo0 & ADC_FIFO_DATA_MASK);
                while((*_fifo0count & ADC_FIFO_COUNT_MASK) == samp)
                    ;
            }
            batchqueue->submit(data);
        }

        //
        // Drain ADC_FIFO1
        //
        if((*_fifo1count & ADC_FIFO_COUNT_MASK) == BUFFERLEN) {
            data = new short[BUFFERLEN];
            for(samp = BUFFERLEN, i = 0; i < BUFFERLEN; i++, samp--) {
                data[i] = (*_fifo1 & ADC_FIFO_DATA_MASK);
                while((*_fifo1count & ADC_FIFO_COUNT_MASK) == samp)
                    ;
            }
            batchqueue->submit(data);
        }
//    } while(cmdMem[OFFSET_SHAREDRAM + ADC_HOST_CMD_UINT_ACK] != ADC_HOST_ACK_EXIT);
    } while(pru_state == running);

    pru_state = stopped;
}

void AM335X_ADC::read(BatchQueue<short *> *queue) {
    if(pru_state != inited)
        return;

    pru_state = running;

    batchqueue = queue;
    std::thread thrd(&AM335X_ADC::process, this);
    sched_param sch;
    sch.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(thrd.native_handle(), SCHED_FIFO, &sch);
    thrd.detach();
}
