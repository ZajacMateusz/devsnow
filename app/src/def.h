#ifndef DEF_H
#define DEF_H

/**
* Struct and variables
*/

typedef struct _nmea{
    char time[50];      /**< Time - hh:mm:ss:ss */
    float  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
    float  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    float speed;
    float course;
    float alt;
    char date[100];
    int fix_quality;
}nmea;

typedef struct _imu
{
    float r;
    float p;
    float y;
    float m;
}imu;

typedef struct device_data{
     nmea *position;
     imu *imu_data;
     imu *imu_zero;
     float snow_depth;
     char *sys_time;
     float temperature;
     float humidity;
     float pressure;
}device_data;

/* ******************* GUI interface ******************* */

#define UI_FILE                         "/home/pi/app/app/src/view.ui"
#define VIEW_MENU_SRC                   "box_view_menu"
#define COORDINATES_BOX_SRC             "bt_coordinates"
#define WEATHER_CONDITIONS_BOX_SRC      "bt_weater_conditions"
#define GPS_MORE_INFORMATION_BOX_SRC    "bt_gps_more_information"

#define VIEW_MENU_FLAG                  0x01
#define COORDINATES_BOX_FLAG            0x02
#define WEATHER_CONDITIONS_BOX_FLAG     0x04
#define GPS_ICON_FLAG                   0x08
#define SNOW_SENSOR_ICON_FLAG           0x10
#define GPS_MORE_INFORMATION_BOX_FLAG   0x20
#define UI_APP_MAIN_WINDOW		"main_window"
#define UI_APP_START_WINDOW		"start_window"

/* ******************* icon sources ******************* */

#define GPS_ICON_ON_SRC                 "/home/pi/app/app/img/gps_icon_ok.png"
#define GPS_ICON_OFF_SRC                "/home/pi/app/app/img/gps_icon_no.png"
#define SNOW_SENSOR_ICON_ON_SRC         "/home/pi/app/app/img/snow_sensor_icon_ok.png"
#define SNOW_SENSOR_ICON_OFF_SRC        "/home/pi/app/app/img/snow_sensor_icon_no.png"
#define DEVICE_MAP_MARKER_SRC           "/home/pi/app/app/img/device_marker.png"
#define START_BACKGROUND_SRC           "/home/pi/app/app/img/start_background.png"


/* ***************** GPS ***************************** */

#define GPS_SERIAL_SOURCE      			"/dev/ttyS0" 
#define INDENT_SPACES          			"  " 

#define ATMOSPHERIC_CONDITIONS_LOG_SRC	"/media/pi/DATA/logs/thp_data/THP_DATA_CURRENT.log"
#define IMU_LOG_SRC                     "/media/pi/DATA/logs/imu_data/IMU_DATA_CURRENT.log"
#define IMU_ZERO_LOG_SRC                "/media/pi/DATA/logs/imu_data/IMU_DATA_ZERO.log"
#define DEVICE_DATA_LOG_SRC				"/media/pi/DATA/logs/device_data/"

#endif
