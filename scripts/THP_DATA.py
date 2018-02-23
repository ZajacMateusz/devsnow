#!/usr/bin/python
# Copyright (c) 2014 Adafruit Industries
# Author: Tony DiCola

import sys
import Adafruit_DHT
import time
import logging
import smbus
import os
import subprocess

#subprocess.call("python /home/pi/app/scripts/LED.py RED OFF" , shell=True)

LOGS_DIRECTORY = "/media/pi/DATA/logs/thp_data/"

def set_path():

    FILE_NAME = "THP_DATA_%s" % time.strftime("%Y_%m_%d",time.localtime())

    while not os.path.exists(LOGS_DIRECTORY):
        time.sleep(1)

    if not os.path.exists(LOGS_DIRECTORY + FILE_NAME + ".log"):
        file = open(LOGS_DIRECTORY + FILE_NAME + ".log", 'w+')
        #file.close()
    else:
        if not os.path.exists(LOGS_DIRECTORY + "backup"):
            os.mkdir( LOGS_DIRECTORY + "backup", 0775);
        subprocess.call("cp "+ LOGS_DIRECTORY + FILE_NAME+ ".log "+ LOGS_DIRECTORY+ "backup/_"+ FILE_NAME+ ".log " , shell=True)

    logging.basicConfig(filename=LOGS_DIRECTORY + FILE_NAME + '.log', format='%(asctime)s %(message)s', level=logging.INFO)

def get_pressure():
    # Get I2C bus
    bus = smbus.SMBus(1)

    # BMP280 address, 0x77(118)
    # Read data back from 0x88(136), 24 bytes
    b1 = bus.read_i2c_block_data(0x77, 0x88, 24)

    # Convert the data
    # Temp coefficents
    dig_T1 = b1[1] * 256 + b1[0]
    dig_T2 = b1[3] * 256 + b1[2]
    if dig_T2 > 32767 :
        dig_T2 -= 65536
    dig_T3 = b1[5] * 256 + b1[4]
    if dig_T3 > 32767 :
        dig_T3 -= 65536

    # Pressure coefficents
    dig_P1 = b1[7] * 256 + b1[6]
    dig_P2 = b1[9] * 256 + b1[8]
    if dig_P2 > 32767 :
        dig_P2 -= 65536
    dig_P3 = b1[11] * 256 + b1[10]
    if dig_P3 > 32767 :
        dig_P3 -= 65536
    dig_P4 = b1[13] * 256 + b1[12]
    if dig_P4 > 32767 :
        dig_P4 -= 65536
    dig_P5 = b1[15] * 256 + b1[14]
    if dig_P5 > 32767 :
        dig_P5 -= 65536
    dig_P6 = b1[17] * 256 + b1[16]
    if dig_P6 > 32767 :
        dig_P6 -= 65536
    dig_P7 = b1[19] * 256 + b1[18]
    if dig_P7 > 32767 :
        dig_P7 -= 65536
    dig_P8 = b1[21] * 256 + b1[20]
    if dig_P8 > 32767 :
        dig_P8 -= 65536
    dig_P9 = b1[23] * 256 + b1[22]
    if dig_P9 > 32767 :
        dig_P9 -= 65536

    # BMP280 address, 0x77(118)
    # Select Control measurement register, 0xF4(244)
    #       0x27(39)    Pressure and Temperature Oversampling rate = 1
    #                   Normal mode
    bus.write_byte_data(0x77, 0xF4, 0x27)
    # BMP280 address, 0x77(118)
    # Select Configuration register, 0xF5(245)
    #       0xA0(00)    Stand_by time = 1000 ms
    bus.write_byte_data(0x77, 0xF5, 0xA0)

    time.sleep(0.5)

    # BMP280 address, 0x77(118)
    # Read data back from 0xF7(247), 8 bytes
    # Pressure MSB, Pressure LSB, Pressure xLSB, Temperature MSB, Temperature LSB
    # Temperature xLSB, Humidity MSB, Humidity LSB
    data = bus.read_i2c_block_data(0x77, 0xF7, 8)

    # Convert pressure and temperature data to 19-bits
    adc_p = ((data[0] * 65536) + (data[1] * 256) + (data[2] & 0xF0)) / 16
    adc_t = ((data[3] * 65536) + (data[4] * 256) + (data[5] & 0xF0)) / 16

    # Temperature offset calculations
    var1 = ((adc_t) / 16384.0 - (dig_T1) / 1024.0) * (dig_T2)
    var2 = (((adc_t) / 131072.0 - (dig_T1) / 8192.0) * ((adc_t)/131072.0 - (dig_T1)/8192.0)) * (dig_T3)
    t_fine = (var1 + var2)
    cTemp = (var1 + var2) / 5120.0
    fTemp = cTemp * 1.8 + 32

    # Pressure offset calculations
    var1 = (t_fine / 2.0) - 64000.0
    var2 = var1 * var1 * (dig_P6) / 32768.0
    var2 = var2 + var1 * (dig_P5) * 2.0
    var2 = (var2 / 4.0) + ((dig_P4) * 65536.0)
    var1 = ((dig_P3) * var1 * var1 / 524288.0 + ( dig_P2) * var1) / 524288.0
    var1 = (1.0 + var1 / 32768.0) * (dig_P1)
    p = 1048576.0 - adc_p
    p = (p - (var2 / 4096.0)) * 6250.0 / var1
    var1 = (dig_P9) * p * p / 2147483648.0
    var2 = p * (dig_P8) / 32768.0
    pressure = (p + (var1 + var2 + (dig_P7)) / 16.0) / 100
    return pressure


# Un-comment the line below to convert the temperature to Fahrenheit.
# temperature = temperature * 9/5.0 + 32

# Note that sometimes you won't get a reading and
# the results will be null (because Linux can't
# guarantee the timing of calls to read the sensor).
# If this happens try again!

while True:
    set_path()
    read_ok = False

    while not read_ok:
        humidity, temperature = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, 25)
        humidity_2, temperature_2 = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, 23)
        pressure= get_pressure()

        if humidity is not None and temperature is not None and humidity_2 is not None and temperature_2 is not None and pressure is not None:
            #info = 'THP_DATA T_IN={0:0.1f}  H_IN={1:0.1f} T_OUT={2:0.1f}  H_OUT={3:0.1f} P_IN={4:0.2f}'.format(temperature, humidity, temperature_2, humidity_2, pressure)
            info = 'INFO {0:0.1f} {1:0.1f} {2:0.1f} {3:0.1f} {4:0.2f}'.format(temperature, humidity, temperature_2, humidity_2, pressure)
            read_ok = True
        else:
            break

        a= float(time.strftime("%S",time.localtime())) % 5
        while a != 0:
            a= float(time.strftime("%S",time.localtime())) % 5
        logging.info(info)

        #if os.path.exists(LOGS_DIRECTORY + 'THP_DATA_CURRENT.log'):
            #subprocess.call("rm "+ LOGS_DIRECTORY+ "THP_DATA_CURRENT.log ", shell=True)
        file = open(LOGS_DIRECTORY + "THP_DATA_CURRENT.log", 'w+')
        #print(time.strftime("%Y-%m-%d %H:%M:%S,000",time.localtime())+ ' '+ info)
        file.write(time.strftime("%Y-%m-%d %H:%M:%S,000",time.localtime())+ ' '+ info)
        file.close()
        
        