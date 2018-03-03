
#include "../src/gps_module.h"
#include "../src/minmea.h"

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

int gps_set_interface_attribs (int serial_port, int speed, int parity){
        
    struct termios tty;
    memset (&tty, 0, sizeof tty);

    if (tcgetattr (serial_port, &tty) != 0)
    {
        printf("Błąd %d z tcgetattr \n", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (serial_port, TCSANOW, &tty) != 0)
    {
        printf("Błąd %d z tcsetattr \n", errno);
        return -1;
    }
    return 0;
}

bool time_delay(struct timeval  time_start, float delay_in_sec){
    struct timeval  time_now;
    gettimeofday(&time_now, NULL);

    if(((float) (time_now.tv_usec - time_start.tv_usec) / 1000000 + (float) (time_now.tv_sec - time_start.tv_sec)) >= delay_in_sec){
        return true;
    }
       
    return false;
}

int gps_init(char *gps_source, struct functionProgress *f_progress){

    switch((int)(f_progress->progress* 100)){
        case 0:            
            f_progress->message = "Inicjalizacja modułu GPS";

            int serial_port= gps_open_connection(gps_source);
            if(serial_port< 0){
                f_progress->message = "Błąd otwarcia portu szeregowego modułu GPS";
                return 0;
            }

            f_progress->message = "Konfiguracja modułu GPS";              
            gps_set_interface_attribs (serial_port, B9600, 0);
            char message[50];

            strcpy(message, GPS_SET_BAUDRATE_115200); // set gps baut rate to 115200
            write(serial_port, message, strlen(message));
            close(serial_port);

            gettimeofday(&f_progress->start_time, NULL); 
            f_progress->progress= 0.5; 
            break;
        case 50:
            if(time_delay(f_progress->start_time, 1.0)){          
                int serial_port= gps_open_connection(gps_source);
                if(serial_port< 0){
                    f_progress->message = "Błąd otwarcia portu szeregowego modułu GPS";
                    return 0;
                }
                char message[100];
                gps_set_interface_attribs (serial_port, B115200, 0);               
                strcpy(message, GPS_SET_UPDATERATE_5HZ); // set gps update rate to 5 Hz
                write(serial_port, message, strlen(message));
                close(serial_port);

                gettimeofday(&f_progress->start_time, NULL);  
                f_progress->progress= 0.9;
            }
            break;
        case 90:
            if(time_delay(f_progress->start_time, 1.0)){  
                f_progress->message = "Inicjalizacja modułu GPS zakończona";
                f_progress->progress= 1.0;
            }
            break;
    }
    return 1;
}

void gps_print_sentence(char * sentence){
    char c='a';
    int i= 0;
    while(c!='\n'){
        c= *(sentence+i* sizeof(*sentence));
        printf("%c", c);
        ++i;
    } 
}

int gps_read_sentence(int serial_port, char *sentence, int offset_on){

    struct timeval  tv1, tv2;        
    double offset= 0;
    unsigned char c='0';
    gettimeofday(&tv1, NULL);

    while (c!='q'){ 
        if (read(serial_port,&c,1)>0){
            if(c=='$'){
                *sentence= c;
                sentence++;
                while(c!= '\n'){
                    if (read(serial_port,&c,1)>0){
                        *sentence= c;
                        sentence++;       
                    }                                                       
                } 
                c= 'q';                               
            }
        }
        if(offset_on){
            gettimeofday(&tv2, NULL);
            offset= (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec); 
            if(offset> 0.05)
                break; 
        }
        else{
            gettimeofday(&tv2, NULL);
            offset= (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec); 
            if(offset> 1.0)
                break; 
            
        }
    }
    if(c!='q')
        return 0;
    else 
        return 1;
}

int gps_position_data_update(char *sentence, device_data *dev){

	char line[200];
	char c='a';
	int i= 0;
	char h[2], m[2], s[2];
	int microseconds;
	struct minmea_sentence_rmc frame;
	struct minmea_sentence_gga frame;

	while (c != '\n') {
	c = *(sentence + i * sizeof(*sentence));
	line[i++] = c;
	if (c == '\n')
		line[i] = '\0';
	}
	switch (minmea_sentence_id(line, false)) {
		case MINMEA_SENTENCE_RMC: {
			if (minmea_parse_rmc(&frame, line)) {
				dev->position->lat = minmea_tocoord(&frame.latitude);
				dev->position->lon = minmea_tocoord(&frame.longitude);
				dev->position->speed =
					minmea_tofloat(&frame.speed) * GPS_KNOTS_TO_KM_PER_H;
				dev->position->course = minmea_tofloat(&frame.course);
			}
			break;
		}
		case MINMEA_SENTENCE_GGA: {
			if (minmea_parse_gga(&frame, line)) {
				dev->position->alti = minmea_tofloat(&frame.altitude);
				dev->position->fix_quality = frame.fix_quality;
				microseconds = frame.time.microseconds / 10000;
				if (microseconds != 0) {
					microseconds /= 10;
				}
				/* strefa czasowa + 1 */
				if (frame.time.hours < 9) {
					sprintf(h, "0%d", frame.time.hours + 1);
				} else {
					sprintf(h, "%d", frame.time.hours + 1);
				}
				if (frame.time.minutes < 10) {
					sprintf(m, "0%d", frame.time.minutes);
				} else {
					sprintf(m, "%d", frame.time.minutes);
				}
				if (frame.time.seconds < 10) {
					sprintf(s, "0%d", frame.time.seconds);
				} else {
					sprintf(s, "%d", frame.time.seconds);
				}
				sprintf(dev->position->time, "%s:%s:%s:%d0", h, m, s, microseconds );
			}
			break;
		}
		default: {
			/* do nothing */
			break;
		}
	}
	return 0;
}

int gps_read_all(char *gps_source, device_data *dev){
    
    int serial_port= gps_open_connection(gps_source);
    if(serial_port< 0){
        return 0;
    } 

    tcflush(serial_port,TCIOFLUSH);
    char *sentence= malloc(256 * sizeof(*sentence));
    int size_char= sizeof(*sentence);
    int first_sentence= 0;
    int stop = 0;
    int offset= 0;

    while(!stop){
        if(!gps_read_sentence(serial_port, sentence, offset)){
            stop= 1;
            close(serial_port);
            if( first_sentence )
                return 1;
            else
                return 0;
        }
        else{                        
            if(!first_sentence && *(sentence+ 3* size_char) == 'G' && *(sentence+ 4* size_char) == 'G' && *(sentence+ 5* size_char) == 'A'){ 
                first_sentence= offset= 1;
            }
            if( first_sentence ){
		/* output sentence to console for debugging */
                gps_print_sentence(sentence);
                gps_position_data_update(sentence, dev);
            }         
        }
    }
    return 0;
}

