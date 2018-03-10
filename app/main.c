/*
 * main.c
 * Copyright (C) Bartłomiej Sięka 2018
 * Written by Mateusz Zając <zajac382@gmail.com>
*/

 /* ******************* include *************************** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "osm-gps-map.h"
#include "src/def.h"

#include "src/minmea.h"
#include "src/gps_module.h"
#include "src/ui_library.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>

/* ******************* variables **************************** */

int init_module= 3;
struct timeval  function_start;

GtkBuilder * builder;
GtkWidget * main_window;
GtkWidget *map_w;
OsmGpsMap *map;
OsmGpsMapLayer *osd;

int rc;
struct timeval tv;

device_data *device;
/* ******************* function  ******************* */

static
void save_log_to_file(device_data * device){

	FILE *fp;
	char src[200];
	printf(src, "%sDEVICE_DATA_%s.log", DEVICE_DATA_LOG_SRC, device->position->date);

	if( access(src, F_OK) == -1) {
		fp = fopen(src, "a");
		fprintf(fp, "time\tlatitude\tlongitude\taltitude\tspeed\tsnow_depth\ttemperature\thumidity\tpressure\trotate_1\trotate_2\n");
		fclose(fp);
	}
	fp = fopen(src, "a");

	fprintf(fp, "%s\t%f\t%f\t%f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", device->position->time, device->position->lat,
	device->position->lon, device->position->alt, device->position->speed, device->snow_depth, 
	device->temperature, device->humidity, device->pressure, device->imu_data->r, device->imu_data->p );

	fclose(fp);
}

static int
ui_read_temp_and_hum(void){

	char temp[100];
	float tem;
	float hum;
	float press;
	FILE *fptr;

	char file_name[100];
	sprintf(file_name, "%s", ATMOSPHERIC_CONDITIONS_LOG_SRC);

	if ((fptr = fopen(file_name, "r")) == NULL){
		return 0;
	}

	bool read_ok = false;
	for(int i= 0; i < 8; ++i){
		if(fscanf(fptr,"%s", temp) == EOF)
			break;
		switch(i){
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				tem = atof(temp);
				break;
			case 6:
				hum = atof(temp);
				break;
			case 7:
				press = atof(temp);
				break;
		}
		read_ok = true;
	}

	if(read_ok == 1){
		device->temperature = tem;
		device->humidity = hum;
		device->pressure = press;
		return 1;
	}
	return 0;
}


static int
read_imu_data(imu *imu_data, char *src_file){

	imu *imu_temp = malloc(sizeof(imu));
	FILE *fptr;

	if ((fptr = fopen(src_file,"r")) == NULL){
		return 0;
	}

	char temp[100];
	bool read_ok= false;

	for(int i= 0; i < 8; ++i){
		if(fscanf(fptr,"%s", temp) == EOF)
			break;
		switch(i){
			case 0:
				break;
			case 1:
				break;
			case 2:
				imu_temp->r = atof(temp);
				break;
			case 3:
				imu_temp->p = atof(temp);
				break;
			case 4:
				imu_temp->y = atof(temp);
				break;
			case 5:
				imu_temp->m = atof(temp);
				break;
		}
		read_ok = true;
	}

	if(read_ok == 1){
		*imu_data = *imu_temp;
		return 1;
	}
	return 0;
}

static void
calculate_imu_data(imu *imu_data, imu *imu_zero){

	float r, p;
	r = imu_data->r;
	if(r < 0)
	r = 360.0+ r;
	r = (r- imu_zero->r) * -1;
	if(r > -0.01 && r < 0.01)
	r = 0.0;
	imu_data->r = r;
	p = imu_data->p;
	p = (p- imu_zero->p) * -1;
	if(p > -0.01 && p < 0.01)
		p = 0.0;
	imu_data->p = p;
}

/* ************  UI button function  ******************* */


