#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <libsoc_i2c.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* define 2kb capacity EEPROM devie CAT24C02WI-GT3 related information */ 
#define I2C_BUS              1
#define EEPROM_ADDRESS		0x50

#define EEPROM_SIZE 256*8
#define EEPROM_PAGE_SIZE 16
#define EEPROM_NUM_PAGES EEPROM_SIZE/EEPROM_PAGE_SIZE

i2c* eeprom_init(uint8_t i2c_bus, uint8_t i2c_address);
int eeprom_free(i2c *i2c);
size_t eeprom_read_buf(i2c *i2c, void *buf, size_t len);
size_t eeprom_write_buf(i2c *i2c, void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _EEPROM_H_ */
