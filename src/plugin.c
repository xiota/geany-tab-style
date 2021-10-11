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
 * Function Declarations
 */
// Plugin Setup
static gboolean tweaks_init(GeanyPlugin *plugin, gpointer data);
static void tweaks_cleanup(GeanyPlugin *plugin, gpointer data);
static GtkWidget *tweaks_configure(GeanyPlugin *plugin, GtkDialog *dialog,
                                   gpointer pdata);

// Pane Position Callbacks
static void pane_position_update(gboolean enable);
static gboolean on_draw_pane(GtkWidget *self, cairo_t *cr, gpointer user_data);

// Sidebar Tab Focus Callbacks
static void sidebar_focus_update(gboolean enable);
static void on_grab_notify(GtkWidget *self, gboolean was_grabbed,
                           gpointer user_data);
static void on_switch_page(GtkNotebook *self, GtkWidget *page, guint page_num,
                           gpointer user_data);
static void on_grab_focus_sidebar(GtkWidget *self, gpointer user_data);
static void on_grab_focus_msgwin(GtkWidget *self, gpointer user_data);
static void on_grab_focus_editor(GtkWidget *self, gpointer user_data);
static void on_set_focus_child_sidebar(GtkContainer *self, GtkWidget *object,
                                       gpointer user_data);
static void on_set_focus_child_msgwin(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data);
static void on_set_focus_child_editor(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data);
static void sidebar_focus_highlight();

// Preferences Callbacks
static void on_pref_reload_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog);
static void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog);
static void on_menu_preferences(GtkWidget *self, GtkWidget *dialog);

// Keybinding Functions and Callbacks
void on_toggle_editor_vte();
void on_toggle_editor_sidebar();
void on_toggle_editor_msgwin();
void on_toggle_editor_sidebar_msgwin();
bool on_key_binding(int key_id);
static GtkWidget *find_focus_widget(GtkWidget *widget);

// Geany Signal Callbacks
static void on_document_signal(GObject *obj, GeanyDocument *doc,
                               gpointer user_data);

/* ********************
 * Globals
 */
GeanyPlugin *geany_plugin;
GeanyData *geany_data;

static GtkWindow *geany_window = NULL;
static GtkNotebook *geany_sidebar = NULL;
static GtkNotebook *geany_msgwin = NULL;
static GtkNotebook *geany_editor = NULL;
static GtkWidget *geany_hpane = NULL;

GtkWidget *g_tweaks_menu = NULL;
static GeanyDocument *g_current_doc = NULL;

static gulong g_handle_set_focus_child_sidebar = 0;
static gulong g_handle_set_focus_child_msgwin = 0;
static gulong g_handle_set_focus_child_editor = 0;
static gulong g_handle_grab_focus_sidebar = 0;
static gulong g_handle_grab_focus_msgwin = 0;
static gulong g_handle_grab_focus_editor = 0;
static gulong g_handle_grab_notify = 0;
static gulong g_handle_switch_page = 0;
static gulong g_handle_pane_position = 0;

static gulong g_handle_tab_object = 0;
static GtkWidget *g_tab_object = NULL;

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

  // geany widgets for later use
  geany_window = GTK_WINDOW(geany->main_widgets->window);
  geany_sidebar = GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook);
  geany_msgwin = GTK_NOTEBOOK(geany->main_widgets->message_window_notebook);
  geany_editor = GTK_NOTEBOOK(geany->main_widgets->notebook);
  geany_hpane = ui_lookup_widget(GTK_WIDGET(geany_window), "hpaned1");

  // set up menu
  GtkWidget *item;

  g_tweaks_menu = gtk_menu_item_new_with_label("Xi/Tweaks");
  ui_add_document_sensitive(g_tweaks_menu);

  GtkWidget *submenu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(g_tweaks_menu), submenu);

  item = gtk_menu_item_new_with_label("Edit Config File");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_edit_config), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Reload Config File");
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

  gtk_widget_show_all(g_tweaks_menu);

  gtk_menu_shell_append(GTK_MENU_SHELL(geany_data->main_widgets->tools_menu),
                        g_tweaks_menu);

  pane_position_update(settings.hpaned_position_enabled ||
                       settings.hpaned_position_auto);
  sidebar_focus_update(settings.sidebar_focus_enabled);
  return TRUE;
}

static void tweaks_cleanup(GeanyPlugin *plugin, gpointer data) {
  gtk_widget_destroy(g_tweaks_menu);

  sidebar_focus_update(FALSE);
  pane_position_update(FALSE);

  save_settings();
}

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

  pane_position_update(settings.hpaned_position_enabled ||
                       settings.hpaned_position_auto);
  sidebar_focus_update(settings.sidebar_focus_enabled);
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
 * Pane Position Callbacks
 */

