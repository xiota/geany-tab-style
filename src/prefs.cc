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

#include "auxiliary.h"
#include "prefs.h"

// Global Variables
struct TweakSettings settings;

// Functions

void open_settings() {
  std::string conf_fn =
      cstr_assign_free(g_build_filename(geany_data->app->configdir, "plugins",
                                        "xitweaks", "xitweaks.conf", nullptr));
  std::string conf_dn = g_path_get_dirname(conf_fn.c_str());
  g_mkdir_with_parents(conf_dn.c_str(), 0755);

  GKeyFile *kf = g_key_file_new();

  // if file does not exist, create it
  if (!g_file_test(conf_fn.c_str(), G_FILE_TEST_EXISTS)) {
    save_default_settings();
  }

  g_key_file_load_from_file(
      kf, conf_fn.c_str(),
      GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS),
      nullptr);

  init_settings();
  load_settings(kf);

  GKEY_FILE_FREE(kf);
}

void save_default_settings() {
  std::string conf_fn =
      cstr_assign_free(g_build_filename(geany_data->app->configdir, "plugins",
                                        "xitweaks", "xitweaks.conf", nullptr));
  std::string conf_dn = cstr_assign_free(g_path_get_dirname(conf_fn.c_str()));
  g_mkdir_with_parents(conf_dn.c_str(), 0755);

  // delete if file exists
  GFile *file = g_file_new_for_path(conf_fn.c_str());
  if (!g_file_trash(file, nullptr, nullptr)) {
    g_file_delete(file, nullptr, nullptr);
  }

  // copy default config
  std::string contents = file_get_contents(TWEAKS_CONFIG);
  if (!contents.empty()) {
    file_set_contents(conf_fn, contents);
  }

  g_object_unref(file);
}

void save_settings() {
  GKeyFile *kf = g_key_file_new();
  std::string fn =
      cstr_assign_free(g_build_filename(geany_data->app->configdir, "plugins",
                                        "xitweaks", "xitweaks.conf", nullptr));

  // Load old contents in case user changed file outside of GUI
  g_key_file_load_from_file(
      kf, fn.c_str(),
      GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS),
      nullptr);

  // Update settings with new contents
  SET_KEY(boolean, "sidebar_focus_enabled", settings.sidebar_focus_enabled);
  SET_KEY(boolean, "sidebar_focus_bold", settings.sidebar_focus_bold);
  SET_KEY(string, "sidebar_focus_color", settings.sidebar_focus_color);

  SET_KEY(boolean, "sidebar_save_size_enabled",
          settings.sidebar_save_size_enabled);
  SET_KEY(boolean, "sidebar_save_size_update",
          settings.sidebar_save_size_update);
  SET_KEY(integer, "sidebar_save_size_normal",
          settings.sidebar_save_size_normal);
  SET_KEY(integer, "sidebar_save_size_maximized",
          settings.sidebar_save_size_maximized);

  SET_KEY(boolean, "sidebar_auto_size_enabled",
          settings.sidebar_auto_size_enabled);
  SET_KEY(integer, "sidebar_auto_size_normal",
          settings.sidebar_auto_size_normal);
  SET_KEY(integer, "sidebar_auto_size_maximized",
          settings.sidebar_auto_size_maximized);

  SET_KEY(boolean, "hide_menubar", settings.hide_menubar);

  SET_KEY(boolean, "column_marker_enable", settings.column_marker_enable);

  g_key_file_set_integer_list(kf, PLUGIN_GROUP, "column_marker_columns",
                              settings.column_marker_columns,
                              settings.column_marker_count);

  g_key_file_set_integer_list(kf, PLUGIN_GROUP, "column_marker_colors",
                              settings.column_marker_colors,
                              settings.column_marker_count);

  // Store back on disk
  std::string contents =
      cstr_assign_free(g_key_file_to_data(kf, nullptr, nullptr));
  if (!contents.empty()) {
    file_set_contents(fn, contents);
  }

  GKEY_FILE_FREE(kf);
}

