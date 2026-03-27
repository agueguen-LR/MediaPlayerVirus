/**
 * @file Liste
 * @brief A simple C program of a to-do-list
 *
 * @author Enzocte <enzo.cateau@etudiant.univ-lr.fr>
 * @date 2026
 */
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *entry;
    GtkWidget *listbox;
    GtkWidget *status_label;
    int item_count;
} AppWidgets;

static void update_status(AppWidgets *app) {
    gchar *text = g_strdup_printf("Elements: %d", app->item_count);
    gtk_label_set_text(GTK_LABEL(app->status_label), text);
    g_free(text);
}

static void remove_row_and_update(AppWidgets *app, GtkWidget *row) {
    if (!row) return;
    gtk_widget_destroy(row);
    if (app->item_count > 0) {
        app->item_count--;
    }
    update_status(app);
}

static void on_delete_row_clicked(GtkButton *button, gpointer user_data) {
    (void)user_data;
    GtkWidget *row = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "row"));
    AppWidgets *app = (AppWidgets *)g_object_get_data(G_OBJECT(button), "app");
    remove_row_and_update(app, row);
}

static GtkWidget *create_list_row(AppWidgets *app, const gchar *text) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *label = gtk_label_new(text);
    GtkWidget *btn_delete = gtk_button_new_with_label("Supprimer");

    gtk_widget_set_hexpand(label, TRUE);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    g_object_set_data(G_OBJECT(btn_delete), "row", row);
    g_object_set_data(G_OBJECT(btn_delete), "app", app);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_delete_row_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 8);
    gtk_box_pack_start(GTK_BOX(hbox), btn_delete, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(row), hbox);
    return row;
}

static void on_add_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppWidgets *app = (AppWidgets *)user_data;

    const gchar *input = gtk_entry_get_text(GTK_ENTRY(app->entry));
    if (!input || *input == '\0') {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Entre un texte avant d'ajouter.");
        return;
    }

    GtkWidget *row = create_list_row(app, input);
    gtk_container_add(GTK_CONTAINER(app->listbox), row);
    gtk_widget_show_all(row);

    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
    app->item_count++;
    update_status(app);
}

static void on_entry_activate(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    on_add_clicked(NULL, user_data);
}

static void on_remove_selected_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppWidgets *app = (AppWidgets *)user_data;
    GtkListBoxRow *selected = gtk_list_box_get_selected_row(GTK_LIST_BOX(app->listbox));

    if (!selected) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Selectionne une ligne a supprimer.");
        return;
    }

    remove_row_and_update(app, GTK_WIDGET(selected));
}

static void on_clear_all_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppWidgets *app = (AppWidgets *)user_data;

    GList *children = gtk_container_get_children(GTK_CONTAINER(app->listbox));
    for (GList *it = children; it != NULL; it = it->next) {
        gtk_widget_destroy(GTK_WIDGET(it->data));
    }
    g_list_free(children);

    app->item_count = 0;
    update_status(app);
}

static void on_quit_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_window_close(GTK_WINDOW(window));
}

static void on_activate(GtkApplication *gtk_app, gpointer user_data) {
    (void)user_data;

    AppWidgets *app = g_malloc0(sizeof(AppWidgets));

    GtkWidget *window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(window), "Mini gestionnaire GTK3");
    gtk_window_set_default_size(GTK_WINDOW(window), 520, 420);
    gtk_container_set_border_width(GTK_CONTAINER(window), 12);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), root);

    GtkWidget *title = gtk_label_new("Liste simple (GTK 3)");
    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);

    GtkWidget *input_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry), "Ex: Acheter du pain");
    GtkWidget *btn_add = gtk_button_new_with_label("Ajouter");

    gtk_box_pack_start(GTK_BOX(input_row), app->entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(input_row), btn_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), input_row, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled, TRUE);

    app->listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(app->listbox), GTK_SELECTION_SINGLE);
    gtk_container_add(GTK_CONTAINER(scrolled), app->listbox);
    gtk_box_pack_start(GTK_BOX(root), scrolled, TRUE, TRUE, 0);

    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *btn_remove_selected = gtk_button_new_with_label("Supprimer selection");
    GtkWidget *btn_clear_all = gtk_button_new_with_label("Vider tout");
    GtkWidget *btn_quit = gtk_button_new_with_label("Quitter");

    gtk_box_pack_start(GTK_BOX(actions), btn_remove_selected, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_clear_all, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(actions), btn_quit, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), actions, FALSE, FALSE, 0);

    app->status_label = gtk_label_new("Elements: 0");
    gtk_box_pack_start(GTK_BOX(root), app->status_label, FALSE, FALSE, 0);

    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_clicked), app);
    g_signal_connect(app->entry, "activate", G_CALLBACK(on_entry_activate), app);
    g_signal_connect(btn_remove_selected, "clicked", G_CALLBACK(on_remove_selected_clicked), app);
    g_signal_connect(btn_clear_all, "clicked", G_CALLBACK(on_clear_all_clicked), app);
    g_signal_connect(btn_quit, "clicked", G_CALLBACK(on_quit_clicked), window);

    g_object_set_data_full(G_OBJECT(window), "app_data", app, g_free);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.exemple.gtk3.simpleplus", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return status;
}
