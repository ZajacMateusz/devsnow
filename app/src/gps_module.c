
#include "gps_module.h"
#include "minmea.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <glib.h>

int gps_open_connection(char * serial_port_src){
    
	return open(serial_port_src , O_RDWR | O_NOCTTY | O_NDELAY);
}

int gps_set_interface_attribs(int serial_port, int speed, int parity){

	struct termios tty;
	memset (&tty, 0, sizeof tty);

	if (tcgetattr (serial_port, &tty) != 0)
	{
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
							// disable IGNBRK for mismatched speed tests; otherwise receive break
							// as \000 chars
	tty.c_iflag &= ~IGNBRK;				// disable break processing
	tty.c_lflag = 0;				// no signaling chars, no echo,
							// no canonical processing
	tty.c_oflag = 0;				// no remapping, no delays
	tty.c_cc[VMIN]  = 0;				// read doesn't block
	tty.c_cc[VTIME] = 1;				// 0.1 seconds read timeout
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);		// shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);		// ignore modem controls,
                                        		// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);		// shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (serial_port, TCSANOW, &tty) != 0)
	{
		printf("Błąd %d z tcsetattr \n", errno);
		return -1;
	}
	return 1;
}

int gps_init(char *gps_source){

	int serial_port= gps_open_connection(gps_source);
	char sentence[NMEA_SENTENCE_MAX_LENGTH];

	if (serial_port < 0){
		return -1;
	}

	gps_set_interface_attribs (serial_port, B9600, 0);
	strcpy(sentence, GPS_SET_BAUDRATE_115200); // set gps baut rate to 115200
	write(serial_port, sentence, strlen(sentence));

	usleep(GPS_INIT_TIME_IN_US);
	           
	gps_set_interface_attribs (serial_port, B115200, 0);               
	strcpy(sentence, GPS_SET_UPDATERATE_5HZ); // set gps update rate to 5 Hz
	write(serial_port, sentence, strlen(sentence));

	usleep(GPS_INIT_TIME_IN_US);
	tcflush(serial_port,TCIOFLUSH);
	if (gps_read(serial_port, NULL, GPS_READ_MAX_TIME_IN_S) != 1){
		close(serial_port);
		return 0;
	}
	close(serial_port);
	return 1;
}

void gps_print_sentence(char *sentence){

	char c = 'a';
	int i = 0;
	while (c != '\n' && i < NMEA_SENTENCE_MAX_LENGTH){
		c = *(sentence + i * sizeof(*sentence));
		printf("%c", c);
		++i;
	} 
}

int gps_position_data_update(char *sentence, nmea *position){

	struct minmea_sentence_rmc frame_rmc;
	struct minmea_sentence_gga frame_gga;
	bool update = false;

	switch (minmea_sentence_id(sentence, false)) {
		case MINMEA_SENTENCE_RMC: {
			if (minmea_parse_rmc(&frame_rmc, sentence)) {
				position->lat = minmea_tocoord(&frame_rmc.latitude);
				position->lon = minmea_tocoord(&frame_rmc.longitude);
				position->speed = minmea_tofloat(&frame_rmc.speed) * GPS_KNOTS_TO_KM_PER_H;
				position->course = minmea_tofloat(&frame_rmc.course);
				update = true;
			}
			break;
		}
		case MINMEA_SENTENCE_GGA: {
			if (minmea_parse_gga(&frame_gga, sentence)) {
				position->alt = minmea_tofloat(&frame_gga.altitude);
				position->fix_quality = frame_gga.fix_quality;
				/* XXX don't know why, but need to divide ...*/
				frame_gga.time.microseconds /= 10000;
				/* strefa czasowa + 1 */
				frame_gga.time.hours += 2;
				if (frame_gga.time.hours > 24){
					frame_gga.time.hours = frame_gga.time.hours - 24;
				}
				sprintf(position->time, "%02i:%02i:%02i:%02i",
					frame_gga.time.hours,
					frame_gga.time.minutes,
					frame_gga.time.seconds,
					frame_gga.time.microseconds);
				update = true;
			}
			break;
		}
		default: {
			/* do nothing */
			break;
		}
	}
	if (update == true){
		return 1;
	}
	return 0;
}


int gps_read(int serial_port, nmea *position, double TIMEOUT_IN_S){

	bool sentence_find = false;
	bool timeout_set = false;

	if(serial_port< 0){
		return -1;
	}
	// Initialize file descriptor sets
	fd_set read_fds, write_fds, except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(serial_port, &read_fds);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = TIMEOUT_IN_S * 1000000;

	if (select(serial_port + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1){

		timeout.tv_usec = 0.02 * 1000000;
		do {
			char *sentence= malloc(NMEA_SENTENCE_MAX_LENGTH * sizeof(*sentence));
			char *begin = sentence;
			bool first_char_find = false;
			do {
				if (first_char_find != true){
					if (select(serial_port + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1){
						if (read(serial_port, sentence, sizeof(*sentence)) > 0 && *sentence == '$'){

							first_char_find = true;
							sentence++;
						}
					} else {
						timeout_set = true;
						tcflush(serial_port, TCIOFLUSH);
						break;
					}
				} else {
					if (read(serial_port, sentence, sizeof(*sentence)) > 0){
						sentence++;
					} else {
						usleep(100);
					}
				}
			} while (!(*(sentence- sizeof(*sentence)) == '\n' && first_char_find) == true);

			*sentence = '\0';

			if (timeout_set == false && minmea_check(begin, false)){
				sentence_find = true;
				if (position != NULL){
					gps_position_data_update(begin, position);
				}
			}

		} while (timeout_set == false);
	} else {
		return 0;
	}

	if (sentence_find == true){
		return 1;
	}
	return 0;
}


