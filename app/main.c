/*
 * main.c
 * Copyright (C) Bartłomiej Sięka 2018 
 * <zajac382@gmail.com>
 */


 /* ******************* libraries *************************** */

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

int  gps_serial= -1;

int init_module= 3;
struct timeval  function_start;


/* ******************* variables **************************** */

GtkBuilder * builder;
GtkWidget * main_window;
GtkWidget *map_w; 

OsmGpsMap *map;
OsmGpsMapLayer *osd; 

int rc;
struct timeval tv;



/* ******************* Function  ******************* */
static
void save_log_to_file(device_data * device){

    FILE *fp;
    char src[200];
    sprintf(src, "%sDEVICE_DATA_%s.log", DEVICE_DATA_LOG_SRC, device->position->date);
    
    if( access( src, F_OK ) == -1 ) {
        fp = fopen(src, "a");
        fprintf(fp, "time\tlatitude\tlongitude\taltitude\tspeed\tsnow_depth\ttemperature\thumidity\tpressure\n");
        fclose(fp);
    } 
    
    fp = fopen(src, "a");
    fprintf(fp, "%s\t%f\t%f\t%f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", device->position->time, device->position->lat, 
        device->position->lon, device->position->alt, device->position->speed, device->snow_depth, 
        device->temperature, device->humidity, device->pressure );

    /*fprintf(fp, "%s\t%f\t%f\t%f\t%.2f\t%.2f\n", device->position->time, device->position->lat, 
        device->position->lon, device->position->alt, device->position->speed, device->snow_depth );*/ // bez warunków atmosferycznych

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
   
    if ((fptr = fopen(file_name,"r")) == NULL){
       return 0;
    }

    //date       time             T_IN H_IN T_OUT H_OUT P_IN
    //2018-02-19 13:02:10,000 INFO 34.7 16.1 23.3 25.8 951.96

    bool read_ok= false;
    for(int i= 0; i< 8; ++i){
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
                tem= atof(temp);
                break;
            case 6:
                hum= atof(temp);
                break;
            case 7:
                press= atof(temp);
                break;  
        }
        read_ok= true;
    }

    if(read_ok){
        device->temperature = tem;
        device->humidity = hum;
        device->pressure = press; 
        return 1;
    } 
    return 0;
}



/* ************  UI button function  ******************* */


static void 
ui_map_set_center(void){

    osm_gps_map_set_center_and_zoom ( map, device->position->lat, device->position->lon, 20);  
} 

static void
print_error(char *message){
    printf("\nERROR: %s\n\n", message);
    gtk_main_quit();
}




int function_init_counter= 0;
struct functionProgress *f_progress;

