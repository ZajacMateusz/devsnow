
#include "ui_library.h"
#include "minmea.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
 
short ui_visible_item= 0b00100001;


void ui_set_label_text(GtkBuilder * builder, char *id, char *text){
    GtkLabel *label = (GtkLabel *)gtk_builder_get_object(builder, id);
    gtk_label_set_text (label, text );
}

void set_loading_progressbar( GtkBuilder * builder, char *id, float progress ){ // progres ( 0 to 1 )
    GtkProgressBar *pbar = (GtkProgressBar *)gtk_builder_get_object(builder, id);
    gtk_progress_bar_set_fraction (pbar, progress); 
}

void set_loading_module_progress(GtkBuilder * builder, float progress, char *progress_message ){
    ui_set_label_text(builder, "lb_progress_info_start_window", progress_message);
    set_loading_progressbar(builder, "progressbar_start_window", progress);
}

bool ui_box_is_visible( int flag){
    return (ui_visible_item & flag);
}

void ui_set_flag(int flag){
    ui_visible_item|= flag;
}

void ui_reset_flag(int flag){
    ui_visible_item&= ~flag;
}

void ui_set_visible_box(GtkBuilder * builder, char *src, int flag){
    if(!ui_box_is_visible(flag)){
        gtk_widget_show (GTK_WIDGET(gtk_builder_get_object(builder, src)));  
        ui_set_flag(flag);
    }
    else{
        gtk_widget_hide (GTK_WIDGET(gtk_builder_get_object(builder, src)));  
        ui_reset_flag(flag);
    } 
}   



void ui_on_view_menu( GtkWidget *widget, GtkBuilder * builder){
    ui_set_visible_box(builder, VIEW_MENU_SRC, VIEW_MENU_FLAG);
}

void ui_on_view_coordinates( GtkWidget *widget, GtkBuilder * builder){    
    ui_set_visible_box(builder, COORDINATES_BOX_SRC, COORDINATES_BOX_FLAG);
}

void ui_on_view_snow_depth(GtkWidget *widget, GtkBuilder * builder ){    
    ui_set_visible_box(builder, WEATHER_CONDITIONS_BOX_SRC, WEATHER_CONDITIONS_BOX_FLAG);
}

void ui_on_view_gps_more_information( GtkWidget *widget, GtkBuilder * builder){  
  ui_set_visible_box(builder, GPS_MORE_INFORMATION_BOX_SRC, GPS_MORE_INFORMATION_BOX_FLAG); 
}

void create_builder(GtkBuilder * builder){
    
    GError * error = NULL;  
    /* Tworzy obiekt buildera */
    /* Wczytuje zawartość interfejsu i sprawdza ewentualne błędy */
    if( !gtk_builder_add_from_file( builder, UI_FILE, & error ) )
    {
        g_warning( "Nie można wczytać plilu buildera: %s", error->message );
       g_error_free( error );
    }
        
    /* Łączy sygnały zawarte w pliku interfejsu z odpowiednimi funkcjami */
    gtk_builder_connect_signals( builder, NULL );
    
}

GtkWidget *create_window(GtkBuilder * builder, char *name){
    
    GtkWidget * window; 
    
    /* Pobiera obiekt z nazwą "window1" */
    window = GTK_WIDGET( gtk_builder_get_object( builder, name ) );  
    gtk_window_fullscreen(GTK_WINDOW(window));
    
    return window;
}

void on_set_information(GtkBuilder * builder,  device_data *device){

    char text[100];
    GtkLabel *label;

    sprintf(text, "%f", device->position->lat);
    ui_set_label_text(builder, "lb_lat_coordinates", text);
    ui_set_label_text(builder, "lb_gps_more_lat", text);


    //sprintf(text, "%c", device->position->ns);
    //ui_set_label_text(builder, "lb_lat_coordinates_ns", &text);

    sprintf(text, "%f", device->position->lon);
    ui_set_label_text(builder, "lb_lon_coordinates", text);
    ui_set_label_text(builder, "lb_gps_more_lon", text);

    //sprintf(text, "%c", device->position->ew);
    //ui_set_label_text(builder, "lb_lon_coordinates_ew", &text);

    sprintf(text, "%0.2fm", device->position->alt);
    ui_set_label_text(builder, "lb_gps_more_alt", text);

    sprintf(text, "%0.2fkm/h", device->position->speed);
    ui_set_label_text(builder, "lb_gps_more_speed", text);


    ui_set_label_text(builder, "lb_gps_more_speed1", device->position->time);

    sprintf(text, "%0.1f°C", device->temperature);
    ui_set_label_text(builder, "lb_tem", text);

    char znak= '%';
    sprintf(text, "%0.1f%c", device->humidity, znak);
    ui_set_label_text(builder, "lb_hum", text);

    sprintf(text, "%0.0f°", device->imu_data->r);
    ui_set_label_text(builder, "lb_rotate_1", text);

    sprintf(text, "%0.0f°", device->imu_data->p);
    ui_set_label_text(builder, "lb_rotate_2", text);

    sprintf(text, "%0.1fkPa", device->pressure);
    ui_set_label_text(builder, "lb_press", text);

    sprintf(text, "%f", device->snow_depth);
    char tab[5]= "";
    for(int i= 0; i< 4; ++i)
        tab[i]= text[i];

    ui_set_label_text(builder, "lb_snow_depth", tab);

    ui_set_label_text(builder, "lb_time", device->sys_time);
}

void ui_gps_icon_change(GtkBuilder * builder){    
    GtkImage *image= (GtkImage*)gtk_builder_get_object(builder, "img_gps_status");
    GdkPixbuf * image_src;    
    if(ui_box_is_visible(GPS_ICON_FLAG)){        
        image_src = gdk_pixbuf_new_from_file (GPS_ICON_ON_SRC, NULL);
        gtk_image_set_from_pixbuf(image, image_src);
    }
    else {
        image_src = gdk_pixbuf_new_from_file (GPS_ICON_OFF_SRC, NULL);
        gtk_image_set_from_pixbuf(image, image_src);
    }
}

void ui_snow_sensor_icon_change(GtkBuilder * builder){
    GtkImage *image= (GtkImage*)gtk_builder_get_object(builder, "img_snow_sensor_status");   
    GdkPixbuf * image_src; 
    if(ui_box_is_visible(SNOW_SENSOR_ICON_FLAG)){        
        image_src = gdk_pixbuf_new_from_file (SNOW_SENSOR_ICON_ON_SRC, NULL);
        gtk_image_set_from_pixbuf(image, image_src);
    }
    else{
        image_src = gdk_pixbuf_new_from_file (SNOW_SENSOR_ICON_OFF_SRC, NULL);
        gtk_image_set_from_pixbuf(image, image_src);
    }  
}

void ui_show_interface_item(GtkBuilder * builder){

    ui_gps_icon_change( builder );
    ui_snow_sensor_icon_change(builder);
    ui_set_visible_box(builder, VIEW_MENU_SRC, VIEW_MENU_FLAG);
    ui_set_visible_box(builder, WEATHER_CONDITIONS_BOX_SRC, WEATHER_CONDITIONS_BOX_FLAG);
    ui_set_visible_box(builder, COORDINATES_BOX_SRC, COORDINATES_BOX_FLAG);
    ui_set_visible_box(builder, GPS_MORE_INFORMATION_BOX_SRC, GPS_MORE_INFORMATION_BOX_FLAG );
}

void ui_init(GtkBuilder * builder){
    ui_show_interface_item(builder);
}
