#!/usr/bin/python
# Copyright (c) 2018 Bartlomiej Sieka
# Author: Mateusz Zajac

import sys, getopt
import time
import smbus
import os
import subprocess

sys.path.append('.')
import RTIMU
import math


#For all scripts and program
MAINLOG_PATH = "/home/pi/app/var/mainlog"
STORAGE_PATH = "/media/pi/DATA"
LOGS_MAIN_DIRECTORY = "logs"
BACKUP_DIRECTORY = "backup"
LOG_EX = ".log"
LOG_EX_BACKUP = "_BACKUP.log"
LED_COMMAND = "python /home/pi/app/scripts/LED.py" 

#Only for this script
SCRIPT_NAME = "IMU_DATA"
LOGS_DIRECTORY = "imu_data"
CURRENT_LOG_NAME = "IMU_DATA_CURRENT"
LOG_NAME = ""
LOGS_PATH = ""
SETTINGS_FILE = "/home/pi/app/scripts/RTIMULib"

logging_on = False
waiting_on = False
sensor_error = False

def write_message(MESSAGE, LABEL_NAME):
    #label name ( INFO, ERROR )
    
    try:
        file = open(MAINLOG_PATH, 'a+')
        if file is None:
            return False
        INFO = " {0}: [{1}]\t{2}\n".format(SCRIPT_NAME, LABEL_NAME, MESSAGE)
        file.write(time.strftime("%G-%m-%d %H:%M:%S",time.localtime()) + INFO)
        file.close()
    except IOError:
        return False
    return True

def write_info(MESSAGE):
    write_message(MESSAGE, "INFO")

def write_error(MESSAGE):
    write_message(MESSAGE, "ERROR")

def set_path():

    global LOG_NAME
    global LOGS_PATH
    global logging_on
    global waiting_on
    USB_error = False

    try:
        file = open(STORAGE_PATH + "/info_file", 'r')
        if not file is None:
            file.close()
    except IOError:
        USB_error = True
    
    if not os.path.exists(STORAGE_PATH) or USB_error:
        if logging_on:
            write_error("USB storage was unplugged.")
            write_error("STOP logging.")
            logging_on = False
        if not waiting_on:
            write_info("Waiting for the USB storage to be connected.")
            waiting_on = True
            subprocess.call("{0} RED ON ".format(LED_COMMAND),  shell=True)
        return False
    
    if waiting_on:
        waiting_on = False
        subprocess.call("{0} RED OFF ".format(LED_COMMAND),  shell=True)

    if not os.path.exists("{0}/{1}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY)):
        time.sleep(0.5) #when usb 
        subprocess.call("mkdir {0}/{1} -m 665".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY) , shell=True)
        if not os.path.exists("{0}/{1}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY)):
            return False
        write_info("Main logs directory was created.")

    if not os.path.exists("{0}/{1}/{2}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY)):
        subprocess.call("mkdir {0}/{1}/{2} -m 665".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY),  shell=True)
        if not os.path.exists("{0}/{1}/{2}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY)):
            return False
        write_info("Thp data logs directory was created.")

    if not os.path.exists("{0}/{1}/{2}/{3}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY, BACKUP_DIRECTORY)):
        subprocess.call("mkdir {0}/{1}/{2}/{3} -m 665".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY, BACKUP_DIRECTORY),  shell=True)
        if not os.path.exists("{0}/{1}/{2}/{3}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY, BACKUP_DIRECTORY)):
            return False
        write_info("Thp data logs backup directory was created.")

    LOG_NAME = "{0}_{1}".format(SCRIPT_NAME, time.strftime("%Y_%m_%d",time.localtime()))
    LOGS_PATH = "{0}/{1}/{2}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY)

    #if os.path.exists("{0}/{1}{2}".format(LOGS_PATH, LOG_NAME, LOG_EX)):
    #    if not logging_on:
    #        write_info("Backup was created: {0}{1}".format(LOG_NAME, LOG_EX_BACKUP))
    #        subprocess.call("cp {0}/{1}{2} {0}/{3}/{1}{4}".format(LOGS_PATH, LOG_NAME, LOG_EX, BACKUP_DIRECTORY, LOG_EX_BACKUP),  shell=True)

    #else:
    #    try:
    #        file = open("{0}/{1}{2}".format(LOGS_PATH, LOG_NAME, LOG_EX), 'w+')
    #        if file is None:
    #            return False
    #        INFO = "DATE\tTIME\tTEMP_IN\tHUM_IN\tTEMP_OUT\tHUM_out\tPRESS\n"
    #        file.write(INFO)
    #        file.close()
    #        write_info("New log file was created: {0}{1}".format(LOG_NAME, LOG_EX))
    #    except IOError:
    #        write_error("IOError: read only system.")
    #        return False

    if not logging_on:
        write_info("START logging.")

    logging_on= True
    return True;

write_info("START script.")
subprocess.call("{0} RED OFF ".format(LED_COMMAND),  shell=True)
subprocess.call("{0} GREEN OFF ".format(LED_COMMAND),  shell=True)

s = RTIMU.Settings(SETTINGS_FILE)
imu = RTIMU.RTIMU(s)

while not imu.IMUInit():
    write_error("Failed initialization IMU")

imu.setSlerpPower(0.02)
imu.setGyroEnable(True)
imu.setAccelEnable(True)
imu.setCompassEnable(True)

poll_interval = imu.IMUGetPollInterval()

while True:
    if not set_path():
        time.sleep(1)
    else:
        read_ok = False
        while not read_ok:
            if imu.IMURead():
                data = imu.getIMUData()
                fusionPose = data["fusionPose"]
                press = data["compass"]

                INFO = "%.2f\t%.2f\t%.2f\t%.2f\n" % (math.degrees(fusionPose[0]), 
                    math.degrees(fusionPose[1]), math.degrees(fusionPose[2]), math.degrees(press[0]))

                try:
                    file = open("{0}/{1}{2}".format(LOGS_PATH, CURRENT_LOG_NAME, LOG_EX) , 'w+')
                    if file is None:
                        break
                    file.write(time.strftime("%G-%m-%d\t%H:%M:%S,000",time.localtime())+ '\t'+ INFO)
                    file.close()
                except Exception:
                    break;
                read_ok = True
           

