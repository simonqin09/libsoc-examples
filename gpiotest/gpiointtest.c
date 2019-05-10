#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "libsoc_gpio.h"
#include "libsoc_board.h"

/*
 * libsoc GPIO documentation: http://jackmitch.github.io/libsoc/c/gpio/#gpio
 */

volatile bool led_status = false;
static uint32_t times = 0;

gpio *gpio_interrupt = NULL;
gpio *gpio_led = NULL;
board_config *config;

int gpio_interrupt_callback(void* arg)
{
        /* interrupt button input debounce function */
        uint32_t i;
        uint32_t debounce = 0;
        
        for (i=0;i<3;i++) {
        
                usleep(20000);
                if (0 == libsoc_gpio_get_level(gpio_interrupt)) {
                        debounce ++;
                }
        }

        if (debounce >=2) {
                return 0;
        }
        
        /* Led gpio output level transition */
        if (!led_status) {
		led_status = true;
		libsoc_gpio_set_level(gpio_led, HIGH);
                printf("LED turns on\n");
	} else {
		led_status = false;
		libsoc_gpio_set_level(gpio_led, LOW);
                printf("LED turns off\n");
	}
	times++;
	printf("Interrupt occurred %d times\n", times);

	return 0;
}

int main(void)
{
	uint32_t ret = 0;

	config = libsoc_board_init();

	/*
	 * It is expected that the gpio being requested is pin multiplexed correctly
	 * in the kernel device tree or board file as applicable else gpio request
	 * is bound to fail.
	 */
	gpio_interrupt = libsoc_gpio_request(libsoc_board_gpio_id(config, "MXM3_11"), LS_GPIO_SHARED);
	if (gpio_interrupt == NULL) {
		perror("gpio request failed");
		goto exit;
	}

	gpio_led = libsoc_gpio_request(libsoc_board_gpio_id(config, "MXM3_13"), LS_GPIO_SHARED);
	if (gpio_led == NULL) {
		perror("led gpio request failed");
		goto exit;
	}

	ret = libsoc_gpio_set_direction(gpio_led, OUTPUT);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio led direction");
		goto exit;
	}

	ret = libsoc_gpio_set_direction(gpio_interrupt, INPUT);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio direction");
		goto exit;
	}

	ret = libsoc_gpio_set_edge(gpio_interrupt, RISING);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio edge");
		goto exit;
	}

	ret = libsoc_gpio_callback_interrupt(gpio_interrupt, &gpio_interrupt_callback, NULL);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio callback");
		goto exit;
	}

	printf("Waiting for interrupt. Press 'q' and 'Enter' at any time to exit\n");

	char key = -1;
	while (true) {
		key = fgetc(stdin);
		if (key == 'q')
			goto exit;
	}

exit:
	if (gpio_interrupt) {
		ret = libsoc_gpio_free(gpio_interrupt);
		if (ret == EXIT_FAILURE)
			perror("Could not free gpio");
	}

	if (gpio_led) {
		ret = libsoc_gpio_free(gpio_led);
		if (ret == EXIT_FAILURE)
			perror("Could not free led gpio");
	}

	return ret;
}
