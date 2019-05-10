/*
 * io.c
 * A simple test application for toggling PCF8574 IOs using libsoc API.
 *
 * Copyright (C) 2017 - BhuvanChandra DV <bhuvanchandradv@gmail.com>
 *
 * io is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * io is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with io. If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "eeprom.h"

int main(int argc, char **argv)
{
	i2c *i2c_eeprom;
	/* number of page to write/read */
        uint8_t page = 0;
        /* data buffers for write/read */
        uint8_t data_write[EEPROM_PAGE_SIZE+1];
        uint8_t data_read[EEPROM_PAGE_SIZE];
        uint8_t ret = 0, i = 0;
	//static bool on = false;
	
	/* get from which page to write from applicaiton parameter */
        if(argc < 2){
			printf("Usage: %s <page number between 0 to 15>\n", argv[0]);
			return -1;
	}

	i2c_eeprom = eeprom_init(I2C_BUS, EEPROM_ADDRESS);
	if (i2c_eeprom == NULL)
		return EXIT_FAILURE;
        
        page = atoi(argv[1]) * EEPROM_PAGE_SIZE;
        // The first byte of the i2c write in the EEPROM protocol is the page
        data_write[0] = page;
  
        // Fill the rest of the write buffer with random data
        for (i=1; i<(EEPROM_PAGE_SIZE+1); i++) {
          //data_write[i] = rand() % 255;
          data_write[i] = page+(i-1);  
        }

        /* start to write page */
        printf ("Writing to page starting at byte address: 0x%02x\n", page);
        ret = eeprom_write_buf(i2c_eeprom, data_write, EEPROM_PAGE_SIZE+1);
	/* Wait for the page to be written */
        printf("Waiting for data to be written\n");  
        while (libsoc_i2c_write(i2c_eeprom, &page, 1) == EXIT_FAILURE) {
          printf("Waiting...\n");
          usleep(1);
        }
        printf("Data sucessfully written!\n");


        /* start to read page */
        /* Write page address to read */
        ret = libsoc_i2c_write(i2c_eeprom, &page, 1);
  
        /* Read page */
        ret = eeprom_read_buf(i2c_eeprom, data_read, EEPROM_PAGE_SIZE);
        if (ret != EEPROM_PAGE_SIZE) {
          printf("Read page failed\n");
        }
        printf ("Reading page starting at byte address: 0x%02x\n", page);

        /* Print out read back page contects, while checking validity */
        for (i=0; i<EEPROM_PAGE_SIZE; i++) {
          printf("data_write[%d] = 0x%02x : data_read[%d] = 0x%02x", i, data_write[(i+1)], i, data_read[i]);
    
          if (data_write[(i+1)] == data_read[i]) {
            printf(" : Correct\n");
          } else {
            printf(" : Incorrect\n");
          }
        }
        
	goto exit;

exit:
	ret = eeprom_free(i2c_eeprom);
	if (ret == EXIT_FAILURE)
		perror("Failed to close I2C port");

	return 0;
}
