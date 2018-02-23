import sys
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(5,GPIO.OUT)
GPIO.setup(6,GPIO.OUT)

LED_args = { 'GREEN': 5,
                'RED': 6 }
MODE_args = { 'ON': GPIO.HIGH,
                'OFF': GPIO.LOW }     

if len(sys.argv) == 3 and sys.argv[1] in LED_args and sys.argv[2] in MODE_args:
    pin = LED_args[sys.argv[1]]
    mode = MODE_args[sys.argv[2]]
else:
    sys.exit(1)

GPIO.output(pin, mode)