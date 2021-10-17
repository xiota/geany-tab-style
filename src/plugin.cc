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

#include <time.h>

#include <string>

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
static void on_switch_page(GtkNotebook *self, GtkWidget *page, guint page_num,
                           gpointer user_data);
static gboolean on_select_page(GtkNotebook *self, gboolean object,
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
static gboolean sidebar_focus_highlight(gboolean highlight);

// Preferences Callbacks
static void on_pref_reload_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog);
static void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog);
static void on_menu_preferences(GtkWidget *self, GtkWidget *dialog);

// Keybinding Functions and Callbacks
static void on_switch_focus_editor_sidebar_msgwin();
static void on_toggle_visibility_menubar();
static bool on_key_binding(int key_id);
static GtkWidget *find_focus_widget(GtkWidget *widget);

// Geany Signal Callbacks
static void on_document_signal(GObject *obj, GeanyDocument *doc,
                               gpointer user_data);

// Other functions
static void show_column_markers(GeanyDocument *doc = nullptr);

/* ********************
 * Globals
 */
GeanyPlugin *geany_plugin;
GeanyData *geany_data;

static GtkWindow *geany_window = nullptr;
static GtkNotebook *geany_sidebar = nullptr;
static GtkNotebook *geany_msgwin = nullptr;
static GtkNotebook *geany_editor = nullptr;
static GtkWidget *geany_hpane = nullptr;
static GtkWidget *geany_menubar = nullptr;

GtkWidget *g_tweaks_menu = nullptr;
static GeanyDocument *g_current_doc = nullptr;

static gulong g_handle_set_focus_child_sidebar = 0;
static gulong g_handle_set_focus_child_msgwin = 0;
static gulong g_handle_set_focus_child_editor = 0;
static gulong g_handle_grab_focus_sidebar = 0;
static gulong g_handle_grab_focus_msgwin = 0;
static gulong g_handle_grab_focus_editor = 0;
static gulong g_handle_switch_page = 0;
static gulong g_handle_select_page = 0;
static gulong g_handle_pane_position = 0;

static gulong g_handle_tab_object = 0;
static GtkWidget *g_tab_object = nullptr;

static clock_t g_lost_focus_clock = 0;
static clock_t g_gain_focus_clock = 0;

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
  GEANY_PSC("document-filetype-set", on_document_signal);

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
  geany_menubar = ui_lookup_widget(GTK_WIDGET(geany_window), "hbox_menubar");

  // set up menu
  GtkWidget *item;

  g_tweaks_menu = gtk_menu_item_new_with_label("Xi/Tweaks");
  ui_add_document_sensitive(g_tweaks_menu);

  GtkWidget *submenu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(g_tweaks_menu), submenu);

  item = gtk_menu_item_new_with_label("Edit Config File");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_edit_config), nullptr);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Reload Config File");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_reload_config),
                   nullptr);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Open Config Folder");
  g_signal_connect(item, "activate", G_CALLBACK(on_pref_open_config_folder),
                   nullptr);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  item = gtk_menu_item_new_with_label("Preferences");
  g_signal_connect(item, "activate", G_CALLBACK(on_menu_preferences), nullptr);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

  gtk_widget_show_all(g_tweaks_menu);

  gtk_menu_shell_append(GTK_MENU_SHELL(geany_data->main_widgets->tools_menu),
                        g_tweaks_menu);

  // Set keyboard shortcuts
  GeanyKeyGroup *group = plugin_set_key_group(
      geany_plugin, _("Xi/Tweaks"), 2, (GeanyKeyGroupCallback)on_key_binding);

  keybindings_set_item(
      group, TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN, nullptr, 0,
      GdkModifierType(0), "xitweaks_switch_focus_editor_sidebar_msgwin",
      _("Switch focus among editor, sidebar, and message window."), nullptr);

  keybindings_set_item(group, TWEAKS_KEY_TOGGLE_VISIBILITY_MENUBAR, nullptr, 0,
                       GdkModifierType(0),
                       "xitweaks_toggle_visibility_menubar_",
                       _("Toggle visibility of the menubar."), nullptr);

  // Enable features
  pane_position_update(settings.sidebar_save_size_enabled ||
                       settings.sidebar_auto_size_enabled);
  sidebar_focus_update(settings.sidebar_focus_enabled);
  g_lost_focus_clock = g_gain_focus_clock = clock();

  if (settings.hide_menubar) {
    gtk_widget_hide(geany_menubar);
  } else {
    gtk_widget_show(geany_menubar);
  }

  show_column_markers();

  return true;
}

