// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <glib.h>
#include <gtk/gtk.h>

// #include <Scintilla.h>
// #include <ScintillaWidget.h>

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <locale>

#include "geanyplugin.h"

extern GeanyKeyGroup *keybindings_get_core_group(guint id);

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern class TweakSettings settings;

enum TweakShortcuts {
  TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN,

  TWEAKS_KEY_COUNT,
};

// Plugin Setup
gboolean tweaks_init(GeanyPlugin *plugin, gpointer data);
void tweaks_cleanup(GeanyPlugin *plugin, gpointer data);
GtkWidget *tweaks_configure(GeanyPlugin *plugin, GtkDialog *dialog,
                                   gpointer pdata);

// Sidebar Tab Focus Callbacks
void notebook_focus_update(gboolean enable);

gboolean notebook_focus_highlight_callback(gpointer user_data);
gboolean notebook_focus_highlight(gboolean highlight);

// Preferences Callbacks
gboolean reload_config(gpointer user_data);
void on_pref_reload_config(GtkWidget *self = nullptr,
                                  GtkWidget *dialog = nullptr);
void on_pref_save_config(GtkWidget *self, GtkWidget *dialog);
void on_pref_reset_config(GtkWidget *self, GtkWidget *dialog);
void on_pref_open_config_folder(GtkWidget *self, GtkWidget *dialog);
void on_pref_edit_config(GtkWidget *self, GtkWidget *dialog);
void on_menu_preferences(GtkWidget *self, GtkWidget *dialog);

// Keybinding Functions and Callbacks
void on_switch_focus_editor_sidebar_msgwin();
bool on_key_binding(int key_id);
GtkWidget *find_focus_widget(GtkWidget *widget);

// Geany Signal Callbacks
void on_startup_signal(GObject *obj, GeanyDocument *doc,
                              gpointer user_data);
void on_document_signal(GObject *obj, GeanyDocument *doc,
                               gpointer user_data);
void on_project_signal(GObject *obj, GKeyFile *config,
                              gpointer user_data);
bool on_editor_notify(GObject *obj, GeanyEditor *editor,
                             SCNotification *notif, gpointer user_data);

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
