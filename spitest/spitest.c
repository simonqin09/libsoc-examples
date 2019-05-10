#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "libsoc_spi.h"
#include "libsoc_debug.h"

/**
 * 
 * This spi_test is intended to be run on beaglebone white hardware
 * and uses the SPIDEV1 device on pins P9_31 as SCLK, P9_29 as SI, 
 * P9_30 as as SO, P9_28 as CS.
 *
 * The BeagleBone is connected to a MicroChip 25LC640-I/P 64K EEPROM.
 *
 * The test covers writing 32 bytes of random data, to a random page
 * on the EEPROM. It then reads the page back, and compares the data
 * read against the data sent, the test passes if all data matches.
 * 
 */
 
#define SPI_DEVICE   0
#define CHIP_SELECT  0


static uint8_t tx[32], rx[32];

int data_transfer(spi* spi_dev, uint8_t* txdata, uint8_t* rxdata, int len) {
    
  if (len > 32) {
    printf("data size should less than 32 bytes\n");
    return EXIT_FAILURE;
  }
    
  int status;
  status = libsoc_spi_rw(spi_dev, txdata, rxdata, len);
   
  if (status < 0) {
    //printf("Transfer failed...\n");
    return EXIT_FAILURE;  
  } else {
    //printf("Transfer successfully!\n");
    return EXIT_SUCCESS;
  }  
}


int main()
{
  /* enable debug info or not */
  libsoc_set_debug(0);

  int i;
  
  /* initiate SPI device */
  spi* spi_dev = libsoc_spi_init(SPI_DEVICE, CHIP_SELECT);

  if (!spi_dev)
  {
    printf("Failed to get spidev device!\n");
    return EXIT_FAILURE;
  }

  libsoc_spi_set_mode(spi_dev, MODE_1);  
  libsoc_spi_set_speed(spi_dev, 500000);
  libsoc_spi_set_bits_per_word(spi_dev, BITS_8);

   
  printf("Start to transfer!\n");
  
  int len = 16;
  
  /* generate random data to initial tx buffer array */
  memset(tx, 0, len);
  
  struct timeval t1;
  gettimeofday(&t1, NULL);
  srand(t1.tv_usec * t1.tv_sec);
  
  for (i=0; i<len; i++) {
    tx[i] = rand() % 255;
  }  

  /* 1-byte variate for data transfer */
  uint8_t data_send[1], data_read[1];
  
  /* start random data send */
  for (i=0; i<len; i++) {
    data_send[0] = tx[i];
    if (data_transfer(spi_dev, data_send, data_read, 1) < 0){
        printf("data send failed!\n");
        goto free;
    }     
  }
  
  sleep(1);

  /* start read data of just sent */
  data_send[0] = 0xFF;
  for (i=0; i<len; i++) {    
    if (data_transfer(spi_dev, data_send, data_read, 1) < 0){
        printf("data read failed!\n");
        goto free;
    }
    rx[i] = data_read[0];
  }
  
  /* print out and compare sent and received data */  
  for (i=0; i<len; i++) {
    printf("data_send[%d] = 0x%02x : data_read[%d] = 0x%02x", i, tx[i], i, rx[i]);
    
    if (tx[i] == rx[i]) {
      printf(" : Correct\n");
    } else {
      printf(" : Incorrect\n");
    }
  }
   
  /* free spi device */
  
  free:
  
  libsoc_spi_free(spi_dev);

  return EXIT_SUCCESS;
}
