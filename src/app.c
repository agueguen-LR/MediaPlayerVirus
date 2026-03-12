#include <gtk/gtk.h>

GtkWidget *image;
static GtkWindow *window = NULL;

static void file_selected(GObject *source, GAsyncResult *result,
                          gpointer data) {
  GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
  GError *error = NULL;
  GFile *file = gtk_file_dialog_open_finish(dialog, result, &error);

  if (!file)
    return;

  char *path = g_file_get_path(file);
  gtk_image_set_from_file(GTK_IMAGE(image), path);

  g_free(path);
  g_object_unref(file);
}

static void show_file_dialog(GtkWidget *widget, gpointer data) {
  GtkFileDialog *dialog = gtk_file_dialog_new();
  gtk_file_dialog_open(dialog, window, NULL, file_selected, NULL);
  g_object_unref(dialog);
}

static void on_activate(GtkApplication *app) {
  window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(window, "Image Viewer");
  gtk_window_set_default_size(window, 900, 700);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *button = gtk_button_new_with_label("Open Image");

  image = gtk_image_new();
  gtk_image_set_pixel_size(GTK_IMAGE(image), 800);

  g_signal_connect(button, "clicked", G_CALLBACK(show_file_dialog), NULL);

  gtk_box_append(GTK_BOX(box), button);
  gtk_box_append(GTK_BOX(box), image);

  gtk_window_set_child(window, box);

  gtk_window_present(window);
}

int main(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("com.example.GtkApplication",
                                            G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  return g_application_run(G_APPLICATION(app), argc, argv);
}
