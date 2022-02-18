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

#ifndef XITWEAKS_PREFS_H
#define XITWEAKS_PREFS_H

#include "plugin.h"

class TweakSettings {
 public:
  TweakSettings() = default;
  ~TweakSettings() { save(); }

  void open();
  void load(GKeyFile *kf);
  void save();
  void save_default();

 public:
  gboolean sidebar_focus_enabled = false;
};

// Macros to make loading settings easier
#define PLUGIN_GROUP "tweaks"

#define HAS_KEY(key) g_key_file_has_key(kf, PLUGIN_GROUP, (key), nullptr)
#define GET_KEY(T, key) g_key_file_get_##T(kf, PLUGIN_GROUP, (key), nullptr)
#define SET_KEY(T, key, _val) \
  g_key_file_set_##T(kf, PLUGIN_GROUP, (key), (_val))

#define GET_KEY_BOOLEAN(key, def)            \
  do {                                       \
    if (HAS_KEY(#key)) {                     \
      settings.key = GET_KEY(boolean, #key); \
    } else {                                 \
      settings.key = (def);                  \
    }                                        \
  } while (0)

#define GET_KEY_INTEGER(key, def, min)  \
  do {                                  \
    if (HAS_KEY(#key)) {                \
      int val = GET_KEY(integer, #key); \
      if (val) {                        \
        if (val < (min)) {              \
          settings.key = (min);         \
        } else {                        \
          settings.key = val;           \
        }                               \
      } else {                          \
        settings.key = (def);           \
      }                                 \
    }                                   \
  } while (0)

#define GET_KEY_DOUBLE(key, def, min)  \
  do {                                 \
    if (HAS_KEY(#key)) {               \
      int val = GET_KEY(double, #key); \
      if (val) {                       \
        if (val < (min)) {             \
          settings.key = (min);        \
        } else {                       \
          settings.key = val;          \
        }                              \
      } else {                         \
        settings.key = (def);          \
      }                                \
    }                                  \
  } while (0)

#define GKEY_FILE_FREE(_z_) \
  do {                      \
    g_key_file_free(_z_);   \
    _z_ = nullptr;          \
  } while (0)

#endif  // XITWEAKS_PREFS_H