static void
ui_map_set_center(void){

	osm_gps_map_set_center_and_zoom (map, device->position->lat, device->position->lon, 20);
}

static void
print_error(char *message){

	printf("\nERROR: %s\n\n", message);
	gtk_main_quit();
}

int function_init_counter = 0;
struct functionProgress *f_progress;

static bool
timer_handler(void){

	switch(function_init_counter){
		case 0:
			++function_init_counter;
			break;
		case 1:
			if(f_progress->progress < 1.0){
				if(gps_init( GPS_SERIAL_SOURCE, f_progress) == 1)
					set_loading_module_progress(builder, (f_progress->progress + (float)function_init_counter - 1) / (float)init_module, f_progress->message);
				else{
					print_error(f_progress->message);
					return false;
				}
			}
			else
				++function_init_counter;
			break;
		case 2:
			f_progress->message = "Oczekiwanie na ustalenie pozycji geograficznej";
			f_progress->progress = 0.5;
			set_loading_module_progress(builder, (f_progress->progress + (float)function_init_counter - 1) / (float)init_module, f_progress->message);

			if(gps_read_all(GPS_SERIAL_SOURCE, device) == 1){
				if(device->position->fix_quality >= 1){
					if(device->position->lat != 0 && device->position->lon != 0){
						osm_gps_map_set_center_and_zoom (map, device->position->lat, device->position->lon, 20);
					}
					f_progress->progress = 0.0;
					++function_init_counter;
				}
			}
			else{
				f_progress->progress = 0;
				--function_init_counter;
			}
			break;
		case 3:
		{
			f_progress->message = "Odczyt danych z czujników";
			set_loading_module_progress(builder, (f_progress->progress + (float)function_init_counter - 1) / (float)init_module, f_progress->message);
			if(f_progress->progress < 1.0){
				GDateTime *date_time;
				date_time = g_date_time_new_now_local();                        // get local time
				device->sys_time = g_date_time_format(date_time, "%H:%M:%S");
				int sec = atof(g_date_time_format(date_time, "%S"));

				if(sec % 5 == 2){
					ui_read_temp_and_hum();
					f_progress->progress = 1.0;
					set_loading_module_progress(builder, (f_progress->progress + (float)function_init_counter - 1) / (float)init_module, f_progress->message);
				}
			}
			else{
				on_set_information(builder, device);
				++function_init_counter;
			}
			break;
		}
		case 4:
		{
			f_progress->message = "Konfiguracja interfejsu";
			set_loading_module_progress(builder, (f_progress->progress + (float)function_init_counter - 1) / (float)init_module, f_progress->message);
			GtkWidget * start_window;
			start_window = GTK_WIDGET( gtk_builder_get_object(builder, "start_window" ));
			gtk_widget_destroy(GTK_WIDGET(start_window));
			++function_init_counter;
			break;
		}
		case 5:
		{
			/* ************** THP DATA *************************** */
			GDateTime *date_time;
			date_time = g_date_time_new_now_local();                        // get local time
			device->sys_time = g_date_time_format(date_time, "%H:%M:%S");

			int sec = atof(g_date_time_format(date_time, "%S"));

			if(sec % 5 == 2)
				ui_read_temp_and_hum();

			read_imu_data(device->imu_data, IMU_LOG_SRC);
			calculate_imu_data(device->imu_data, device->imu_zero);

			/* ************** GPS *************************** */
			int gps_read_status;
			gps_read_status = gps_read_all(GPS_SERIAL_SOURCE, device);

			if(device->position->fix_quality >= 1){
				if(device->position->lat != 0 && device->position->lon != 0){
					osm_gps_map_image_remove_all (map);
					GdkPixbuf * image;
					image = gdk_pixbuf_new_from_file (DEVICE_MAP_MARKER_SRC, NULL);
					if(image != NULL){
						osm_gps_map_gps_clear (map);
						osm_gps_map_gps_add (map, device->position->lat, device->position->lon, device->position->course);
					}

					osm_gps_map_set_center_and_zoom ( map, device->position->lat, device->position->lon, 18);
					ui_set_flag(GPS_ICON_FLAG);
					ui_gps_icon_change( builder );
					save_log_to_file(device);

				}
			}
			else{
				ui_reset_flag(GPS_ICON_FLAG);
				ui_gps_icon_change( builder );
			}
			on_set_information(builder, device);
			break;
		}
	}
	return TRUE;
}