static void pane_position_update(gboolean enable) {
  if (enable && !g_handle_pane_position) {
    g_handle_pane_position = g_signal_connect(GTK_WIDGET(geany_hpane), "draw",
                                              G_CALLBACK(on_draw_pane), NULL);
  }

  if (!enable && g_handle_pane_position) {
    g_clear_signal_handler(&g_handle_pane_position, GTK_WIDGET(geany_hpane));
  }
}

static gboolean on_draw_pane(GtkWidget *self, cairo_t *cr, gpointer user_data) {
  if (!settings.hpaned_position_enabled && !settings.hpaned_position_auto) {
    pane_position_update(FALSE);
    return FALSE;
  }

  static int pos76 = 0;
  static int pos100 = 0;

  if (settings.hpaned_position_auto) {
    if (pos76 < 1 || pos100 < 1) {
      pos76 = settings.hpaned_position_normal;
      pos100 = settings.hpaned_position_maximized;
    }

    GeanyDocument *doc = document_get_current();
    if (doc != NULL) {
      const char *char76 =
          "12345678901234567890123456789012345678901234567890"
          "12345678901234567890213456";
      const char *char100 =
          "12345678901234567890123456789012345678901234567890"
          "12345678901234567890123456789012345678901234567890";
      int col00 = (int)scintilla_send_message(doc->editor->sci,
                                              SCI_POINTXFROMPOSITION, 0, 1);
      const int col76 = (int)scintilla_send_message(
          doc->editor->sci, SCI_TEXTWIDTH, 0, (gulong)char76);
      const int col100 = (int)scintilla_send_message(
          doc->editor->sci, SCI_TEXTWIDTH, 0, (gulong)char100);
      pos76 = col00 + col76;
      pos100 = col00 + col100;
    }
  }

  static gboolean window_maximized_previous = FALSE;
  const gboolean window_maximized_current =
      gtk_window_is_maximized(geany_window);

  if (window_maximized_current == window_maximized_previous) {
    if (settings.hpaned_position_update && window_maximized_current) {
      settings.hpaned_position_maximized =
          gtk_paned_get_position(GTK_PANED(self));
    } else if (settings.hpaned_position_update && !window_maximized_current) {
      settings.hpaned_position_normal = gtk_paned_get_position(GTK_PANED(self));
    }
  } else if (window_maximized_current && settings.hpaned_position_maximized) {
    if (settings.hpaned_position_auto) {
      gtk_paned_set_position(GTK_PANED(self), pos100);
    } else {
      gtk_paned_set_position(GTK_PANED(self),
                             settings.hpaned_position_maximized);
    }
    window_maximized_previous = window_maximized_current;
  } else if (!window_maximized_current && settings.hpaned_position_normal) {
    if (settings.hpaned_position_auto) {
      gtk_paned_set_position(GTK_PANED(self), pos76);
    } else {
      gtk_paned_set_position(GTK_PANED(self), settings.hpaned_position_normal);
    }
    window_maximized_previous = window_maximized_current;
  }

  return FALSE;
}

/* ********************
 * Sidebar Tab Focus Callbacks
 */

static void sidebar_focus_update(gboolean enable) {
  if (enable && !g_handle_grab_notify) {
    g_handle_grab_notify =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "grab-notify",
                         G_CALLBACK(on_grab_notify), NULL);
    g_handle_switch_page =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "switch-page",
                         G_CALLBACK(on_switch_page), NULL);

    g_handle_set_focus_child_sidebar =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "set-focus-child",
                         G_CALLBACK(on_set_focus_child_sidebar), NULL);
    g_handle_set_focus_child_msgwin =
        g_signal_connect(GTK_WIDGET(geany_msgwin), "set-focus-child",
                         G_CALLBACK(on_set_focus_child_msgwin), NULL);
    g_handle_set_focus_child_editor =
        g_signal_connect(GTK_WIDGET(geany_editor), "set-focus-child",
                         G_CALLBACK(on_set_focus_child_editor), NULL);

    g_handle_grab_focus_sidebar =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "grab-focus",
                         G_CALLBACK(on_grab_focus_sidebar), NULL);
    g_handle_grab_focus_msgwin =
        g_signal_connect(GTK_WIDGET(geany_msgwin), "grab-focus",
                         G_CALLBACK(on_grab_focus_msgwin), NULL);
    g_handle_grab_focus_editor =
        g_signal_connect(GTK_WIDGET(geany_editor), "grab-focus",
                         G_CALLBACK(on_grab_focus_editor), NULL);
  }

  if (!enable && g_handle_grab_notify) {
    g_clear_signal_handler(&g_handle_grab_notify, GTK_WIDGET(geany_sidebar));

    g_clear_signal_handler(&g_handle_set_focus_child_sidebar,
                           GTK_WIDGET(geany_sidebar));
    g_clear_signal_handler(&g_handle_set_focus_child_msgwin,
                           GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_set_focus_child_editor,
                           GTK_WIDGET(geany_editor));

    g_clear_signal_handler(&g_handle_grab_focus_sidebar,
                           GTK_WIDGET(geany_sidebar));
    g_clear_signal_handler(&g_handle_grab_focus_msgwin,
                           GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_grab_focus_editor,
                           GTK_WIDGET(geany_editor));
  }
}

