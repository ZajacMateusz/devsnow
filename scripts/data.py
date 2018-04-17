#!/usr/bin/python
# Copyright (c) 2018 Bartlomiej Sieka
# Author: Mateusz Zajac

import time
import os
import subprocess

MAINLOG_PATH = "/home/pi/app/var/mainlog"
STORAGE_PATH = "/media/pi/DATA"
LOGS_MAIN_DIRECTORY = "logs"
BACKUP_DIRECTORY = "backup"
LOG_EX = ".log"
LOG_EX_BACKUP = "_BACKUP.log"
LED_COMMAND = "python /home/pi/app/scripts/LED.py" 
LOG_NAME = ""
LOGS_PATH = ""

logging_on = False
waiting_on = False

def write_message(MESSAGE, NAME, LABEL_NAME):
    #label name ( INFO, ERROR )
    
    try:
        file = open(MAINLOG_PATH, 'a+')
        if file is None:
            return False
        INFO = " {0}: [{1}]\t{2}\n".format(NAME, LABEL_NAME, MESSAGE)
        file.write(time.strftime("%G-%m-%d %H:%M:%S",time.localtime()) + INFO)
        file.close()
    except IOError:
        return False
    return True

def write_info(MESSAGE, NAME):
    write_message(MESSAGE, NAME, "INFO")

def write_error(MESSAGE, NAME):
    write_message(MESSAGE, NAME, "ERROR")

def set_path(LOGS_DIRECTORY, SCRIPT_NAME, backup):

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
            write_error("USB storage was unplugged.", SCRIPT_NAME)
            write_error("STOP logging.", SCRIPT_NAME)
            logging_on = False
        if not waiting_on:
            write_info("Waiting for the USB storage to be connected.", SCRIPT_NAME)
            waiting_on = True
            subprocess.call("{0} RED ON ".format(LED_COMMAND),  shell=True)
        return False
    
    if waiting_on:
        waiting_on = False
        subprocess.call("{0} RED OFF ".format(LED_COMMAND),  shell=True)

    if not os.path.exists("{0}/{1}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY)):
        time.sleep(0.5) #when usb 
        write_info("Main logs directory was created.", SCRIPT_NAME)
        subprocess.call("mkdir {0}/{1} -m 665".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY) , shell=True)
        if not os.path.exists("{0}/{1}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY)):
            return False

    if not os.path.exists("{0}/{1}/{2}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY)):
        write_info("Data logs directory was created.", SCRIPT_NAME)
        subprocess.call("mkdir {0}/{1}/{2} -m 665".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY),  shell=True)
        if not os.path.exists("{0}/{1}/{2}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY)):
            return False

    if not os.path.exists("{0}/{1}/{2}/{3}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY, BACKUP_DIRECTORY)):
        write_info("Data logs backup directory was created.", SCRIPT_NAME)
        subprocess.call("mkdir {0}/{1}/{2}/{3} -m 665".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY, BACKUP_DIRECTORY),  shell=True)
        if not os.path.exists("{0}/{1}/{2}/{3}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY, BACKUP_DIRECTORY)):
            return False

    LOG_NAME = "THP_DATA_{0}".format(time.strftime("%Y_%m_%d",time.localtime()))
    LOGS_PATH = "{0}/{1}/{2}".format(STORAGE_PATH, LOGS_MAIN_DIRECTORY, LOGS_DIRECTORY)

    if backup:
        if os.path.exists("{0}/{1}{2}".format(LOGS_PATH, LOG_NAME, LOG_EX)):
            if not logging_on:
                write_info("Backup was created: {0}{1}".format(LOG_NAME, LOG_EX_BACKUP), SCRIPT_NAME)
                subprocess.call("cp {0}/{1}{2} {0}/{3}/{1}{4}".format(LOGS_PATH, LOG_NAME, LOG_EX, BACKUP_DIRECTORY, LOG_EX_BACKUP),  shell=True)

        else:
            try:
                file = open("{0}/{1}{2}".format(LOGS_PATH, LOG_NAME, LOG_EX), 'w+')
                if file is None:
                    return False
                if SCRIPT_NAME is "THP_DATA": 
                    INFO = "DATE\tTIME\tTEMP_IN\tHUM_IN\tTEMP_OUT\tHUM_out\tPRESS\n"
                    file.write(INFO)
                file.close()
                write_info("New log file was created: {0}{1}".format(LOG_NAME, LOG_EX), SCRIPT_NAME)
            except IOError:
                write_error("IOError: read only system.", SCRIPT_NAME)
                return False

    if not logging_on:
        write_info("START logging.", SCRIPT_NAME)

    logging_on= True
    return True;

