/*
 * main.c
 * Copyright (C) Bartłomiej Sięka 2018
 * Author: Mateusz Zając <zajac382@gmail.com>
*/

 /* ******************* include *************************** */

#include <fcntl.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <math.h>
#include "osm-gps-map.h"
#include "src/def.h"
#include "src/gps_module.h"
#include "src/minmea.h"
#include "src/ui_library.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

/* ******************* global variables **************************** */

bool imu_data_zero_load = false;
device_data *device;
GtkBuilder *builder;
GtkWidget *main_window;
GtkWidget *map_w;
int gps_serial = 0;
int rc;
int turn_first_timer = 0;
OsmGpsMap *map;
struct timeval tv;

/* ******************* functions  ******************* */

static int
save_log_to_file(device_data *device){

	char *file_name = malloc(strlen(device->position->date) + strlen(SCRIPT_NAME) +strlen(LOG_EX) + 1);
	char *path = malloc(strlen(STORAGE_PATH) + strlen(TEST_FILE_NAME)+1);
	FILE *file;
	struct stat sb;

	sprintf(file_name, "%s_%s%s", SCRIPT_NAME, device->position->date, LOG_EX);
	sprintf(path, "%s/%s", STORAGE_PATH, TEST_FILE_NAME);

	file = fopen(path, "r");
	free(path);
	if (file == NULL){
		return 0;
	} else {	
		fclose(file);
		path = malloc(strlen(STORAGE_PATH) + strlen(LOGS_MAIN_DIRECTORY) + strlen(LOGS_DIRECTORY) + 3 + strlen(file_name));
		sprintf(path, "%s/%s", STORAGE_PATH, LOGS_MAIN_DIRECTORY);

		if (stat(path, &sb) == -1){
			mkdir(path, 0777);
			if (stat(path, &sb) == -1){
				free(path);
				return -1;
			}
		} 

		sprintf(path, "%s/%s", path, LOGS_DIRECTORY);

		if (stat(path, &sb) == -1){
			mkdir(path, 0777);
			if (stat(path, &sb) == -1){
				printf("Folder nie istnieje.%s\n", path);
				free(path);
				return -1;
			}
		}

		sprintf(path, "%s/%s", path, file_name);

		if (access(path, F_OK) == -1) {
			file = fopen(path, "a+");
			free(path);
			if ( file != NULL){
				fprintf(file, "time\tlatitude\tlongitude\taltitude\tspeed\tsnow_depth\ttemperature\thumidity\tpressure\trotate_1\trotate_2\n");
				fclose(file);
			} else {
				return -1;
			}
		}

		file = fopen(path, "a+");
		free(file_name);
		free(path);
		if (file != NULL){
			fprintf(file, "%s\t%f\t%f\t%f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", device->position->time, device->position->lat,
					device->position->lon, device->position->alt, device->position->speed, device->snow_depth, 
					device->temperature, device->humidity, device->pressure, device->imu_data->r, device->imu_data->p );
			fclose(file);
			return 1;
		}
		return 0;
		
	}
	return 1;
}

static int
print_log(char *time, char *message, char label){
	
	char *lab;
	char *path = malloc(strlen(APP_LOG_PATH));
	FILE *file;

	switch (label){
		case 'E':
			lab = "ERROR";
			break;
		case 'I':
			lab = "INFO";
			break;
		default:
			lab = "INFO";
			break;
	}

	sprintf(path, "%s", APP_LOG_PATH);

	file = fopen(path, "a+");
	free(path);
	if (file == NULL){
		return 0;
	} else {
		fprintf(file, "%s [%s]\t%s\n", time, lab, message);
		fclose(file);
	}
	return 1;
}

static int
ui_read_temp_and_hum(void){

	bool read_ok = false;
	char *temp = malloc(30);
	char *file_name = malloc(strlen(ATMOSPHERIC_CONDITIONS_LOG_SRC));
	float tem, hum, press;
	FILE *fptr;

	sprintf(file_name, "%s", ATMOSPHERIC_CONDITIONS_LOG_SRC);

	if ((fptr = fopen(file_name, "r")) == NULL){
		free(temp);
		free(file_name);
		return 0;
	}

	for (int i = 0; i < 8; ++i){
		if (fscanf(fptr,"%s", temp) == EOF){
			break;
		}
		switch (i){
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
			defaut:
				// Do nothing.
				break;
		}
		read_ok = true;
	}
	free(temp);
	free(file_name);
	fclose(fptr);
	if (read_ok == 1){
		device->temperature = tem;
		device->humidity = hum;
		device->pressure = press;
		return 1;
	}
	return 0;
}

