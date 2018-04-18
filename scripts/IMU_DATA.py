#!/usr/bin/python
# Copyright (c) 2018 Bartlomiej Sieka
# Author: Mateusz Zajac

import sys, getopt
import time
import smbus
import os
import subprocess
import RTIMU
import math
import data  

sys.path.append('.')

#Only for this script
SCRIPT_NAME = "IMU_DATA"
LOGS_DIRECTORY = "imu_data"
CURRENT_LOG_NAME = "IMU_DATA_CURRENT"
SETTINGS_FILE = "/home/pi/app/scripts/RTIMULib"

sensor_error = False

data.write_info("START script.", SCRIPT_NAME)
subprocess.call("{0} RED OFF ".format(data.LED_COMMAND),  shell=True)
subprocess.call("{0} GREEN OFF ".format(data.LED_COMMAND),  shell=True)

s = RTIMU.Settings(SETTINGS_FILE)
imu = RTIMU.RTIMU(s)

while not imu.IMUInit():
    data.write_error("Failed initialization IMU", SCRIPT_NAME)
    sleep(0.5)

imu.setSlerpPower(0.02)
imu.setGyroEnable(True)
imu.setAccelEnable(True)
imu.setCompassEnable(True)

poll_interval = imu.IMUGetPollInterval()

while True:
    if not data.set_path(LOGS_DIRECTORY, SCRIPT_NAME, False):
        time.sleep(1)
    else:
        read_ok = False
        while not read_ok:
            if imu.IMURead():
                data_imu = imu.getIMUData()
                fusionPose = data_imu["fusionPose"]
                press = data_imu["compass"]

                INFO = "%.2f\t%.2f\t%.2f\t%.2f\n" % (math.degrees(fusionPose[0]), 
                    math.degrees(fusionPose[1]), math.degrees(fusionPose[2]), math.degrees(press[0]))

                file = open("{0}/{1}{2}".format(data.LOGS_PATH, CURRENT_LOG_NAME, data.LOG_EX), 'w+')
                try:
                    if file is None:
                        break
                    file.write(time.strftime("%G-%m-%d\t%H:%M:%S,000",time.localtime())+ '\t'+ INFO)
                    file.close()
                except Exception:
                    break;
                read_ok = True
            else:
                time.sleep(0.01)
           

