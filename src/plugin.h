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

#ifndef XITWEAKS_PLUGIN_H
#define XITWEAKS_PLUGIN_H

#include <glib.h>
#include <gtk/gtk.h>

//#include <Scintilla.h>
//#include <ScintillaWidget.h>

#include <ctype.h>
#include <errno.h>
#include <geanyplugin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern GeanyKeyGroup *keybindings_get_core_group(guint id);

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern class TweakSettings settings;

enum TweakShortcuts {
  TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN,
  TWEAKS_KEY_TOGGLE_VISIBILITY_MENUBAR,
  TWEAKS_KEY_COPY,
  TWEAKS_KEY_PASTE_1,
  TWEAKS_KEY_PASTE_2,

  TWEAKS_KEY_COUNT,
};

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

static void on_switch_page_sidebar(GtkNotebook *self, GtkWidget *page,
                                   guint page_num, gpointer user_data);
static gboolean on_select_page_sidebar(GtkNotebook *self, gboolean object,
                                       gpointer user_data);
static void on_set_focus_child_sidebar(GtkContainer *self, GtkWidget *object,
                                       gpointer user_data);
static void on_grab_focus_sidebar(GtkWidget *self, gpointer user_data);
static void on_grab_notify_sidebar(GtkWidget *self, gpointer user_data);

static void on_switch_page_msgwin(GtkNotebook *self, GtkWidget *page,
                                  guint page_num, gpointer user_data);
static gboolean on_select_page_msgwin(GtkNotebook *self, gboolean object,
                                      gpointer user_data);
static void on_set_focus_child_msgwin(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data);
static void on_grab_focus_msgwin(GtkWidget *self, gpointer user_data);

static void on_switch_page_editor(GtkNotebook *self, GtkWidget *page,
                                  guint page_num, gpointer user_data);
static gboolean on_select_page_editor(GtkNotebook *self, gboolean object,
                                      gpointer user_data);
static void on_set_focus_child_editor(GtkContainer *self, GtkWidget *object,
                                      gpointer user_data);
static void on_grab_focus_editor(GtkWidget *self, gpointer user_data);

static gboolean sidebar_focus_highlight(gboolean highlight);

// Preferences Callbacks
static gboolean reload_config(gpointer user_data);
static void on_pref_reload_config(GtkWidget *self = nullptr,
                                  GtkWidget *dialog = nullptr);
static void on_pref_save_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog);
static void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog);
static void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog);
static void on_menu_preferences(GtkWidget *self, GtkWidget *dialog);

// Keybinding Functions and Callbacks
static void on_switch_focus_editor_sidebar_msgwin();
static bool hide_menubar();
static void on_toggle_visibility_menubar();
static bool on_key_binding(int key_id);
static GtkWidget *find_focus_widget(GtkWidget *widget);

// Geany Signal Callbacks
static void on_startup_signal(GObject *obj, GeanyDocument *doc,
                              gpointer user_data);
static void on_document_signal(GObject *obj, GeanyDocument *doc,
                               gpointer user_data);
static void on_project_signal(GObject *obj, GKeyFile *config,
                              gpointer user_data);
static bool on_editor_notify(GObject *obj, GeanyEditor *editor,
                             SCNotification *notif, gpointer user_data);

// Other functions
static gboolean show_column_markers(gpointer user_data = nullptr);

#define GEANY_PSC(sig, cb)                                                  \
  plugin_signal_connect(geany_plugin, nullptr, (sig), TRUE, G_CALLBACK(cb), \
                        nullptr)

#define GFREE(_z_) \
  do {             \
    g_free(_z_);   \
    _z_ = nullptr; \
  } while (0)

#define GSTRING_FREE(_z_)     \
  do {                        \
    g_string_free(_z_, TRUE); \
    _z_ = nullptr;            \
  } while (0)

#define GERROR_FREE(_z_) \
  do {                   \
    g_error_free(_z_);   \
    _z_ = nullptr;       \
  } while (0)

#ifndef G_SOURCE_FUNC
#define G_SOURCE_FUNC(f) ((GSourceFunc)(void (*)(void))(f))
#endif  // G_SOURCE_FUNC

#ifndef g_clear_signal_handler
#include "gobject/gsignal.h"
// g_clear_signal_handler was added in glib 2.62
#define g_clear_signal_handler(handler, instance)      \
  do {                                                 \
    if (handler != nullptr && *handler != 0) {         \
      g_signal_handler_disconnect(instance, *handler); \
      *handler = 0;                                    \
    }                                                  \
  } while (0)
#endif  // g_clear_signal_handler

#endif  // XITWEAKS_PLUGIN_H
