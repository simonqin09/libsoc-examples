#!/usr/bin/python3
import threading
import time
import sys

from libsoc import gpio


def test_interrupt_handler(gpio_in, gpio_out):
    
    debounce = 0
    
    for x in range(3):
        time.sleep(0.02)
        if gpio_in.is_high() == 1:
            debounce = debounce + 1
    
    if debounce >= 2:
        if gpio_out.is_high() == 1:
            gpio_out.set_low()
            print("The LED turns OFF")
        else:
            gpio_out.set_high()
            print("The LED turns ON")


def main(gpio_input_id, gpio_output_id):
    gpio_in = gpio.GPIO(gpio_input_id, gpio.DIRECTION_INPUT)
    gpio_out = gpio.GPIO(gpio_output_id, gpio.DIRECTION_OUTPUT)
    
    with gpio.request_gpios((gpio_in, gpio_out)):
        assert gpio.DIRECTION_INPUT == gpio_in.get_direction()
        assert gpio.DIRECTION_OUTPUT == gpio_out.get_direction()
        
        gpio_out.set_high()
        assert gpio_out.is_high()
        print("The LED initial status is ON")
        print("please enter 'Q' to quit")
        
        gpio_in.set_direction(gpio.DIRECTION_INPUT, gpio.EDGE_RISING)
        class int_handler(object):
            hits = 0

            def callback(self):
                self.hits += 1
                test_interrupt_handler(gpio_in, gpio_out)

        cb=int_handler()
        h = gpio_in.start_interrupt_handler(cb.callback)

        while 1:
           n = sys.stdin.readline().strip('\n')
           #print(n)
           if n == "Q":
               print("Do you really want to quit? yes or no")
               m = sys.stdin.readline().strip('\n')
               if m == "yes":
                   h.stop()
                   h.join()
                   exit()
                   print("interrupt times is %d" %cb.hits) 


if __name__ == '__main__':
    import os
    gpio_input_id = int(os.environ.get('GPIO_IN', '170'))
    gpio_output_id = int(os.environ.get('GPIO_OUT', '169'))
    main(gpio_input_id, gpio_output_id)