// Signal sent when focus has been lost...
static void on_grab_notify(GtkWidget *self, gboolean was_grabbed,
                           gpointer user_data) {
  sidebar_focus_highlight(FALSE);
}

static void on_switch_page(GtkNotebook *self, GtkWidget *page, guint page_num,
                           gpointer user_data) {
  if (gtk_widget_has_focus(GTK_WIDGET(self))) {
    // needs to be delayed to prevent race condition
    // with wrong tab highlighted
    g_timeout_add(50, G_SOURCE_FUNC(sidebar_focus_highlight), (gpointer)TRUE);
  }
}

static void on_grab_focus_sidebar(GtkWidget *self, gpointer user_data) {
  sidebar_focus_highlight(TRUE);
}

static void on_grab_focus_msgwin(GtkWidget *self, gpointer user_data) {
  sidebar_focus_highlight(FALSE);
}

static void on_grab_focus_editor(GtkWidget *self, gpointer user_data) {
  sidebar_focus_highlight(FALSE);
}

static void on_set_focus_child_sidebar(GtkContainer *self, GtkWidget *object,
                                       gpointer user_data) {
  sidebar_focus_highlight(TRUE);
}

static void on_set_focus_child_msgwin(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data) {
  sidebar_focus_highlight(FALSE);
}

static void on_set_focus_child_editor(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data) {
  sidebar_focus_highlight(FALSE);
}

static void sidebar_focus_highlight(gboolean highlight) {
  if (!settings.sidebar_focus_enabled) {
    sidebar_focus_update(FALSE);
    highlight = FALSE;
  }

  gint num_pages = gtk_notebook_get_n_pages(geany_sidebar);
  gint cur_page = gtk_notebook_get_current_page(geany_sidebar);
  GtkWidget *page = gtk_notebook_get_nth_page(geany_sidebar, cur_page);

  for (int i = 0; i < num_pages; i++) {
    GtkWidget *page = gtk_notebook_get_nth_page(geany_sidebar, i);
    gchar *text =
        g_strdup(gtk_notebook_get_tab_label_text(geany_sidebar, page));

    GtkWidget *label = gtk_notebook_get_tab_label(geany_sidebar, page);

    if (highlight && i == cur_page) {
      if (settings.sidebar_focus_bold) {
        gchar *tmp = g_strjoin(NULL, "<b>", text, "</b>", NULL);
        g_free(text);
        text = tmp;
      }
      if (settings.sidebar_focus_color) {
        gchar *tmp =
            g_strjoin(NULL, "<span color='", settings.sidebar_focus_color, "'>",
                      text, "</span>", NULL);
        g_free(text);
        text = tmp;
      }
    }

    gtk_label_set_markup(GTK_LABEL(label), text);
  }
}

/* ********************
 * Keybinding Functions and Callbacks
 */

void on_toggle_editor_sidebar_msgwin() {
  GeanyDocument *doc = document_get_current();
  if (doc != NULL) {
    gint cur_page = gtk_notebook_get_current_page(geany_sidebar);
    GtkWidget *page = gtk_notebook_get_nth_page(geany_sidebar, cur_page);
    page = find_focus_widget(page);

    if (gtk_widget_has_focus(GTK_WIDGET(doc->editor->sci)) &&
        gtk_widget_is_visible(GTK_WIDGET(geany_sidebar))) {
      keybindings_send_command(GEANY_KEY_GROUP_FOCUS, GEANY_KEYS_FOCUS_SIDEBAR);
    } else if (gtk_widget_has_focus(page) &&
               gtk_widget_is_visible(GTK_WIDGET(geany_msgwin))) {
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
