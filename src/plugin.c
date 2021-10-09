/*
 * Tweaks Plugin for Geany
 * Copyright 2021 xiota
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "plugin.h"
#include "prefs.h"

/* ********************
 * Declarations
 */
static gboolean tweaks_init(GeanyPlugin *plugin, gpointer data);
static void tweaks_cleanup(GeanyPlugin *plugin, gpointer data);
static GtkWidget *tweaks_configure(GeanyPlugin *plugin, GtkDialog *dialog,
                                   gpointer pdata);

static void on_document_signal(GObject *obj, GeanyDocument *doc,
                               gpointer user_data);

static void on_pref_reload_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog);
static void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog);

static void on_menu_preferences(GtkWidget *self, GtkWidget *dialog);

void on_toggle_editor_vte();
void on_toggle_editor_sidebar();
void on_toggle_editor_msgwin();
void on_toggle_editor_sidebar_msgwin();
bool on_key_binding(int key_id);

static GtkWidget *find_focus_widget(GtkWidget *widget);

/* ********************
 * Globals
 */
GeanyPlugin *geany_plugin;
GeanyData *geany_data;

static GeanyDocument *g_current_doc = NULL;
GtkNotebook *g_notebook_sidebar = NULL;
GtkNotebook *g_notebook_msgwin = NULL;

/* ********************
 * Plugin Setup
 */
PLUGIN_VERSION_CHECK(211)

PLUGIN_SET_INFO(
    "Xi/Tweaks",
    "Tweaks for Geany.  Multiple column markers.  Extra keybindings.", "0.01.0",
    "xiota")

void plugin_init(GeanyData *data) {
  GEANY_PSC("geany-startup-complete", on_document_signal);
  GEANY_PSC("document-activate", on_document_signal);
  GEANY_PSC("document-new", on_document_signal);
  GEANY_PSC("document-open", on_document_signal);
  GEANY_PSC("document-reload", on_document_signal);

  // Set keyboard shortcuts
  GeanyKeyGroup *group = plugin_set_key_group(
      geany_plugin, _("Xi/Tweaks"), 1, (GeanyKeyGroupCallback)on_key_binding);

  keybindings_set_item(
      group, TWEAKS_KEY_TOGGLE_EDITOR_SIDEBAR_MSGWIN, NULL, 0, 0,
      "xitweaks_toggle_editor_sidebar_msgwin",
      _("Switch focus among editor, sidebar, and message window."), NULL);

  tweaks_init(geany_plugin, geany_data);
}

void plugin_cleanup(void) { tweaks_cleanup(geany_plugin, geany_data); }

GtkWidget *plugin_configure(GtkDialog *dlg) {
  return tweaks_configure(geany_plugin, dlg, geany_data);
}

// void plugin_help(void) { }

static gboolean tweaks_init(GeanyPlugin *plugin, gpointer data) {
  geany_plugin = plugin;
  geany_data = plugin->geany_data;

  open_settings();

  // save notebook and message window
  g_notebook_sidebar = GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook);
  g_notebook_msgwin =
      GTK_NOTEBOOK(geany->main_widgets->message_window_notebook);

  // set up menu
  GeanyKeyGroup *group;
  GtkWidget *item;

  GtkWidget *tweaks_menu = gtk_menu_item_new_with_label("Xi/Tweaks");
  ui_add_document_sensitive(tweaks_menu);

  GtkWidget *submenu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(tweaks_menu), submenu);

  item = gtk_menu_item_new_with_label("Edit Config File");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_edit_config), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Edit Config File");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_reload_config), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Open Config Folder");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_open_config_folder),
                   NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Preferences");
  g_signal_connect(item, "activate", G_CALLBACK(on_menu_preferences), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  gtk_widget_show_all(tweaks_menu);

  gtk_menu_shell_append(GTK_MENU_SHELL(geany_data->main_widgets->tools_menu),
                        tweaks_menu);

  return TRUE;
}

static void tweaks_cleanup(GeanyPlugin *plugin, gpointer data) { return; }

static GtkWidget *tweaks_configure(GeanyPlugin *plugin, GtkDialog *dialog,
                                   gpointer pdata) {
  GtkWidget *box, *btn;
  char *tooltip;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

  tooltip = g_strdup("Save the active settings to the config file.");
  btn = gtk_button_new_with_label("Save Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_save_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup(
      "Reload settings from the config file.  May be used "
      "to apply preferences after editing without restarting Geany.");
  btn = gtk_button_new_with_label("Reload Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_reload_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup(
      "Delete the current config file and restore the default "
      "file with explanatory comments.");
  btn = gtk_button_new_with_label("Reset Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_reset_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup("Open the config file in Geany for editing.");
  btn = gtk_button_new_with_label("Edit Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_edit_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup(
      "Open the config folder in the default file manager.  The config folder "
      "contains the stylesheets, which may be edited.");
  btn = gtk_button_new_with_label("Open Config Folder");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_open_config_folder),
                   dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  return box;
}

