
#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <glib.h>

struct functionProgress{
    float progress;
    char *message;
    struct timeval start_time;
    int error_code;
};

#define GPS_SET_BAUDRATE_115200		"$PMTK251,115200*1F\r\n"
#define GPS_SET_UPDATERATE_5HZ		"$PMTK220,200*2C\r\n"
#define GPS_KNOTS_TO_KM_PER_H		1.852


bool time_delay(struct timeval  time_start, float delay_in_sec);

/**
* Open the serial port connection.
*/

int gps_open_connection(char * serial_port_src);

/**
* Set the serial port attributes.
*/

int gps_set_interface_attribs (int serial_port, int speed, int parity);

/**
* Set the gps baudrate and update rate.
  
  ERROR_CODE
  0 - ok
  1 - Błąd otwarcia portu szeregowego modułu GPS
*/

int gps_init( char *gps_source, struct functionProgress *f_progress);



/**
* Print NMEA raw sentence to the console.
*/

void gps_print_sentence(char * sentence);

/**
* Read one sentence from the serial port.
*/

int gps_read_sentence(int serial_port, char *sentence, int offset_on);

/**
* Update the position data
*/

int gps_position_data_update(char *sentence, device_data *dev);

/**
* Read all sentences available in one time period
*/

int gps_read_all(char *gps_source, device_data *dev);


#endif