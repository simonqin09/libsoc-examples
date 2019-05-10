/*
 * pcf8574.c
 * PCF8574 IO expander helper functions using libsoc API.
 *
 * Copyright (C) 2017 - BhuvanChandra DV <bhuvanchandradv@gmail.com>
 *
 * pcf8574 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * pcf8574 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pcf8574. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "eeprom.h"

i2c* eeprom_init(uint8_t i2c_bus, uint8_t i2c_address)
{
	i2c *i2c = libsoc_i2c_init(i2c_bus, i2c_address);

	if (i2c == NULL) {
		perror("libsoc_i2c_init failed");
		return i2c;
	}

	return i2c;
}

int eeprom_free(i2c *i2c)
{
	return libsoc_i2c_free(i2c);
}


size_t eeprom_read_buf(i2c *i2c, void *buf, size_t len)
{
	uint8_t *_buf = (uint8_t *)buf;

	if (!len || !buf)
		return 0;

	if (libsoc_i2c_read(i2c, _buf, len))
		return 0;

	return len;
}

size_t eeprom_write_buf(i2c *i2c, void *buf, size_t len)
{
	uint8_t *_buf = (uint8_t *)buf;

	if (!len || !buf)
		return 0;

	if (libsoc_i2c_write(i2c, _buf, len))
		return 0;

	return len;
}