void load_settings(GKeyFile *kf) {
  if (!g_key_file_has_group(kf, "tweaks")) {
    return;
  }

  LOAD_KEY_BOOLEAN(sidebar_focus_enabled, false);
  LOAD_KEY_BOOLEAN(sidebar_focus_bold, false);
  LOAD_KEY_STRING(sidebar_focus_color, "green");

  LOAD_KEY_BOOLEAN(sidebar_save_size_enabled, true);
  LOAD_KEY_BOOLEAN(sidebar_save_size_update, true);
  LOAD_KEY_INTEGER(sidebar_save_size_normal, 0, 0);
  LOAD_KEY_INTEGER(sidebar_save_size_maximized, 0, 0);

  LOAD_KEY_BOOLEAN(sidebar_auto_size_enabled, false);
  LOAD_KEY_INTEGER(sidebar_auto_size_normal, 76, 0);
  LOAD_KEY_INTEGER(sidebar_auto_size_maximized, 100, 0);

  LOAD_KEY_BOOLEAN(hide_menubar, false);

  LOAD_KEY_BOOLEAN(column_marker_enable, true);

  if (settings.column_marker_columns != nullptr ||
      settings.column_marker_colors != nullptr) {
    settings.column_marker_count = 0;
    GFREE(settings.column_marker_columns);
    GFREE(settings.column_marker_colors);
  }

  gsize len_a = 0;
  gsize len_b = 0;

  int *tmp_columns = g_key_file_get_integer_list(
      kf, PLUGIN_GROUP, "column_marker_columns", &len_a, nullptr);

  int *tmp_colors = g_key_file_get_integer_list(
      kf, PLUGIN_GROUP, "column_marker_colors", &len_b, nullptr);

  int tmp_count = len_a < len_b ? len_a : len_b;

  if (tmp_count > 0 || tmp_columns != nullptr || tmp_colors != nullptr) {
    GFREE(settings.column_marker_columns);
    GFREE(settings.column_marker_colors);

    settings.column_marker_count = tmp_count;
    settings.column_marker_columns = tmp_columns;
    settings.column_marker_colors = tmp_colors;
  } else {
    GFREE(tmp_columns);
    GFREE(tmp_colors);
  }
}

void init_settings() {
  settings.sidebar_focus_enabled = false;
  settings.sidebar_focus_bold = false;
  settings.sidebar_focus_color = g_strdup("green");

  settings.sidebar_save_size_enabled = true;
  settings.sidebar_save_size_update = true;
  settings.sidebar_save_size_normal = 0;
  settings.sidebar_save_size_maximized = 0;

  settings.sidebar_auto_size_enabled = false;
  settings.sidebar_auto_size_normal = 76;
  settings.sidebar_auto_size_maximized = 100;

  settings.hide_menubar = false;

  settings.column_marker_count = 13;
  settings.column_marker_columns = (int *)g_malloc(13 * sizeof(int));
  settings.column_marker_colors = (int *)g_malloc(13 * sizeof(int));

  // Colors are in BGR order
  ADD_COLUMN_MARKER(0, 60, 0xe5e5e5);
  ADD_COLUMN_MARKER(1, 72, 0xffd0b0);  // blue
  ADD_COLUMN_MARKER(2, 80, 0xffc0ff);  // purple
  ADD_COLUMN_MARKER(3, 88, 0xe5e5e5);
  ADD_COLUMN_MARKER(4, 96, 0xa0b0ff);  // red
  ADD_COLUMN_MARKER(5, 104, 0xe5e5e5);
  ADD_COLUMN_MARKER(6, 112, 0xe5e5e5);
  ADD_COLUMN_MARKER(7, 120, 0xe5e5e5);
  ADD_COLUMN_MARKER(8, 128, 0xe5e5e5);
  ADD_COLUMN_MARKER(9, 136, 0xe5e5e5);
  ADD_COLUMN_MARKER(10, 144, 0xe5e5e5);
  ADD_COLUMN_MARKER(11, 152, 0xe5e5e5);
  ADD_COLUMN_MARKER(12, 160, 0xe5e5e5);
}