static bool 
timer_handler(void){

    switch(function_init_counter){
        case 0:
            ++function_init_counter;
        break;
        case 1:
            if(f_progress->progress< 1.0){
                if(gps_init( GPS_SERIAL_SOURCE, f_progress)== 1)
                    set_loading_module_progress(builder, (f_progress->progress+(float)function_init_counter-1)/(float)init_module, f_progress->message );
                else{
                    print_error(f_progress->message);
                    return false;
                }
            } 
            else
                ++function_init_counter;
        break;
        case 2:
            f_progress->message= "Oczekiwanie na ustalenie pozycji geograficznej";
            f_progress->progress= 0.5;
            set_loading_module_progress(builder, (f_progress->progress+(float)function_init_counter-1)/(float)init_module, f_progress->message );

            if(gps_read_all(GPS_SERIAL_SOURCE, device)== 1){
                if(device->position->fix_quality>= 1){
                    //set_loading_module_progress(builder, (f_progress->progress+(float)function_init_counter-1)/(float)init_module, f_progress->message );
                    if(device->position->lat != 0 && device->position->lon != 0){
                        osm_gps_map_set_center_and_zoom ( map, device->position->lat, device->position->lon, 20);                    
                    }
                    f_progress->progress= 0.0;
                    ++function_init_counter;
                }    
            }
            else{
                f_progress->progress= 0;
                --function_init_counter;
            }       
         
        break;
        case 3:
        {
            f_progress->message= "Odczyt danych z czujników";
            set_loading_module_progress(builder, (f_progress->progress+(float)function_init_counter-1)/(float)init_module, f_progress->message );
            if(f_progress->progress< 1.0){
                GDateTime *date_time;    
                date_time = g_date_time_new_now_local();                        // get local time
                device->sys_time = g_date_time_format(date_time, "%H:%M:%S");   

                int sec = atof(g_date_time_format(date_time, "%S"));

                if( sec % 5 == 2){
                    ui_read_temp_and_hum();
                    f_progress->progress= 1.0;
                    set_loading_module_progress(builder, (f_progress->progress+(float)function_init_counter-1)/(float)init_module, f_progress->message );
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
            f_progress->message= "Konfiguracja interfejsu";
            set_loading_module_progress(builder, (f_progress->progress+(float)function_init_counter-1)/(float)init_module, f_progress->message );
            GtkWidget * start_window;    
            start_window = GTK_WIDGET( gtk_builder_get_object( builder, "start_window" ) ); 
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

            if( sec % 5 == 2)
                ui_read_temp_and_hum();

            /* ************** GPS *************************** */
            int gps_read_status;
            gps_read_status= gps_read_all(GPS_SERIAL_SOURCE, device);

            if(device->position->fix_quality>= 1){
                if(device->position->lat != 0 && device->position->lon != 0){
                    osm_gps_map_image_remove_all (map);
                    GdkPixbuf * image;
                    image = gdk_pixbuf_new_from_file (DEVICE_MAP_MARKER_SRC, NULL);
                    if(image!= NULL){
                        //osm_gps_map_image_add(map, device->position->lat, device->position->lon, image);
                        osm_gps_map_gps_clear (map);
                        osm_gps_map_gps_add (map, device->position->lat, device->position->lon, device->position->course);
                    }

                    osm_gps_map_set_center_and_zoom ( map, device->position->lat, device->position->lon, 18);
                    ui_set_flag(GPS_ICON_FLAG);
                    ui_gps_icon_change( builder );

                    //save_log_to_file(device);

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

/* ************** main *************************** */


int 
main( int argc, char * argv[] ){ 
    
    f_progress = malloc(sizeof(struct functionProgress));
    f_progress->progress= 0.0;
    f_progress->message= "none";

    device = malloc(sizeof(device_data));       
    device->position = malloc(sizeof(nmea)); 
    device->position->lat = 0;
    device->position->lon = 0;
    device->position->speed= 0;
    device->position->course= 0;
    device->position->alt= 0;
    device->temperature = 0;  
    device->snow_depth= 0.00;  

    GDateTime *date_time;    
    date_time = g_date_time_new_now_local();  
    sprintf(device->position->date, "%s", g_date_time_format(date_time, "%Y_%m_%d"));    

    gtk_init( & argc, & argv );

    builder = gtk_builder_new();
    create_builder(builder);

    main_window = create_window(builder, "main_window");

    GtkWidget * start_window;    
    start_window = GTK_WIDGET( gtk_builder_get_object( builder, "start_window" ) );  

    gtk_window_fullscreen(GTK_WINDOW(start_window)); 

    GdkScreen *screen;
    screen= gdk_screen_get_default();

    GdkDisplay *display;
    display= gdk_screen_get_display(screen);
    
    GdkWindow* main_win = gtk_widget_get_window((main_window));
    GdkCursor *Cursor;

    Cursor=gdk_cursor_new_for_display (display, GDK_BLANK_CURSOR);
    gdk_window_set_cursor((main_win),Cursor);
    

    //GdkCursor* Cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
   //gdk_cursor_new_for_display (display, GDK_BLANK_CURSOR);
    /*
    
    gdk_window_set_cursor((win),Cursor);
    GdkWindow *win2 = gtk_widget_get_window((start_window));
   */

    GtkImage *image= (GtkImage*)gtk_builder_get_object(builder, "img_start_background");
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
    /*osd = g_object_new (OSM_TYPE_GPS_MAP_OSD,
                        "show-scale",FALSE,
                        "show-coordinates",FALSE,
                        "show-crosshair",FALSE,
                        "show-dpad",FALSE,
                        "show-zoom",FALSE,
                        "show-gps-in-dpad",TRUE,
                        "show-gps-in-zoom",FALSE,
                        "dpad-radius", 30,
                        NULL);

    osm_gps_map_layer_add(OSM_GPS_MAP(map), osd);*/

    gtk_box_pack_start (
                GTK_BOX(gtk_builder_get_object(builder, "box_map")),
                GTK_WIDGET(map), TRUE, TRUE, 0);   


    
    //map = OSM_GPS_MAP(map);

    //osm_gps_map_set_center_and_zoom ( map, device->position->lat, device->position->lon, 20);
    //osm_gps_map_layer_add(OSM_GPS_MAP(map), osd);
    
    gtk_widget_show_all (main_window);
    gtk_widget_show_all (start_window);
    ui_init(builder);

    /* ****************** signal connect *********************** */

    g_signal_connect(gtk_builder_get_object(builder, "bt_set_center"), "clicked", G_CALLBACK(ui_map_set_center), builder);
    g_signal_connect(gtk_builder_get_object(builder, "bt_show_snow_depth"), "clicked", G_CALLBACK(ui_on_view_snow_depth), builder);
    g_signal_connect(gtk_builder_get_object(builder, "bt_show_coordinates"), "clicked", G_CALLBACK(ui_on_view_coordinates), builder);
    g_signal_connect(gtk_builder_get_object(builder, "bt_show_gps_more_information"), "clicked", G_CALLBACK(ui_on_view_gps_more_information), builder);
    g_signal_connect(gtk_builder_get_object(builder, "bt_view_menu"), "clicked", G_CALLBACK(ui_on_view_menu), builder);
    // temporary:
       // g_signal_connect(gtk_builder_get_object(builder, "button3"), "clicked", G_CALLBACK(), (gpointer) " test button status gps and sensor");



    
    //g_log_set_handler ("OsmGpsMap", G_LOG_LEVEL_MASK, g_log_default_handler, NULL);

    g_timeout_add(200, (GSourceFunc)timer_handler, builder);
    
    gtk_main();

    return 0;
}