/* ********************
 * Preferences Callbacks
 */

static void on_pref_reload_config(GtkWidget *self, GtkWidget *dialog) {
  open_settings();
}

static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog) {
  save_settings();
}

static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog) {
  save_default_settings();
}

static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog) {
  char *conf_dn = g_build_filename(geany_data->app->configdir, "plugins", NULL);

  char *command;
  command = g_strdup_printf("xdg-open \"%s\"", conf_dn);
  if (system(command)) {
    // ignore;
  }
  GFREE(conf_dn);
  GFREE(command);
}

static void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog) {
  open_settings();
  char *conf_fn = g_build_filename(geany_data->app->configdir, "plugins",
                                   "xitweaks", "xitweaks.conf", NULL);
  GeanyDocument *doc = document_open_file(conf_fn, FALSE, NULL, NULL);
  document_reload_force(doc, NULL);
  GFREE(conf_fn);

  if (dialog != NULL) {
    gtk_widget_destroy(GTK_WIDGET(dialog));
  }
}

static void on_menu_preferences(GtkWidget *self, GtkWidget *dialog) {
  plugin_show_configure(geany_plugin);
}

/* ********************
 * Shortcut Callbacks
 */

void on_toggle_editor_sidebar_msgwin() {
  GeanyDocument *doc = document_get_current();
  if (doc != NULL) {
    GtkWidget *sci = GTK_WIDGET(doc->editor->sci);
    gint cur_page = gtk_notebook_get_current_page(g_notebook_sidebar);
    GtkWidget *page = gtk_notebook_get_nth_page(g_notebook_sidebar, cur_page);
    page = find_focus_widget(page);

    if (gtk_widget_has_focus(sci) &&
        gtk_widget_is_visible(GTK_WIDGET(g_notebook_sidebar))) {
      keybindings_send_command(GEANY_KEY_GROUP_FOCUS, GEANY_KEYS_FOCUS_SIDEBAR);
    } else if (gtk_widget_has_focus(page) &&
               gtk_widget_is_visible(GTK_WIDGET(g_notebook_msgwin))) {
      keybindings_send_command(GEANY_KEY_GROUP_FOCUS,
                               GEANY_KEYS_FOCUS_MESSAGE_WINDOW);
    } else {
      keybindings_send_command(GEANY_KEY_GROUP_FOCUS, GEANY_KEYS_FOCUS_EDITOR);
    }
  }
}

bool on_key_binding(int key_id) {
  switch (key_id) {
    case TWEAKS_KEY_TOGGLE_EDITOR_SIDEBAR_MSGWIN:
      on_toggle_editor_sidebar_msgwin();
      break;
    default:
      return FALSE;
  }
  return TRUE;
}

/* ********************
 * Geany Signal Callbacks
 */

static void on_document_signal(GObject *obj, GeanyDocument *doc,
                               gpointer user_data) {
  if (settings.column_marker_enable && DOC_VALID(doc)) {
    scintilla_send_message(doc->editor->sci, SCI_SETEDGEMODE, 3, 3);
    scintilla_send_message(doc->editor->sci, SCI_MULTIEDGECLEARALL, 0, 0);

    if (settings.column_marker_columns != NULL &&
        settings.column_marker_colors != NULL) {
      for (int i = 0; i < settings.column_marker_count; i++) {
        scintilla_send_message(doc->editor->sci, SCI_MULTIEDGEADDLINE,
                               settings.column_marker_columns[i],
                               settings.column_marker_colors[i]);
      }
    }
  }
}

/* ********************
 * Other functions
 */

static GtkWidget *find_focus_widget(GtkWidget *widget) {
  GtkWidget *focus = NULL;

  // optimized simple case
  if (GTK_IS_BIN(widget)) {
    focus = find_focus_widget(gtk_bin_get_child(GTK_BIN(widget)));
  } else if (GTK_IS_CONTAINER(widget)) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
    GList *node;

    for (node = children; node && !focus; node = node->next)
      focus = find_focus_widget(node->data);
    g_list_free(children);
  }

  /* Some containers handled above might not have children and be what we want
   * to focus (e.g. GtkTreeView), so focus that if possible and we don't have
   * anything better */
  if (!focus && gtk_widget_get_can_focus(widget)) {
    focus = widget;
  }
  return focus;
}
