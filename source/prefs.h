// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "config.h"
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
  gboolean notebook_focus_enabled = false;
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