static int
read_imu_data(imu *imu_data, char *src_file){

	bool read_ok = false;
	char *temp = malloc(30);
	FILE *fptr;
	imu *imu_temp = malloc(sizeof(imu));

	for (int i = 0; i< 3; ++i){
		if ((fptr = fopen(src_file, "r")) != NULL){
			for (int i = 0; i < 6; ++i){
				if (fscanf(fptr, "%s", temp) == EOF){
					break;
				}
				switch (i){
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
					defaut:
						// Do nothing.
						break;
				}
				read_ok = true;
			}
			fclose(fptr);
		}
		if (read_ok == true){
			break;
		}
		usleep(10000);
	}
	free(temp);
	free(imu_temp);

	if (read_ok == 1){
		*imu_data = *imu_temp;
		return 1;
	}
	return 0;
}

static void
calculate_imu_data(imu *imu_data, imu *imu_zero){

	float r, p;

	r = imu_data->r;
	if (r < 0){
		r = 360.0 + r;
	}
	r = (r - imu_zero->r) * (-1);
	if (r > (-0.01) && r < 0.01){
		r = 0.0;
	}
	imu_data->r = r;
	p = imu_data->p;
	p = (p - imu_zero->p) * (-1);
	if (p > (-0.01) && p < 0.01){
		p = 0.0;
	}
	imu_data->p = p;
}

static void
ui_map_set_center(void){

	osm_gps_map_set_center_and_zoom(map, device->position->lat, device->position->lon, 20);
}

static bool
timer_handler(void){

	GDateTime *date_time = g_date_time_new_now_local();  
	device->sys_time = g_date_time_format(date_time, "%H:%M:%S");

	//Read temperature and others every 5 seconds with offset by one.
	if (atoi(g_date_time_format(date_time, "%S")) % 5 == 1) {
		if (ui_read_temp_and_hum() != 1){
			print_log(device->sys_time, "Reading temperature, humidity and pressure.", 'E');
		}	
	}

	if (imu_data_zero_load == false){
		if (read_imu_data(device->imu_zero, IMU_ZERO_LOG_SRC) != 1){
			print_log(device->sys_time, "Reading from the IMU zero config.", 'E');
		} else {
			imu_data_zero_load = true;
		}
	}
	if (read_imu_data(device->imu_data, IMU_LOG_SRC) != 1){
		print_log(device->sys_time, "Reading from the IMU data file.", 'E');
	} else {
		calculate_imu_data(device->imu_data, device->imu_zero);
	}

	/* ************** GPS *************************** */
	if (gps_read(gps_serial, device->position, 0.15) != 1){
		print_log(device->sys_time, "Read all GPS sentence.", 'E');
	} else {

		if (device->position->fix_quality >= 1){
			if (device->position->lat != 0 && device->position->lon != 0){
				osm_gps_map_image_remove_all(map);
				osm_gps_map_gps_clear(map);
				osm_gps_map_gps_add(map, device->position->lat, device->position->lon, device->position->course);
				//osm_gps_map_set_center_and_zoom ( map, device->position->lat, device->position->lon, 18);
				ui_set_flag(GPS_ICON_FLAG);
				ui_gps_icon_change(builder);
				save_log_to_file(device);
			}
		} else {
			ui_reset_flag(GPS_ICON_FLAG);
			ui_gps_icon_change(builder);
		}
	}

	on_set_information(builder, device);

	if (turn_first_timer == 1  && device->position->fix_quality >= 1){  
		GtkWidget *start_window;
		start_window = GTK_WIDGET(gtk_builder_get_object(builder, "start_window"));
		gtk_widget_destroy(GTK_WIDGET(start_window));
		turn_first_timer = 0;
	}
	date_time = NULL;
	free(date_time);
	return true;
}

static bool
timer_handler_init(void){

	switch (turn_first_timer){
		case 0:
		{
			GDateTime *date_time = g_date_time_new_now_local();  
			device->sys_time = g_date_time_format(date_time, "%H:%M:%S");
			print_log(device->sys_time, "Start application.", 'I');	
			++turn_first_timer;
			break;
		}
		case 1:	
			print_log(device->sys_time, "Initiation of the GPS module...", 'I');	
			if (gps_init(GPS_SERIAL_SOURCE) == 1){
				gps_serial = gps_open_connection(GPS_SERIAL_SOURCE);
				g_timeout_add(200, (GSourceFunc)timer_handler, builder);
				print_log(device->sys_time, "completed.", 'I');	
				return false;
			}
			print_log(device->sys_time, "falied. ", 'E');	
			break;
		default:
		       	// Do nothing.
			break;
	}	
	return true;
}

