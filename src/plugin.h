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

G_BEGIN_DECLS

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern struct TweakSettings settings;

enum TweakShortcuts {
  TWEAKS_KEY_SWITCH_FOCUS_EDITOR_SIDEBAR_MSGWIN,
  TWEAKS_KEY_TOGGLE_VISIBILITY_MENUBAR,
};

#define GEANY_PSC(sig, cb) \
  plugin_signal_connect(geany_plugin, NULL, (sig), TRUE, G_CALLBACK(cb), NULL)

#define GFREE(_z_) \
  do {             \
    g_free(_z_);   \
    _z_ = NULL;    \
  } while (0)

#define GSTRING_FREE(_z_)     \
  do {                        \
    g_string_free(_z_, TRUE); \
    _z_ = NULL;               \
  } while (0)

#define GERROR_FREE(_z_) \
  do {                   \
    g_error_free(_z_);   \
    _z_ = NULL;          \
  } while (0)

#define REGEX_CHK(_tp, _str) \
  g_regex_match_simple((_tp), (_str), G_REGEX_CASELESS, 0)

G_END_DECLS

#endif  // XITWEAKS_PLUGIN_H
