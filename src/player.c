/**
 * @file player.c
 * @brief A simple image viewer using GTK.
 *
 * @author agueguen-LR <agueguen@proton.me>
 * @date 2026
 */

#include <gtk/gtk.h>

#include "player.h"

static GtkWindow* window;
static GtkWidget* image;

/**
 * @brief Show a file dialog to select an image, load it, and display it in the GTK application.
 *
 * @param widget: The widget that triggered the file dialog (e.g., a button)
 * @param data: Additional data passed to the callback (not used in this function)
 */
void show_file_dialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Open Image", window, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
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

    gtk_image_set_from_pixbuf(GTK_IMAGE(image), image_pixbuf);

    g_object_unref(image_pixbuf);
    g_free(filename);
  }

  gtk_widget_destroy(dialog);
}

/**
 * @brief Activate the GTK application, create the main window, and set up the UI components (button and image display).
 *
 * @param app: The GTK application instance
 * @param user_data: Additional data passed to the callback (not used in this function)
 */
void on_activate(GtkApplication *app, gpointer user_data) {
  window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(window, "Image Viewer");
  gtk_window_set_default_size(window, 900, 700);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *button = gtk_button_new_with_label("Open Image");

  image = gtk_image_new();

  g_signal_connect(button, "clicked", G_CALLBACK(show_file_dialog), NULL);

  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(window), box);
  gtk_widget_show_all(GTK_WIDGET(window));
}

/**
 * @brief The main function to run the media player application, which initializes the GTK application and starts the event loop.
 *
 * @param argc: The argument count passed to the media player function
 * @param argv: The argument vector passed to the media player function
 * @return The exit status of the media player application
 */
int mediaPlayer(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("com.example.GtkApplication",
                                            G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);

  g_object_unref(app);
  return status;
}
