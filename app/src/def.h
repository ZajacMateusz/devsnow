#ifndef DEF_H
#define DEF_H

/**
* IMU data struct. 
*/

typedef struct _imu
{
	float m;
	float p;
	float r;
	float y;
}imu;

/**
* GPS data struct. 
*/


typedef struct _nmea{
	char date[100];
	char ew; 
	char ns; 
	char time[50];  
	float alt;
	float course;
	float lat;   
	float lon;
	float speed;
	int fix_quality;
}nmea;

/**
* Device data struct. 
*/

typedef struct device_data{
	char *sys_time;
	float humidity;
	float pressure;
	float snow_depth;
	float temperature;
	imu *imu_data;
	imu *imu_zero;
	nmea *position;
}device_data;

/**
*  GUI interface. 
*/

#define COORDINATES_BOX_SRC             "bt_coordinates"
#define GPS_MORE_INFORMATION_BOX_SRC    "bt_gps_more_information"
#define UI_FILE                         "/home/pi/app/app/src/view.ui"
#define VIEW_MENU_SRC                   "box_view_menu"
#define WEATHER_CONDITIONS_BOX_SRC      "bt_weater_conditions"

#define VIEW_MENU_FLAG                  0x01
#define COORDINATES_BOX_FLAG            0x02
#define WEATHER_CONDITIONS_BOX_FLAG     0x04
#define GPS_ICON_FLAG                   0x08
#define SNOW_SENSOR_ICON_FLAG           0x10
#define GPS_MORE_INFORMATION_BOX_FLAG   0x20
#define UI_APP_MAIN_WINDOW		"main_window"
#define UI_APP_START_WINDOW		"start_window"

/** 
* Icon sources. 
*/

#define DEVICE_MAP_MARKER_SRC           "/home/pi/app/app/img/device_marker.png"
#define GPS_ICON_ON_SRC                 "/home/pi/app/app/img/gps_icon_ok.png"
#define GPS_ICON_OFF_SRC                "/home/pi/app/app/img/gps_icon_no.png"
#define SNOW_SENSOR_ICON_ON_SRC         "/home/pi/app/app/img/snow_sensor_icon_ok.png"
#define SNOW_SENSOR_ICON_OFF_SRC        "/home/pi/app/app/img/snow_sensor_icon_no.png"
#define START_BACKGROUND_SRC		"/home/pi/app/app/img/start_background.png"

/**
* GPS. 
*/

#define ATMOSPHERIC_CONDITIONS_LOG_SRC	"/media/pi/DATA/logs/thp_data/THP_DATA_CURRENT.log"
#define DEVICE_DATA_LOG_SRC		"/media/pi/DATA/logs/device_data/"
#define GPS_SERIAL_SOURCE      		"/dev/ttyS0" 
#define INDENT_SPACES			"  " 
#define IMU_LOG_SRC                     "/media/pi/DATA/logs/imu_data/IMU_DATA_CURRENT.log"
#define IMU_ZERO_LOG_SRC                "/media/pi/DATA/logs/imu_data/IMU_DATA_ZERO.log"

#endif
