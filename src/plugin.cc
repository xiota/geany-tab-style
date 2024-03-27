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

#ifndef DEBUG
#define DEBUG_STATUS_1(arg)                                    \
  do {                                                         \
    std::string _nb;                                           \
    if (arg == geany_sidebar) {                                \
      _nb = "sidebar";                                         \
    } else if (arg == geany_msgwin) {                          \
      _nb = "msgwin";                                          \
    } else if (arg == geany_editor) {                          \
      _nb = "editor";                                          \
    } else {                                                   \
      _nb = "unknown";                                         \
    }                                                          \
    msgwin_status_add(_("%s: %s"), _nb.c_str(), __FUNCTION__); \
  } while (0)
#define DEBUG_STATUS_0() msgwin_status_add(_("%s"), __FUNCTION__)
#else
#define DEBUG_STATUS_1()
#define DEBUG_STATUS_0()
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>

#include "auxiliary.h"
#include "plugin.h"
#include "prefs.h"

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

GtkWidget *g_tweaks_menu = nullptr;
static GeanyDocument *g_current_doc = nullptr;

static gulong g_handle_focus_editor = 0;
static gulong g_handle_focus_msgwin = 0;
static gulong g_handle_focus_sidebar = 0;
static gulong g_handle_grab_focus_editor = 0;
static gulong g_handle_grab_focus_msgwin = 0;
static gulong g_handle_grab_focus_sidebar = 0;
static gulong g_handle_grab_notify_editor = 0;
static gulong g_handle_grab_notify_msgwin = 0;
static gulong g_handle_grab_notify_sidebar = 0;
static gulong g_handle_set_focus_child_editor = 0;
static gulong g_handle_set_focus_child_msgwin = 0;
static gulong g_handle_set_focus_child_sidebar = 0;
static gulong g_handle_switch_page_editor = 0;
static gulong g_handle_switch_page_msgwin = 0;
static gulong g_handle_switch_page_sidebar = 0;

static gulong g_handle_notebook_focus_highlight = 0;

static gulong g_handle_pane_position = 0;
static gulong g_handle_reload_config = 0;

static GeanyKeyGroup *gKeyGroup = nullptr;

/* ********************
 * Plugin Setup
 */
PLUGIN_VERSION_CHECK(211)

PLUGIN_SET_INFO("Xi/Tweaks", "Miscellaneous tweaks for Geany.", "0.01.0",
                "xiota")

void plugin_init(GeanyData *data) {
  GEANY_PSC("geany-startup-complete", on_startup_signal);
  GEANY_PSC("editor-notify", on_editor_notify);
  GEANY_PSC("document-activate", on_document_signal);
  GEANY_PSC("document-new", on_document_signal);
  GEANY_PSC("document-open", on_document_signal);
  GEANY_PSC("document-reload", on_document_signal);
  GEANY_PSC("project-open", on_project_signal);
  GEANY_PSC("project-close", on_project_signal);
  GEANY_PSC("project-save", on_project_signal);

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

  // geany widgets for later use
  geany_window = GTK_WINDOW(geany->main_widgets->window);
  geany_sidebar = GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook);
  geany_msgwin = GTK_NOTEBOOK(geany->main_widgets->message_window_notebook);
  geany_editor = GTK_NOTEBOOK(geany->main_widgets->notebook);
  geany_hpane = ui_lookup_widget(GTK_WIDGET(geany_window), "hpaned1");

  settings.open();

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
  gKeyGroup = plugin_set_key_group(geany_plugin, "Xi/Tweaks", TWEAKS_KEY_COUNT,
                                   (GeanyKeyGroupCallback)on_key_binding);

  keybindings_set_item(
      gKeyGroup, TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN, nullptr, 0,
      GdkModifierType(0), "xitweaks_switch_focus_editor_sidebar_msgwin",
      _("Switch focus among editor, sidebar, and message window."), nullptr);

  if (g_handle_reload_config == 0) {
    g_handle_reload_config = 1;
    g_idle_add(reload_config, nullptr);
  }

  return true;
}

static void tweaks_cleanup(GeanyPlugin *plugin, gpointer data) {
  gtk_widget_destroy(g_tweaks_menu);

  notebook_focus_update(false);

  settings.save();
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

static gboolean reload_config(gpointer user_data) {
  settings.open();

  notebook_focus_update(settings.sidebar_focus_enabled ||
                        settings.notebook_focus_enabled);

  g_handle_reload_config = 0;
  return false;
}

static void on_pref_reload_config(GtkWidget *self, GtkWidget *dialog) {
  if (g_handle_reload_config == 0) {
    g_handle_reload_config = 1;
    g_idle_add(reload_config, nullptr);
  }
}

static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog) {
  settings.save();
}

static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog) {
  settings.save_default();
}

static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog) {
  std::string conf_dn = cstr_assign(
      g_build_filename(geany_data->app->configdir, "plugins", nullptr));

  std::string command = R"(xdg-open ")" + conf_dn + R"(")";
  (void)!system(command.c_str());
}