static void 
device_data_init_null(device_data *dev){

	dev->position = malloc(sizeof(nmea));
	dev->position->lat = 0;
	dev->position->lon = 0;
	dev->position->speed = 0;
	dev->position->course = 0;
	dev->position->alt = 0;
	dev->temperature = 0;
	dev->snow_depth = 0.00;
	dev->imu_data = malloc(sizeof(imu));
	dev->imu_data->r = 0;
	dev->imu_data->p = 0;
	dev->imu_data->y = 0;
	dev->imu_data->m = 0;
	dev->imu_zero = malloc(sizeof(imu));
}

/* ************** main *************************** */

int
main(int argc, char * argv[]){

	f_progress = malloc(sizeof(struct functionProgress));
	f_progress->progress = 0.0;
	f_progress->message = "none";

	device = malloc(sizeof(device_data));
	device_data_init_null(device);


	GDateTime *date_time;
	date_time = g_date_time_new_now_local();
	sprintf(device->position->date, "%s", g_date_time_format(date_time, "%Y_%m_%d"));

	gtk_init(& argc, & argv);

	builder = gtk_builder_new();
	create_builder(builder);

	main_window = create_window(builder, UI_APP_MAIN_WINDOW);

	GtkWidget * start_window;
	start_window = GTK_WIDGET( gtk_builder_get_object( builder, UI_APP_START_WINDOW));

	gtk_window_fullscreen(GTK_WINDOW(start_window));

	GdkScreen *screen;
	screen = gdk_screen_get_default();

	GdkDisplay *display;
	display = gdk_screen_get_display(screen);

	GdkWindow* main_win = gtk_widget_get_window((main_window));
	GdkCursor *Cursor;

	Cursor = gdk_cursor_new_for_display (display, GDK_BLANK_CURSOR);
	gdk_window_set_cursor((main_win),Cursor);

	GtkImage *image = (GtkImage*)gtk_builder_get_object(builder, "img_start_background");
	GdkPixbuf * image_src;
	image_src = gdk_pixbuf_new_from_file (START_BACKGROUND_SRC, NULL);
	gtk_image_set_from_pixbuf(image, image_src);

	OsmGpsMapSource_t source = OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE;

	if ( !osm_gps_map_source_is_valid(source) )
		return 1;

	g_strdup(OSM_GPS_MAP_CACHE_DISABLED);

	map = g_object_new (OSM_TYPE_GPS_MAP,
			"map-source", source,
			"tile-cache", "/tmp/",
			NULL);

	gtk_box_pack_start (GTK_BOX(gtk_builder_get_object(builder, "box_map")),
			GTK_WIDGET(map), TRUE, TRUE, 0);

	gtk_widget_show_all (main_window);
	gtk_widget_show_all (start_window);
	ui_init(builder);

	/* ****************** signal connect *********************** */

	g_signal_connect(gtk_builder_get_object(builder, "bt_set_center"), "clicked", G_CALLBACK(ui_map_set_center), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_show_snow_depth"), "clicked", G_CALLBACK(ui_on_view_snow_depth), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_show_coordinates"), "clicked", G_CALLBACK(ui_on_view_coordinates), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_show_gps_more_information"), "clicked", G_CALLBACK(ui_on_view_gps_more_information), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_view_menu"), "clicked", G_CALLBACK(ui_on_view_menu), builder);

	g_timeout_add(200, (GSourceFunc)timer_handler, builder);
	gtk_main();
	return 0;
}