static void 
device_data_init_null(device_data *dev){

	dev->imu_data = malloc(sizeof(imu));
	dev->imu_data->r = 0;
	dev->imu_data->p = 0;
	dev->imu_data->y = 0;
	dev->imu_data->m = 0;
	dev->imu_zero = malloc(sizeof(imu));
	dev->imu_zero->r = 0;
	dev->imu_zero->p = 0;
	dev->imu_zero->y = 0;
	dev->imu_zero->m = 0;
	dev->position = malloc(sizeof(nmea));
	dev->position->lat = 0;
	dev->position->lon = 0;
	dev->position->speed = 0;
	dev->position->course = 0;
	dev->position->alt = 0;
	dev->snow_depth = 0.00;
	dev->temperature = 0;
}

static void
ui_signal_connect(GtkBuilder *builder){
	g_signal_connect(gtk_builder_get_object(builder, "bt_set_center"), "clicked", G_CALLBACK(ui_map_set_center), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_show_snow_depth"), "clicked", G_CALLBACK(ui_on_view_snow_depth), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_show_coordinates"), "clicked", G_CALLBACK(ui_on_view_coordinates), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_show_gps_more_information"), "clicked", G_CALLBACK(ui_on_view_gps_more_information), builder);
	g_signal_connect(gtk_builder_get_object(builder, "bt_view_menu"), "clicked", G_CALLBACK(ui_on_view_menu), builder);

}

static void 
on_download_clicked_event(GtkWidget *widget, gpointer user_data)
{
	int zoom, max_zoom;
	OsmGpsMap *map = OSM_GPS_MAP(user_data);
	OsmGpsMapPoint pt1, pt2;

	osm_gps_map_set_center_and_zoom (map, device->position->lat, device->position->lon, 15);
	osm_gps_map_get_bbox(map, &pt1, &pt2);
	g_object_get(map, "zoom", &zoom, "max-zoom", &max_zoom, NULL);
	osm_gps_map_download_maps(map, &pt1, &pt2, zoom, max_zoom);
}

/* ************** main *************************** */

int
main(int argc, char *argv[]){

	device = malloc(sizeof(device_data));
	GDateTime *date_time = g_date_time_new_now_local();
	GdkCursor *Cursor;
	GdkDisplay *display;
	GdkScreen *screen;
	GdkPixbuf *image_src;
	GdkWindow *main_win;
	GtkImage *image;
	GtkWidget *start_window;
	OsmGpsMapSource_t source;

	device_data_init_null(device);
	sprintf(device->position->date, "%s", g_date_time_format(date_time, "%Y_%m_%d"));

	/* ****************** GTK INTERFACE  *********************** */
	gtk_init(&argc, &argv);
	builder = gtk_builder_new();
	create_builder(builder);
	main_window = create_window(builder, UI_APP_MAIN_WINDOW);
	start_window = GTK_WIDGET(gtk_builder_get_object(builder, UI_APP_START_WINDOW));
	gtk_window_fullscreen(GTK_WINDOW(start_window));
	screen = gdk_screen_get_default();
	display = gdk_screen_get_display(screen);
	main_win = gtk_widget_get_window(main_window);
	image = (GtkImage*)gtk_builder_get_object(builder, "img_start_background");
	image_src = gdk_pixbuf_new_from_file(START_BACKGROUND_SRC, NULL);
	gtk_image_set_from_pixbuf(image, image_src);

	Cursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR);
	gdk_window_set_cursor(main_win,Cursor);

	/* ****************** OSM GPS MAP  *********************** */
	source = OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE;
	if (!osm_gps_map_source_is_valid(source)){
		return 1;
	}
	g_strdup(OSM_GPS_MAP_CACHE_DISABLED);
	map = g_object_new(OSM_TYPE_GPS_MAP,
			"map-source", source,
			"tile-cache", "/home/pi/app/app/tmp/",
			NULL);

	g_signal_connect(gtk_builder_get_object(builder, "bt_download"), "clicked", G_CALLBACK(on_download_clicked_event), (gpointer) map);
	gtk_box_pack_start(GTK_BOX(gtk_builder_get_object(builder, "box_map")), GTK_WIDGET(map), TRUE, TRUE, 0);

	/* ****************** GTK INTERFACE  *********************** */
	gtk_widget_show_all(main_window);
	gtk_widget_show_all(start_window);
	ui_init(builder);
	ui_signal_connect(builder);
	
	/* ****************** main and timer function  *********************** */
	g_timeout_add(1000, (GSourceFunc)timer_handler_init, builder);
	gtk_main();

	return 0;
}
