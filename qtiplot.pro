TEMPLATE = subdirs

CONFIG	+= ordered

SUBDIRS = 	fitPlugins \
			3rdparty/qwt \
			3rdparty/qwtplot3d \
			qtiplot/exportEMF.pro \
			qtiplot/importOPJ.pro \
			qtiplot/qtiplot.pro

# EMF export is Windows-only (EmfEngine needs GDI/emf.h); Graph::exportEMF
# no-ops when the plugin is absent
unix:SUBDIRS -= qtiplot/exportEMF.pro
