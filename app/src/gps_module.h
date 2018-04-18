
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

#define GPS_SET_BAUDRATE_115200		"$PMTK251,115200*1F\r\n"
#define GPS_SET_UPDATERATE_5HZ		"$PMTK220,200*2C\r\n"
#define GPS_SET_UPDATERATE_5H_REPLY	"$PMTK,251,2*37\r\n"
#define GPS_KNOTS_TO_KM_PER_H		1.852
#define GPS_READ_MAX_TIME_IN_S		0.2
#define GPS_INIT_TIME_IN_US		1000000
#define CHAR_TIME_9600_IN_US		1042
#define CHAR_TIME_115200_IN_US		87
#define NMEA_SENTENCE_MAX_LENGTH	100

//
// NMEA doc:
// http://www.tronico.fi/OH6NT/docs/NMEA0183.pdf
//

/**
* Open the serial port connection.
*/

int gps_open_connection(char * serial_port_src);
//Returns the file descriptor (nonnegative integer) or -1 if an error occurs.

/**
* Set the serial port attributes.
*/

int gps_set_interface_attribs (int serial_port, int speed, int parity);
//Returns 1 if it succeseeds or -1 if an error occurs.

/**
* Set the gps baudrate and update rate.
*/

int gps_init(char *gps_source);
//Return 1 if it succeseeds, 0 if not or -1 if there is an access error to teh serial.

/**
* Print NMEA raw sentence to the console.
*/

void gps_print_sentence(char * sentence);

/**
* Update the position data
*/

int gps_position_data_update(char *sentence, nmea *position);
//Return 1 if updated or 0 if not

/**
* Read all sentences available in one time period
*/

int gps_read(int gps_serial, nmea *position, double TIMEOUT_IN_S);
//Return 1 if it succeseeds, 0 if there is no data to read 
//or -1 if there is an access error to teh serial.


#endif
