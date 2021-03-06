#!/usr/bin/python3
import threading
import time
import sys
import os

from libsoc import gpio
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtWidgets import *
from PyQt5.uic import loadUi

# define GPIO pins
gpio_input_id = int(os.environ.get('GPIO_IN', '170'))
gpio_output_id = int(os.environ.get('GPIO_OUT', '169'))

# QThread class to process GPIO interrupt
class gpioInterrupt(QThread):
    # LED GPIO status signal
    LedSignal = pyqtSignal(int)
    
    # initial GPIO
    gpio_in = gpio.GPIO(gpio_input_id, gpio.DIRECTION_INPUT)
    gpio_out = gpio.GPIO(gpio_output_id, gpio.DIRECTION_OUTPUT)

    def __init__(self, parent=None):
        super(gpioInterrupt, self).__init__(parent)
        self.working = True

    def __del__(self):
        self.working = False
        self.wait()

    def run(self):
        # GPIO open
        with gpio.request_gpios((self.gpio_in, self.gpio_out)):
            # set LED GPIO out default to 1
            self.gpio_out.set_high()
            assert self.gpio_out.is_high()
            print("The LED initial status is ON")
            
            # loop for processing BUTTON GPIO interrupt
            while self.working == True:
                # start interrput
                self.gpio_in.set_direction(gpio.DIRECTION_INPUT, gpio.EDGE_RISING)
                self.gpio_in.wait_for_interrupt(-1)
                
                # KEY debouncer code
                debounce = 0

                for x in range(3):
                    time.sleep(0.02)
                    if self.gpio_in.is_high() == 1:
                        debounce = debounce + 1

                if debounce >= 2:
                    if self.gpio_out.is_high() == 1:
                        self.gpio_out.set_low()
                        LedStatus = 0
                        # send LED GPIO set to 0 signal
                        self.LedSignal.emit(LedStatus)
                        print("The LED turns OFF")
                    else:
                        self.gpio_out.set_high()
                        LedStatus = 1
                        # send LED GPIO set to 1 signal
                        self.LedSignal.emit(LedStatus)
                        print("The LED turns ON")

# main window QWidget class
class ApplicationWindow(QMainWindow):
    
    def __init__(self):
        QMainWindow.__init__(self)
        # load ui file generated by Qtcreator project
        loadUi('mainwindow.ui', self)
        # LED GPIO current status indicator
        self.LedStatus = 1
        
        # initial relatd button and frame 
        self.Led_Status.setStyleSheet("background-color: rgb(115, 210, 22)")
        self.pushButton_On.setEnabled(False)
        
        # connect Buttion slot
        self.pushButton_On.clicked.connect(self.Button_On_clicked)
        self.pushButton_Off.clicked.connect(self.Button_Off_clicked)
        self.pushButton_Exit.clicked.connect(self.Button_Exit_clicked)
        
        # start the GPIO interrupt thread
        self.thread = gpioInterrupt()
        self.thread.LedSignal.connect(self.LedStatusChange)
        self.thread.start()
    
    # LED frame/label and Push Button processing according to GPIO interrupt thread feedback
    def LedStatusChange(self, LedStat):
        if LedStat == 1:
            self.LedStatus = 1
            self.Led_Status.setStyleSheet("background-color: rgb(115, 210, 22)")
            self.label_On.setText("ON")
            self.pushButton_On.setEnabled(False)
            self.pushButton_Off.setEnabled(True)
            print("button clicked for setting HIGH")
        else:
            self.LedStatus = 0
            self.Led_Status.setStyleSheet("background-color: rgb(255, 255, 255)")
            self.label_On.setText("OFF")
            self.pushButton_Off.setEnabled(False)
            self.pushButton_On.setEnabled(True)
            print("button clicked for setting LOW")
    
    # Push Button On processing
    def Button_On_clicked(self):
        self.gpio_out = self.thread.gpio_out
        
        if self.LedStatus == 0:
            self.thread.gpio_out.set_high()
            self.LedStatus = 1
            self.Led_Status.setStyleSheet("background-color: rgb(115, 210, 22)")
            self.label_On.setText("ON")
            self.pushButton_On.setEnabled(False)
            self.pushButton_Off.setEnabled(True)
            print("set HIGH")
    
    # Push Button Off processing
    def Button_Off_clicked(self):
        self.gpio_out = self.thread.gpio_out
        
        if self.LedStatus == 1:
            self.gpio_out.set_low()
            self.LedStatus = 0
            self.Led_Status.setStyleSheet("background-color: rgb(255, 255, 255)")
            self.label_On.setText("OFF")
            self.pushButton_Off.setEnabled(False)
            self.pushButton_On.setEnabled(True)
            print("set LOW")
    # Push Button Exit processing
    def Button_Exit_clicked(self):
        self.close()
    # Close event processing to exit main program and GPIO interrupt thread accordingly
    def closeEvent(self, event):
        # stop the thread and delete
        if self.thread.isRunning():
            self.thread.quit()
        del self.thread
        super(ApplicationWindow, self).closeEvent(event)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    appWindow = ApplicationWindow()
    appWindow.show()
    sys.exit(app.exec_())
