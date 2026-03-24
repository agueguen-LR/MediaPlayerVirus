#include "player.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

void show_file_dialog(GtkWidget *widget, gpointer data) {
  (void)widget;

  AppData *app_data = (AppData *)data;
  GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Open Image", app_data->window, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
      GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  gint res = gtk_dialog_run(GTK_DIALOG(dialog));

  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    char *filename = gtk_file_chooser_get_filename(chooser);

    GError *error = NULL;
    GdkPixbuf *image_pixbuf = gdk_pixbuf_new_from_file(filename, &error);

    if (image_pixbuf == NULL) {
      if (error != NULL) {
        g_printerr("Erreur lors du chargement de l'image: %s\n",
                   error->message);
        g_error_free(error);
      }
      g_free(filename);
      gtk_widget_destroy(dialog);
      return;
    }

    while (gdk_pixbuf_get_width(image_pixbuf) > 1600 ||
           gdk_pixbuf_get_height(image_pixbuf) > 1000) {
      int new_width = gdk_pixbuf_get_width(image_pixbuf) / 2;
      int new_height = gdk_pixbuf_get_height(image_pixbuf) / 2;

      GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(
          image_pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);

      g_object_unref(image_pixbuf);
      image_pixbuf = scaled_pixbuf;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(app_data->image), image_pixbuf);

    g_object_unref(image_pixbuf);
    g_free(filename);
  }

  gtk_widget_destroy(dialog);
}

void on_activate(GtkApplication *app, gpointer user_data) {
  AppData *app_data = (AppData *)user_data;

  app_data->window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(app_data->window, "Image Viewer");
  gtk_window_set_default_size(app_data->window, 900, 700);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *button = gtk_button_new_with_label("Open Image");

  app_data->image = gtk_image_new();

  g_signal_connect(button, "clicked", G_CALLBACK(show_file_dialog), app_data);

  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), app_data->image, TRUE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(app_data->window), box);
  gtk_widget_show_all(GTK_WIDGET(app_data->window));
}
