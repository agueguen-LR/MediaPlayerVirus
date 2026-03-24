#ifndef VIEWER_H
#define VIEWER_H

#include <gtk/gtk.h>

typedef struct {
  GtkWidget *image;
  GtkWindow *window;
} AppData;

void show_file_dialog(GtkWidget *widget, gpointer data);
void on_activate(GtkApplication *app, gpointer user_data);

#endif
