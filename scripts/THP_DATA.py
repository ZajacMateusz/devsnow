#!/usr/bin/python
# Copyright (c) 2018 Bartlomiej Sieka
# Author: Mateusz Zajac

import sys
import Adafruit_DHT
import time
import smbus
import os
import subprocess
import data 

SCRIPT_NAME = "THP_DATA"
LOGS_DIRECTORY = "thp_data"
CURRENT_LOG_NAME = "THP_DATA_CURRENT"
DHT_IN_PIN = 25
DHT_OUT_PIN = 23
TIME_LOG_IN_S = 5

sensor_error = False

data.write_info("START script.", SCRIPT_NAME)
subprocess.call("{0} RED OFF ".format(data.LED_COMMAND), shell=True)
subprocess.call("{0} GREEN OFF ".format(data.LED_COMMAND), shell=True)

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


while True:
    if not data.set_path(LOGS_DIRECTORY, SCRIPT_NAME, True):
        time.sleep(1)
    else:
        read_ok = False

        while not read_ok:
            humidity_in, temperature_in = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, DHT_IN_PIN, 4, 1)
            if humidity_in is None or temperature_in is None:
                if not sensor_error:
                    data.write_error("The internal temperature and humidity sensor is not responding.", SCRIPT_NAME)
                    data.write_error("STOP logging.", SCRIPT_NAME)
                    sensor_error = True
                break

            humidity_out, temperature_out = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, DHT_OUT_PIN, 4, 1)
            if humidity_out is None or temperature_out is None:
                if not sensor_error:
                    data.write_error("The external temperature and humidity sensor is not responding.", SCRIPT_NAME)

                    data.write_error("STOP logging.", SCRIPT_NAME)

                    sensor_error = True
                break
            
            if sensor_error:
                sensor_error = False
                data.write_info("Reading from the temperature and humidity sensors is correct.", SCRIPT_NAME)
                data.write_info("START logging.", SCRIPT_NAME)

            pressure= get_pressure()

            if humidity_in is not None and humidity_out is not None and temperature_in is not None and temperature_out is not None and pressure is not None:
                INFO = 'INFO\t{0:0.1f}\t{1:0.1f}\t{2:0.1f}\t{3:0.1f}\t{4:0.2f}\n'.format(temperature_in, humidity_in, temperature_out, humidity_out, pressure)
                read_ok = True
            else:
                break

            sec= float(time.strftime("%S",time.localtime())) % TIME_LOG_IN_S 
            while sec != 0:
                time.sleep(0.2)
                sec= float(time.strftime("%S",time.localtime())) % TIME_LOG_IN_S
            if not os.path.exists(data.STORAGE_PATH):
                break                
            try:
                file = open("{0}/{1}{2}".format(data.LOGS_PATH, CURRENT_LOG_NAME, data.LOG_EX) , 'w+')
                if file is None:
                    break
                file.write(time.strftime("%G-%m-%d\t%H:%M:%S,000",time.localtime())+ '\t'+ INFO)
                file.close()
            except Exception:
                break;

            try:
                file = open("{0}/{1}{2}".format(data.LOGS_PATH, data.LOG_NAME, data.LOG_EX), 'a+')
                if file is None:
                    break
                file.write(time.strftime("%G-%m-%d\t%H:%M:%S,000",time.localtime())+ '\t'+ INFO)
                file.close()
            except Exception:
                break


