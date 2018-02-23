CC = gcc

GTK_CFLAGS = -pthread -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include
#GTK_CFLAGS= -pthread -I/usr/include/gtk-3.0 -I/usr/lib64/gtk-3.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 
GLIB_CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include 
CAIRO_CFLAGS = -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16
SOUP24_CFLAGS = -pthread -I/usr/include/libsoup-2.4 -I/usr/include/libxml2 -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include
GTK_LIBS = -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0
WARN_CFLAGS = -I. -Wall -Wstrict-prototypes -Wnested-externs -Werror=missing-prototypes -Werror=implicit-function-declaration -Werror=pointer-arith -Werror=init-self -Werror=format=2 -Werror=missing-include-dirs -Werror=return-type 
GTHREAD_CFLAGS = -pthread -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include

OSMGPSMAP_CFLAGS = \
    $(GLIB_CFLAGS)          \
    $(GTK_CFLAGS)           \
    $(CAIRO_CFLAGS)         \
    $(SOUP24_CFLAGS)

mapviewer_CFLAGS = \
	-I/usr/include/osmgpsmap-1.0   \
	-I/usr/lib/girepository-1.0 \
	-lm /usr/lib/arm-linux-gnueabihf/libosmgpsmap-1.0.a\
	-lm /usr/lib/arm-linux-gnueabihf/libosmgpsmap-1.0.so\
    $(WARN_CFLAGS)          \
    $(OSMGPSMAP_CFLAGS)     \
    $(GTHREAD_CFLAGS)

FILE_FLAG = src/def.h lib/gps_module.c src/gps_module.h lib/ui_library.c src/ui_library.h gps_module.o ui_library.o minmea.o
FILE_LINKER = gps_module.o ui_library.o minmea.o


start: app	
	./app

app: main.c $(FILE_FLAG)
	$(CC) $(FILE_LINKER) $(mapviewer_CFLAGS) $(GTK_LIBS) src/def.h main.c  -o app  -lm -lgps -pedantic -Wall

gps_module.o: lib/gps_module.c src/gps_module.h src/minmea.h 
	$(CC) -c  lib/gps_module.c src/gps_module.h $(GLIB_CFLAGS)

ui_library.o: lib/ui_library.c src/ui_library.h src/gps_module.h 
	$(CC) -c $(GTK_CFLAGS) $(GTK_LIBS) lib/ui_library.c src/ui_library.h 


#gps.o: src/gps_library.c src/gps_library.h src/minmea.c src/minmea.h
#	$(CC) -c -Wall -pedantic src/gps_library.c src/gps_library.h src/minmea.c src/minmea.h