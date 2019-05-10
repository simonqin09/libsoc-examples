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
#include "libsoc_pwm.h"

/*
 * libsoc PWM documentation: http://jackmitch.github.io/libsoc/c/pwm
 */


static uint32_t times = 0;
static uint32_t current_duty = 0;

gpio *gpio_interrupt = NULL;
board_config *config;
pwm *mypwm;

/* Apalis iMX6 PWM1 */
#define PWM1_OUTPUT_CHIP 0
#define PWM1_CHIP_OUTPUT 0
/* Apalis iMX6 PWM2 */
#define PWM2_OUTPUT_CHIP 1
#define PWM2_CHIP_OUTPUT 0
/* Apalis iMX6 PWM3, but device is used by ov5640_mipi be default */
#define PWM3_OUTPUT_CHIP 2
#define PWM3_CHIP_OUTPUT 0


int mypwm_init()
{
        int ret = EXIT_SUCCESS;
        /* init PWM */
        mypwm = libsoc_pwm_request(PWM1_OUTPUT_CHIP, PWM1_CHIP_OUTPUT, LS_PWM_SHARED);
        if (!mypwm)
        {
          printf("Failed to get PWM\n");
          return EXIT_FAILURE;
        }
        /* set PWM period and duty cycle */
        ret = libsoc_pwm_set_period(mypwm, 1000000);
        if (ret == EXIT_FAILURE) {
                perror("PWM set period failed");
                return EXIT_FAILURE;
        }

        ret = libsoc_pwm_set_duty_cycle(mypwm, 500000);
        if (ret == EXIT_FAILURE) {
                perror("PWM set duty cycle failed");
                return EXIT_FAILURE;
        }
        
        /* enable pwm */
        libsoc_pwm_set_enabled(mypwm, ENABLED);
        
        return ret;
}

int gpio_interrupt_callback(void* arg)
{
        /* interrupt button input debounce function */
        uint32_t i,ret;
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
        
        /* switch pwm duty from 25% - 50% - 75% */
        current_duty = libsoc_pwm_get_duty_cycle(mypwm);
        switch(current_duty){
	case 250000:{
		ret = libsoc_pwm_set_duty_cycle(mypwm, 500000);
                if (ret == EXIT_FAILURE) {
                        perror("PWM set duty cycle failed");
                        return EXIT_FAILURE;
                }
		//return 0;
		break;

	}
	case 500000:{
		ret = libsoc_pwm_set_duty_cycle(mypwm, 750000);
                if (ret == EXIT_FAILURE) {
                        perror("PWM set duty cycle failed");
                        return EXIT_FAILURE;
                }
		//return 0;
		break;
	}
	case 750000:{
		ret = libsoc_pwm_set_duty_cycle(mypwm, 250000);
                if (ret == EXIT_FAILURE) {
                        perror("PWM set duty cycle failed");
                        return EXIT_FAILURE;
                }
		//return 0;
		break;
	}
	default:{
		printf("wrong PWM duty cycle set\n");
		return -1;
	}
	}
        
        /* print interrupt times */        
	times++;
	printf("Interrupt occurred %d times\n", times);
        /* print current duty */
        current_duty = libsoc_pwm_get_duty_cycle(mypwm);
        printf("PWM Duty set to %d%%\n", current_duty/(1000000/100));
	return 0;
}

int main(void)
{
	uint32_t ret = 0;

	/* board and pwm init */
        config = libsoc_board_init();
        ret = mypwm_init();
        if (ret == EXIT_FAILURE) {
		perror("Failed to init pwm");
		goto exit;
	}

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
        
        if (mypwm) {
		libsoc_pwm_set_enabled(mypwm, DISABLED);
                ret = libsoc_pwm_free(mypwm);
		if (ret == EXIT_FAILURE)
			perror("Could not free pwm");
	}
        
	return ret;
}
