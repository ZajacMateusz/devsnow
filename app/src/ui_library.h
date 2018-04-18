#ifndef UI_LIBRARY_H

#define UI_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

/**
* Set the text for the label.
*/

void ui_set_label_text(GtkBuilder * builder, char *id, char *text);

/**
* Set the value of progress for the progressbar. (0 >= progress <= 1)
*/

void set_loading_progressbar( GtkBuilder * builder, char *id, float progress );

/**
* Set the value of progress and the text for the start window. 
*/

void set_loading_module_progress(GtkBuilder * builder, float progress, char *progress_message );

/**
* Checks whether the box is visible.
*/

bool ui_box_is_visible( int flag);
//Return true if the box is visible or false if not.

/**
* Set the flag for ui item.
*/

void ui_set_flag(int flag);

/**
* Reset the flag for ui item.
*/

void ui_reset_flag(int flag);

/**
* Sets the visibility of the item depending on its flag.
*/

void ui_set_visible_box(GtkBuilder * builder, char *src, int flag);

/**
* ui_set_visible_box for menu box.
*/

void ui_on_view_menu( GtkWidget *widget, GtkBuilder * builder);

/**
* ui_set_visible_box for coordinates box.
*/

void ui_on_view_coordinates( GtkWidget *widget, GtkBuilder * builder);

/**
* ui_set_visible_box for snow depth box.
*/

void ui_on_view_snow_depth(GtkWidget *widget, GtkBuilder * builder );

/**
* ui_set_visible_box for gps more information box.
*/

void ui_on_view_gps_more_information( GtkWidget *widget, GtkBuilder * builder);

/**
* Create main builder for gtk item.
*/

void create_builder(GtkBuilder * builder);

/**
* Create window.
*/

GtkWidget *create_window(GtkBuilder * builder, char *name);
//Return new window GtkWidget.

/**
* Read device_data structure and set the text for the label.
*/

void on_set_information(GtkBuilder * builder,  device_data *device);

/**
* Change gps icon.
*/

void ui_gps_icon_change(GtkBuilder * builder);

/**
* Change snow sensor icon.
*/

void ui_snow_sensor_icon_change(GtkBuilder * builder);

/**
* Init all interface item.
*/

void ui_show_interface_item(GtkBuilder * builder);

/**
* Init function, gtk item.
*/

void ui_init(GtkBuilder * builder);


#endif