static void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog) {
  settings.open();
  std::string conf_fn =
      cstr_assign(g_build_filename(geany_data->app->configdir, "plugins",
                                   "xitweaks", "xitweaks.conf", nullptr));
  GeanyDocument *doc =
      document_open_file(conf_fn.c_str(), false, nullptr, nullptr);
  document_reload_force(doc, nullptr);

  if (dialog != nullptr) {
    gtk_widget_destroy(GTK_WIDGET(dialog));
  }
}

static void on_menu_preferences(GtkWidget *self, GtkWidget *dialog) {
  plugin_show_configure(geany_plugin);
}

/* ********************
 * Sidebar Tab Focus Callbacks
 */

static void state_flags_changed(GtkNotebook *self, GtkStateFlags flags,
                                gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void change_current_page(GtkNotebook *self, GtkStateFlags flags,
                                gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void focus(GtkNotebook *self, GtkStateFlags flags, gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void focus_tab(GtkNotebook *self, GtkStateFlags flags,
                      gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void move_focus_out(GtkNotebook *self, GtkStateFlags flags,
                           gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void page_added(GtkNotebook *self, GtkStateFlags flags,
                       gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void page_removed(GtkNotebook *self, GtkStateFlags flags,
                         gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void page_reordered(GtkNotebook *self, GtkStateFlags flags,
                           gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void reorder_tab(GtkNotebook *self, GtkStateFlags flags,
                        gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void select_page(GtkNotebook *self, GtkStateFlags flags,
                        gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void switch_page(GtkNotebook *self, GtkStateFlags flags,
                        gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void set_focus_child(GtkNotebook *self, GtkStateFlags flags,
                            gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}
static void grab_focus(GtkNotebook *self, GtkStateFlags flags,
                       gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}
static void grab_notify(GtkNotebook *self, GtkStateFlags flags,
                        gpointer user_data) {
  DEBUG_STATUS_1(self);

  if (g_handle_notebook_focus_highlight == 0) {
    g_handle_notebook_focus_highlight = 1;
    g_idle_add(notebook_focus_highlight_callback, self);
  }
}

static void notebook_focus_update(gboolean enable) {
  DEBUG_STATUS_0();

  if (enable && !g_handle_focus_editor) {
    g_handle_focus_sidebar =
        g_signal_connect(geany_sidebar, "focus", G_CALLBACK(focus), nullptr);
    g_handle_focus_msgwin =
        g_signal_connect(geany_msgwin, "focus", G_CALLBACK(focus), nullptr);
    g_handle_focus_editor =
        g_signal_connect(geany_editor, "focus", G_CALLBACK(focus), nullptr);

    g_handle_grab_focus_sidebar = g_signal_connect(
        geany_sidebar, "grab-focus", G_CALLBACK(grab_focus), nullptr);
    g_handle_grab_focus_msgwin = g_signal_connect(
        geany_msgwin, "grab-focus", G_CALLBACK(grab_focus), nullptr);
    g_handle_grab_focus_editor = g_signal_connect(
        geany_editor, "grab-focus", G_CALLBACK(grab_focus), nullptr);

    g_handle_grab_notify_sidebar = g_signal_connect(
        geany_sidebar, "grab-notify", G_CALLBACK(grab_notify), nullptr);
    g_handle_grab_notify_msgwin = g_signal_connect(
        geany_msgwin, "grab-notify", G_CALLBACK(grab_notify), nullptr);
    g_handle_grab_notify_editor = g_signal_connect(
        geany_editor, "grab-notify", G_CALLBACK(grab_notify), nullptr);

    g_handle_set_focus_child_sidebar = g_signal_connect(
        geany_sidebar, "set-focus-child", G_CALLBACK(set_focus_child), nullptr);
    g_handle_set_focus_child_msgwin = g_signal_connect(
        geany_msgwin, "set-focus-child", G_CALLBACK(set_focus_child), nullptr);
    g_handle_set_focus_child_editor = g_signal_connect(
        geany_editor, "set-focus-child", G_CALLBACK(set_focus_child), nullptr);

    g_handle_switch_page_sidebar = g_signal_connect(
        geany_sidebar, "switch-page", G_CALLBACK(switch_page), nullptr);
    g_handle_switch_page_msgwin = g_signal_connect(
        geany_msgwin, "switch-page", G_CALLBACK(switch_page), nullptr);
    g_handle_switch_page_editor = g_signal_connect(
        geany_editor, "switch-page", G_CALLBACK(switch_page), nullptr);

    notebook_focus_highlight(true);
  }

  if (!enable && g_handle_focus_editor) {
    g_clear_signal_handler(&g_handle_focus_editor, GTK_WIDGET(geany_editor));
    g_clear_signal_handler(&g_handle_focus_msgwin, GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_focus_sidebar, GTK_WIDGET(geany_sidebar));
    g_clear_signal_handler(&g_handle_grab_focus_editor,
                           GTK_WIDGET(geany_editor));
    g_clear_signal_handler(&g_handle_grab_focus_msgwin,
                           GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_grab_focus_sidebar,
                           GTK_WIDGET(geany_sidebar));
    g_clear_signal_handler(&g_handle_grab_notify_editor,
                           GTK_WIDGET(geany_editor));
    g_clear_signal_handler(&g_handle_grab_notify_msgwin,
                           GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_grab_notify_sidebar,
                           GTK_WIDGET(geany_sidebar));
    g_clear_signal_handler(&g_handle_set_focus_child_editor,
                           GTK_WIDGET(geany_editor));
    g_clear_signal_handler(&g_handle_set_focus_child_msgwin,
                           GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_set_focus_child_sidebar,
                           GTK_WIDGET(geany_sidebar));
    g_clear_signal_handler(&g_handle_switch_page_editor,
                           GTK_WIDGET(geany_editor));
    g_clear_signal_handler(&g_handle_switch_page_msgwin,
                           GTK_WIDGET(geany_msgwin));
    g_clear_signal_handler(&g_handle_switch_page_sidebar,
                           GTK_WIDGET(geany_sidebar));

    notebook_focus_highlight(false);
  }
}

static gboolean notebook_focus_highlight_callback(gpointer user_data) {
  notebook_focus_highlight(true);
  g_handle_notebook_focus_highlight = 0;
  return false;
}

static gboolean notebook_focus_highlight(gboolean highlight) {
  static GtkNotebook *notebooks[3] = {geany_sidebar, geany_editor,
                                      geany_msgwin};

  if (!settings.sidebar_focus_enabled && !settings.notebook_focus_enabled) {
    highlight = false;
  }

  for (GtkNotebook *nb : notebooks) {
    gint num_pages = gtk_notebook_get_n_pages(nb);
    gint cur_page = gtk_notebook_get_current_page(nb);

    for (int i = 0; i < num_pages; i++) {
      GtkWidget *page = gtk_notebook_get_nth_page(nb, i);
      GtkWidget *label = gtk_notebook_get_tab_label(nb, page);

      if (highlight && i == cur_page &&
          (gtk_widget_has_focus(GTK_WIDGET(nb)) ||
           gtk_widget_has_focus(find_focus_widget(GTK_WIDGET(nb))) ||
           gtk_widget_has_focus(page) ||
           gtk_widget_has_focus(find_focus_widget(GTK_WIDGET(page))) ||
           gtk_widget_has_focus(label)) &&
          (settings.notebook_focus_enabled ||
           (settings.sidebar_focus_enabled && nb == geany_sidebar))) {
        gtk_widget_set_name(label, "geany-xitweaks-notebook-tab-focus");
        gtk_widget_set_name(page, "geany-xitweaks-notebook-page-focus");
      } else {
        gtk_widget_set_name(label, "geany-xitweaks-notebook-tab-unfocus");
        gtk_widget_set_name(page, "geany-xitweaks-notebook-page-unfocus");
      }
    }
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
      g_signal_emit_by_name(G_OBJECT(geany_sidebar), "grab-focus", nullptr);
    } else if (gtk_widget_has_focus(page) &&
               gtk_widget_is_visible(GTK_WIDGET(geany_msgwin))) {
      keybindings_send_command(GEANY_KEY_GROUP_FOCUS,
                               GEANY_KEYS_FOCUS_MESSAGE_WINDOW);
      g_signal_emit_by_name(G_OBJECT(geany_sidebar), "grab-notify", nullptr);
    } else {
      keybindings_send_command(GEANY_KEY_GROUP_FOCUS, GEANY_KEYS_FOCUS_EDITOR);
      g_signal_emit_by_name(G_OBJECT(geany_sidebar), "grab-notify", nullptr);
    }
  }
}

static bool on_key_binding(int key_id) {
  switch (key_id) {
    case TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN:
      on_switch_focus_editor_sidebar_msgwin();
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
                               gpointer user_data) {}

static void on_startup_signal(GObject *obj, GeanyDocument *doc,
                              gpointer user_data) {
  if (g_handle_reload_config == 0) {
    g_handle_reload_config = 1;
    g_idle_add(reload_config, nullptr);
  }
}

static void on_project_signal(GObject *obj, GKeyFile *config,
                              gpointer user_data) {}

static bool on_editor_notify(GObject *obj, GeanyEditor *editor,
                             SCNotification *notif, gpointer user_data) {
  switch (notif->nmhdr.code) {
    case SCN_FOCUSIN:
    case SCEN_SETFOCUS:
    case SCN_UPDATEUI:
    case SCEN_CHANGE:
    case SCN_FOCUSOUT:
    case SCEN_KILLFOCUS:
    default:
      if (g_handle_notebook_focus_highlight == 0) {
        g_handle_notebook_focus_highlight = 1;
        g_idle_add(notebook_focus_highlight_callback, nullptr);
      }
      break;
  }
  return false;
}
