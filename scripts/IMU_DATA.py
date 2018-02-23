
import sys, getopt

sys.path.append('.')
import RTIMU
import os.path
import time
import math
import smbus

SETTINGS_FILE = "RTIMULib"
LOGS_DIRECTORY = "/media/pi/DATA/logs/imu_data/"

print("Using settings file " + SETTINGS_FILE + ".ini")
if not os.path.exists(SETTINGS_FILE + ".ini"):
  print("Settings file does not exist, will be created")

s = RTIMU.Settings(SETTINGS_FILE)
imu = RTIMU.RTIMU(s)

print("IMU Name: " + imu.IMUName())

if (not imu.IMUInit()):
    print("IMU Init Failed")
    sys.exit(1)
else:
    print("IMU Init Succeeded")

# this is a good time to set any fusion parameters

imu.setSlerpPower(0.02)
imu.setGyroEnable(True)
imu.setAccelEnable(True)
imu.setCompassEnable(True)
counter= 0;
counter_max= 50;

poll_interval = imu.IMUGetPollInterval()
print("Recommended Poll Interval: %dmS\n" % poll_interval)
while True:
    if imu.IMURead():
        # x, y, z = imu.getFusionData()
        # print("%f %f %f" % (x,y,z))
        data = imu.getIMUData()
        fusionPose = data["fusionPose"]
        press = data["compass"]
        if(counter* poll_interval)>= counter_max:
            file = open(LOGS_DIRECTORY + "IMU_DATA_CURRENT.log", 'w+')
            #print(time.strftime("%Y-%m-%d %H:%M:%S,000",time.localtime())+ ' '+ info)
            #file.write("r: %.2f p: %.2f y: %.2f m: %f" % (math.degrees(fusionPose[0]), 
            #print(time.strftime("%Y-%m-%d %H:%M:%S,000",time.localtime())+ "\t%.2f\t%.2f\t%.2f\t%.2f" % (math.degrees(fusionPose[0]), 
            #    math.degrees(fusionPose[1]), math.degrees(fusionPose[2]), math.degrees(press[0])))
            file.write(time.strftime("%Y-%m-%d %H:%M:%S,000",time.localtime())+ "\t%.2f\t%.2f\t%.2f\t%.2f" % (math.degrees(fusionPose[0]), 
                math.degrees(fusionPose[1]), math.degrees(fusionPose[2]), math.degrees(press[0])))
            file.close()
            counter= 0
        else:
            counter+= 1
        time.sleep(poll_interval*1.0/1000)
        
        
        