static void tweaks_cleanup(GeanyPlugin *plugin, gpointer data) {
  gtk_widget_destroy(g_tweaks_menu);

  sidebar_focus_update(false);
  pane_position_update(false);

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
  gtk_box_pack_start(GTK_BOX(box), btn, false, false, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup(
      "Reload settings from the config file.  May be used "
      "to apply preferences after editing without restarting Geany.");
  btn = gtk_button_new_with_label("Reload Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_reload_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, false, false, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup(
      "Delete the current config file and restore the default "
      "file with explanatory comments.");
  btn = gtk_button_new_with_label("Reset Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_reset_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, false, false, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup("Open the config file in Geany for editing.");
  btn = gtk_button_new_with_label("Edit Config");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_edit_config), dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, false, false, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  tooltip = g_strdup(
      "Open the config folder in the default file manager.  The config folder "
      "contains the stylesheets, which may be edited.");
  btn = gtk_button_new_with_label("Open Config Folder");
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_open_config_folder),
                   dialog);
  gtk_box_pack_start(GTK_BOX(box), btn, false, false, 3);
  gtk_widget_set_tooltip_text(btn, tooltip);
  GFREE(tooltip);

  return box;
}

/* ********************
 * Preferences Callbacks
 */

static void on_pref_reload_config(GtkWidget *self, GtkWidget *dialog) {
  open_settings();

  pane_position_update(settings.sidebar_save_size_enabled ||
                       settings.sidebar_auto_size_enabled);
  sidebar_focus_update(settings.sidebar_focus_enabled);

  if (settings.hide_menubar) {
    gtk_widget_hide(geany_menubar);
  } else {
    gtk_widget_show(geany_menubar);
  }
}

static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog) {
  save_settings();
}

static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog) {
  save_default_settings();
}

static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog) {
  char *conf_dn =
      g_build_filename(geany_data->app->configdir, "plugins", nullptr);

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
                                   "xitweaks", "xitweaks.conf", nullptr);
  GeanyDocument *doc = document_open_file(conf_fn, false, nullptr, nullptr);
  document_reload_force(doc, nullptr);
  GFREE(conf_fn);

  if (dialog != nullptr) {
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
    g_handle_pane_position = g_signal_connect(
        GTK_WIDGET(geany_hpane), "draw", G_CALLBACK(on_draw_pane), nullptr);
  }

  if (!enable && g_handle_pane_position) {
    g_clear_signal_handler(&g_handle_pane_position, GTK_WIDGET(geany_hpane));
  }
}

static gboolean on_draw_pane(GtkWidget *self, cairo_t *cr, gpointer user_data) {
  if (!settings.sidebar_save_size_enabled &&
      !settings.sidebar_auto_size_enabled) {
    pane_position_update(false);
    return false;
  }

  int pos_auto_normal = 0;
  int pos_auto_maximized = 0;

  if (settings.sidebar_auto_size_enabled) {
    GeanyDocument *doc = document_get_current();
    if (doc != nullptr) {
      const char *const str_auto_normal =
          g_strnfill(settings.sidebar_auto_size_normal, '0');
      const char *const str_auto_maximized =
          g_strnfill(settings.sidebar_auto_size_maximized, '0');

      const int pos_origin = (int)scintilla_send_message(
          doc->editor->sci, SCI_POINTXFROMPOSITION, 0, 1);
      const int pos_normal = (int)scintilla_send_message(
          doc->editor->sci, SCI_TEXTWIDTH, 0, (gulong)str_auto_normal);
      const int pos_maximized = (int)scintilla_send_message(
          doc->editor->sci, SCI_TEXTWIDTH, 0, (gulong)str_auto_maximized);
      pos_auto_normal = pos_origin + pos_normal;
      pos_auto_maximized = pos_origin + pos_maximized;
    }
  }

  static gboolean window_maximized_previous = false;
  const gboolean window_maximized_current =
      gtk_window_is_maximized(geany_window);

  if (window_maximized_current == window_maximized_previous) {
    // save current sidebar divider position
    if (settings.sidebar_save_size_update) {
      if (window_maximized_current) {
        settings.sidebar_save_size_maximized =
            gtk_paned_get_position(GTK_PANED(self));
      } else {
        settings.sidebar_save_size_normal =
            gtk_paned_get_position(GTK_PANED(self));
      }
    }
  } else if (settings.sidebar_auto_size_enabled) {
    if (window_maximized_current) {
      if (pos_auto_maximized > 100) {
        gtk_paned_set_position(GTK_PANED(self), pos_auto_maximized);
      }
    } else {
      if (pos_auto_normal > 100) {
        gtk_paned_set_position(GTK_PANED(self), pos_auto_normal);
      }
    }
    window_maximized_previous = window_maximized_current;
  } else if (settings.sidebar_save_size_enabled) {
    if (window_maximized_current) {
      if (settings.sidebar_save_size_maximized) {
        gtk_paned_set_position(GTK_PANED(self),
                               settings.sidebar_save_size_maximized);
      }
    } else {
      if (settings.sidebar_save_size_normal) {
        gtk_paned_set_position(GTK_PANED(self),
                               settings.sidebar_save_size_normal);
      }
    }
    window_maximized_previous = window_maximized_current;
  }

  return false;
}

