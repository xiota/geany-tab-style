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
TweakSettings settings;

// Functions

void TweakSettings::open() {
  std::string conf_fn =
      cstr_assign(g_build_filename(geany_data->app->configdir, "plugins",
                                        "xitweaks", "xitweaks.conf", nullptr));
  std::string conf_dn = g_path_get_dirname(conf_fn.c_str());
  g_mkdir_with_parents(conf_dn.c_str(), 0755);

  GKeyFile *kf = g_key_file_new();

  // if file does not exist, create it
  if (!g_file_test(conf_fn.c_str(), G_FILE_TEST_EXISTS)) {
    settings.save_default();
  }

  g_key_file_load_from_file(
      kf, conf_fn.c_str(),
      GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS),
      nullptr);

  settings.load(kf);

  GKEY_FILE_FREE(kf);
}

void TweakSettings::save_default() {
  std::string conf_fn =
      cstr_assign(g_build_filename(geany_data->app->configdir, "plugins",
                                        "xitweaks", "xitweaks.conf", nullptr));
  std::string conf_dn = cstr_assign(g_path_get_dirname(conf_fn.c_str()));
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

void TweakSettings::save() {
  GKeyFile *kf = g_key_file_new();
  std::string fn =
      cstr_assign(g_build_filename(geany_data->app->configdir, "plugins",
                                        "xitweaks", "xitweaks.conf", nullptr));

  // Load old contents in case user changed file outside of GUI
  g_key_file_load_from_file(
      kf, fn.c_str(),
      GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS),
      nullptr);

  // Update settings with new contents
  SET_KEY(boolean, "sidebar_focus_enabled", sidebar_focus_enabled);

  SET_KEY(boolean, "menubar_hide_on_start", menubar_hide_on_start);
  SET_KEY(boolean, "menubar_restore_state", menubar_restore_state);

  GtkWidget * geany_menubar = ui_lookup_widget(GTK_WIDGET(geany->main_widgets->window), "hbox_menubar");
  settings.menubar_previous_state = gtk_widget_is_visible(geany_menubar);
  SET_KEY(boolean, "menubar_previous_state", menubar_previous_state);

  // Store back on disk
  std::string contents =
      cstr_assign(g_key_file_to_data(kf, nullptr, nullptr));
  if (!contents.empty()) {
    file_set_contents(fn, contents);
  }

  GKEY_FILE_FREE(kf);
}

void TweakSettings::load(GKeyFile *kf) {
  if (!g_key_file_has_group(kf, "tweaks")) {
    return;
  }

  GET_KEY_BOOLEAN(sidebar_focus_enabled, false);

  GET_KEY_BOOLEAN(menubar_hide_on_start, false);
  GET_KEY_BOOLEAN(menubar_restore_state, false);
  GET_KEY_BOOLEAN(menubar_previous_state, true);
}
