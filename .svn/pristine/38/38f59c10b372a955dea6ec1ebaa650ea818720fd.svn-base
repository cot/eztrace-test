
SUBDIRS = tools doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = fxt.pc

EXTRA_DIST = kernel INSTALL.FKT bootstrap

dist-hook:
	rm -rf `find $(distdir)/kernel -name CVS`
