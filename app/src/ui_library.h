#ifndef UI_LIBRARY_H

#define UI_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>

#include <stdbool.h>

/**
* Set the text for the label.
*/

void ui_set_label_text(GtkBuilder * builder, char *id, char *text);

/**
* Set the value of progress for the progressbar.
*/

void set_loading_progressbar( GtkBuilder * builder, char *id, float progress );

void set_loading_module_progress(GtkBuilder * builder, float progress, char *progress_message );

bool ui_box_is_visible( int flag);

void ui_set_flag(int flag);

void ui_reset_flag(int flag);

void ui_set_visible_box(GtkBuilder * builder, char *src, int flag);

void ui_on_view_menu( GtkWidget *widget, GtkBuilder * builder);

void ui_on_view_coordinates( GtkWidget *widget, GtkBuilder * builder);

void ui_on_view_snow_depth(GtkWidget *widget, GtkBuilder * builder );

void ui_on_view_gps_more_information( GtkWidget *widget, GtkBuilder * builder);

void create_builder(GtkBuilder * builder);

GtkWidget *create_window(GtkBuilder * builder, char *name);

void on_set_information(GtkBuilder * builder,  device_data *device);

void ui_gps_icon_change(GtkBuilder * builder);

void ui_snow_sensor_icon_change(GtkBuilder * builder);

void ui_show_interface_item(GtkBuilder * builder);

void ui_init(GtkBuilder * builder);


#endif