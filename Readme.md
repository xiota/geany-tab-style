# Xi/Tweaks Plugin for Geany

This plugin provides miscellaneous tweaks for Geany that don't fit anywhere else.

## Features

* Show multiple column markers in the editor.
* Set a keybinding to switch among Editor, Sidebar, and Message Window.
* Highlight sidebar tabs that have keyboard focus.
* Save different sidebar sizes for different window states.
* Auto size the sidebar according to window state.
* Quick access to the Geany user config folder.
* Switch `.h` files to C++ if corresponding `.cc` file found

## Installation

The Xi/Tweaks plugin can be installed on Ubuntu via PPA.
```
sudo add-apt-repository ppa:xiota/geany-plugins
sudo apt-get update
sudo apt-get install geany-plugin-xi-tweaks
```

The plugin can then be enabled in the Plugin Manager (*Tools/Plugin Manager*).

## Requirements

This plugin depends on the following libraries and programs:

* [Geany](https://geany.org/)
* [GTK/Glib](http://www.gtk.org)

## License

The Preview plugin for Geany is licensed under the [GPLv3](License.md) or later.
