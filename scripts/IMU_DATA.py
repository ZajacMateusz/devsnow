
import sys, getopt

sys.path.append('.')
import RTIMU
import os.path
import time
import math
import smbus

SETTINGS_FILE = "RTIMULib"
LOGS_DIRECTORY = "/media/pi/DATA/logs/imu_data/"

s = RTIMU.Settings(SETTINGS_FILE)
imu = RTIMU.RTIMU(s)

if (not imu.IMUInit()):
    sys.exit(1)

imu.setSlerpPower(0.02)
imu.setGyroEnable(True)
imu.setAccelEnable(True)
imu.setCompassEnable(True)
counter= 0;
counter_max= 50;

poll_interval = imu.IMUGetPollInterval()
while True:
    if imu.IMURead():
        data = imu.getIMUData()
        fusionPose = data["fusionPose"]
        press = data["compass"]
        while not os.path.exists(LOGS_DIRECTORY):
            time.sleep(1)
        if(counter* poll_interval)>= counter_max:
            file = open(LOGS_DIRECTORY + "IMU_DATA_CURRENT.log", 'w+')
            file.write(time.strftime("%Y-%m-%d\t%H:%M:%S,000",time.localtime())+ "\t%.2f\t%.2f\t%.2f\t%.2f" % (math.degrees(fusionPose[0]), 
                math.degrees(fusionPose[1]), math.degrees(fusionPose[2]), math.degrees(press[0])))
            file.close()
            counter= 0
        else:
            counter+= 1
        time.sleep(poll_interval*1.0/1000)
        
        
        