/* ********************
 * Sidebar Tab Focus Callbacks
 */

static void sidebar_focus_update(gboolean enable) {
  if (enable && !g_handle_switch_page) {
    g_handle_switch_page =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "switch-page",
                         G_CALLBACK(on_switch_page), nullptr);
    g_handle_select_page =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "select-page",
                         G_CALLBACK(on_select_page), nullptr);

    g_handle_set_focus_child_sidebar =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "set-focus-child",
                         G_CALLBACK(on_set_focus_child_sidebar), nullptr);
    g_handle_set_focus_child_msgwin =
        g_signal_connect(GTK_WIDGET(geany_msgwin), "set-focus-child",
                         G_CALLBACK(on_set_focus_child_msgwin), nullptr);
    g_handle_set_focus_child_editor =
        g_signal_connect(GTK_WIDGET(geany_editor), "set-focus-child",
                         G_CALLBACK(on_set_focus_child_editor), nullptr);

    g_handle_grab_focus_sidebar =
        g_signal_connect(GTK_WIDGET(geany_sidebar), "grab-focus",
                         G_CALLBACK(on_grab_focus_sidebar), nullptr);
    g_handle_grab_focus_msgwin =
        g_signal_connect(GTK_WIDGET(geany_msgwin), "grab-focus",
                         G_CALLBACK(on_grab_focus_msgwin), nullptr);
    g_handle_grab_focus_editor =
        g_signal_connect(GTK_WIDGET(geany_editor), "grab-focus",
                         G_CALLBACK(on_grab_focus_editor), nullptr);
  }

  if (!enable && g_handle_switch_page) {
    g_clear_signal_handler(&g_handle_switch_page, GTK_WIDGET(geany_sidebar));

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

static void on_switch_page(GtkNotebook *self, GtkWidget *page, guint page_num,
                           gpointer user_data) {
  // msgwin_status_add("switch page");
  if (gtk_widget_has_focus(GTK_WIDGET(self))) {
    // prevent race condition
    g_timeout_add(25, G_SOURCE_FUNC(sidebar_focus_highlight), (gpointer) true);
  }
}

static gboolean on_select_page(GtkNotebook *self, gboolean object,
                               gpointer user_data) {
  // msgwin_status_add("select page");
  if (gtk_widget_has_focus(GTK_WIDGET(self))) {
    // prevent race condition
    sidebar_focus_highlight(true);
  }
  return false;
}

static void on_grab_focus_sidebar(GtkWidget *self, gpointer user_data) {
  // msgwin_status_add("grab focus sidebar");
  sidebar_focus_highlight(true);
}

static void on_grab_focus_msgwin(GtkWidget *self, gpointer user_data) {
  // msgwin_status_add("grab focus msgwin");
  sidebar_focus_highlight(false);
}

static void on_grab_focus_editor(GtkWidget *self, gpointer user_data) {
  // msgwin_status_add("grab focus editor");
  sidebar_focus_highlight(false);
}

static void on_set_focus_child_sidebar(GtkContainer *self, GtkWidget *object,
                                       gpointer user_data) {
  // msgwin_status_add("set focus child sidebar");
  sidebar_focus_highlight(true);
}

static void on_set_focus_child_msgwin(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data) {
  // msgwin_status_add("set focus child msgwin");
  sidebar_focus_highlight(false);
}

static void on_set_focus_child_editor(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data) {
  // msgwin_status_add("set focus child editor");
  sidebar_focus_highlight(false);
}

static gboolean sidebar_focus_highlight(gboolean highlight) {
  static gboolean has_focus = false;

  if (!has_focus && !highlight) {
    return false;
  }

  if (!settings.sidebar_focus_enabled) {
    sidebar_focus_update(false);
    highlight = false;
  }
  if (highlight && clock() - g_lost_focus_clock < 100) {
    return false;
  }
  if (!highlight && clock() - g_gain_focus_clock < 100) {
    return false;
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
        gchar *tmp = g_strjoin(nullptr, "<b>", text, "</b>", nullptr);
        g_free(text);
        text = tmp;
      }
      if (settings.sidebar_focus_color) {
        gchar *tmp =
            g_strjoin(nullptr, "<span color='", settings.sidebar_focus_color,
                      "'>", text, "</span>", nullptr);
        g_free(text);
        text = tmp;
      }
    }

    gtk_label_set_markup(GTK_LABEL(label), text);
  }

  sidebar_focus_update(settings.sidebar_focus_enabled);
  has_focus = highlight;

  if (highlight) {
    g_gain_focus_clock = clock();
  } else {
    g_lost_focus_clock = clock();
  }
  return false;
}

