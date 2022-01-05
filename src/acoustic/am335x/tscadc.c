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
#include "tscadc.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "am335x_adc.h"

void* adc_open(int *fd) {
    *fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (*fd < 0) {
        fprintf(stderr, "Failed to open /dev/mem\n");
        exit(EXIT_FAILURE);
    }

    void *map = mmap(NULL, ADC_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, ADC_OFFSET);
    if(map == MAP_FAILED) {
        close(*fd);
        fprintf(stderr, "Failed to map the ADC device\n");
        exit(EXIT_FAILURE);
    }

    return map;
}

void adc_close(int fd, void *map) {
    munmap(map, ADC_LENGTH);
    close(fd);
}
