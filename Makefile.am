ACLOCAL_AMFLAGS = -I m4

cf_docdir = $(docdir)
cf_doc_DATA = \
	Readme.md

cf_datadir = $(datadir)/geany-plugins/@PACKAGE@
cf_data_DATA = \
	data/xi-tweaks.conf

geany_plugindir = $(libdir)/geany
geany_plugin_LTLIBRARIES = xi_tweaks.la

xi_tweaks_la_CXXFLAGS = $(GEANY_CFLAGS) \
	-DG_LOG_DOMAIN=\""Tweaks"\" \
	-DTWEAKS_README=\""$(cf_docdir)/Readme.md"\" \
	-DTWEAKS_CONFIG=\""$(cf_datadir)/xi-tweaks.conf"\"
xi_tweaks_la_LIBADD = $(GEANY_LIBS)
xi_tweaks_la_LDFLAGS = -module -avoid-version
xi_tweaks_la_SOURCES = \
	src/plugin.cc \
	src/plugin.h \
	src/prefs.cc \
	src/prefs.h \
	src/auxiliary.cc \
	src/auxiliary.h