/* ********************
 * Keybinding Functions and Callbacks
 */

static void on_switch_focus_editor_sidebar_msgwin() {
  GeanyDocument *doc = document_get_current();
  if (doc != nullptr) {
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

static void on_toggle_visibility_menubar() {
  if (gtk_widget_is_visible(geany_menubar)) {
    gtk_widget_hide(geany_menubar);
  } else {
    gtk_widget_show(geany_menubar);
  }
}

static bool on_key_binding(int key_id) {
  switch (key_id) {
    case TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN:
      on_switch_focus_editor_sidebar_msgwin();
      break;
    case TWEAKS_KEY_TOGGLE_VISIBILITY_MENUBAR:
      on_toggle_visibility_menubar();
      break;
    default:
      return false;
  }
  return true;
}

static GtkWidget *find_focus_widget(GtkWidget *widget) {
  GtkWidget *focus = nullptr;

  // optimized simple case
  if (GTK_IS_BIN(widget)) {
    focus = find_focus_widget(gtk_bin_get_child(GTK_BIN(widget)));
  } else if (GTK_IS_CONTAINER(widget)) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
    GList *node;

    for (node = children; node && !focus; node = node->next)
      focus = find_focus_widget(GTK_WIDGET(node->data));
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
  if (DOC_VALID(doc) && doc->file_type->id == GEANY_FILETYPES_C) {
    std::string fn{DOC_FILENAME(doc)};
    if (fn[fn.length() - 2] == '.' && fn[fn.length() - 1] == 'h') {
      std::string cc_fn = fn.substr(0, fn.length() - 2);
      cc_fn += ".cc";
      if (g_file_test(cc_fn.c_str(), G_FILE_TEST_EXISTS)) {
        document_set_filetype(doc, filetypes[GEANY_FILETYPES_CPP]);
      }
    }
  }
  show_column_markers(doc);
}

/* ********************
 * Other Functions
 */

static void show_column_markers(GeanyDocument *doc) {
  if (!doc) {
    doc = document_get_current();
  }
  if (settings.column_marker_enable && DOC_VALID(doc)) {
    scintilla_send_message(doc->editor->sci, SCI_SETEDGEMODE, 3, 3);
    scintilla_send_message(doc->editor->sci, SCI_MULTIEDGECLEARALL, 0, 0);

    if (settings.column_marker_columns != nullptr &&
        settings.column_marker_colors != nullptr) {
      for (int i = 0; i < settings.column_marker_count; i++) {
        scintilla_send_message(doc->editor->sci, SCI_MULTIEDGEADDLINE,
                               settings.column_marker_columns[i],
                               settings.column_marker_colors[i]);
      }
    }
  }
}
