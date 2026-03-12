#include <gtk/gtk.h>

GtkWidget *image;
static GtkWindow *window = NULL;

static void show_file_dialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  gint res;

  dialog = gtk_file_chooser_dialog_new(
      "Open Image", window, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
      GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  res = gtk_dialog_run(GTK_DIALOG(dialog));

  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    char *filename = gtk_file_chooser_get_filename(chooser);

    gtk_image_set_from_file(GTK_IMAGE(image), filename);

    g_free(filename);
  }

  gtk_widget_destroy(dialog);
}

static void on_activate(GtkApplication *app, gpointer user_data) {
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

int main(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("com.example.GtkApplication",
                                            G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
