#include <errno.h>
#include <string.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "sftp.h"

GtkBuilder *builder;
GtkTreeView *htree;
GtkTreeStore *treestore;
GtkTreeIter toplevel, child;


void on_button_clicked (GtkButton *button, gpointer data){
    GtkEntry *hport, *huser, *hip, *hpw;
    GtkToggleButton *hmode;

    hmode = GTK_TOGGLE_BUTTON(gtk_builder_get_object (builder, "checkbutton1"));

    hport = GTK_ENTRY(gtk_builder_get_object (builder, "entry2"));
    hip = GTK_ENTRY(gtk_builder_get_object (builder, "entry1"));
    huser = GTK_ENTRY(gtk_builder_get_object (builder, "entry3"));
    hpw = GTK_ENTRY(gtk_builder_get_object (builder, "entry4"));

    int rc = ssh_start(g_strdup(gtk_entry_get_text(hpw)), g_strdup(gtk_entry_get_text(huser)), g_strdup(gtk_entry_get_text(hip)), g_strdup(gtk_entry_get_text(hport)), (int) gtk_toggle_button_get_active  (hmode));
    if (rc == SSH_OK){
        gtk_tree_store_append(treestore, &toplevel, NULL);
        gtk_tree_store_set(treestore, &toplevel, 0, "/",-1);
    }

}

void view_onRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn  *col, gpointer userdata)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    char *str[100];

    model = gtk_tree_view_get_model(treeview);
    gtk_tree_model_get_iter(model, &iter, path);

    if (!(gtk_tree_model_iter_has_child(model, &iter))){
        gchar *name, dir;
        gtk_tree_model_get(model, &iter, 0, &name, -1);

        for(int x =  0; x < )



        sftp_list_dir(name, str);

        int i = 0;
        while(str[i] != NULL){
            gtk_tree_store_append(treestore, &child, &toplevel);
            gtk_tree_store_set(treestore, &child,0,  str[i],-1);
            i++;
        }

        g_free(name);
    }

}


int main (int argc, char *argv[]){

    GError *errors = NULL;
    GtkWidget *window, *button;
    gtk_init (&argc, &argv);

    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "gui.glade", &errors);
    gtk_builder_connect_signals (builder, builder);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));

    htree = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview1"));

    GtkTreeViewColumn *col = gtk_tree_view_column_new();

    gtk_tree_view_column_set_title(col, "Files on the Server");
    gtk_tree_view_append_column(htree, col);

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", 0);

    treestore = gtk_tree_store_new(1, G_TYPE_STRING);

    gtk_tree_view_set_model(htree, GTK_TREE_MODEL(treestore));
    g_object_unref(GTK_TREE_MODEL(treestore));

    g_signal_connect(htree, "row-activated", (GCallback) view_onRowActivated, NULL);

    button = GTK_WIDGET(gtk_builder_get_object (builder, "button1"));
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);

    gtk_widget_show_all (window);

    gtk_main();

    ssh_close();
    return 0;

}

/*
GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Programming languages");
    gtk_tree_view_append_column(htree, col);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", 0);

    GtkTreeStore *treestore;
    GtkTreeIter toplevel, child;

    treestore = gtk_tree_store_new(1, G_TYPE_STRING);

    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, 0, "Scripting languages",-1);

    gtk_tree_store_append(treestore, &child, &toplevel);
    gtk_tree_store_set(treestore, &child,0, "Python",-1);


    gtk_tree_view_set_model(htree, GTK_TREE_MODEL(treestore));
    g_object_unref(GTK_TREE_MODEL(treestore));

*/

