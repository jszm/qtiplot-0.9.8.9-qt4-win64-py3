QTI_ROOT = ..
!include( $$QTI_ROOT/build.conf ) {
  message( "You need a build.conf file with local settings!" )
}

INCLUDEPATH	+= $$QWT_INCLUDEPATH
INCLUDEPATH	+= $$GSL_INCLUDEPATH

include(src/core/core.pri)
include(src/lib/libqti.pri)
include(src/table/table.pri)
include(src/scripting/scripting.pri)
include(src/plot2D/plot2D.pri)
include(src/matrix/matrix.pri)

HEADERS =
SOURCES =

include(src/plugins/exportEMF/exportEMF.pri)

MOC_DIR		= ../tmp/qtiplot
OBJECTS_DIR	= ../tmp/qtiplot
QT			+= qt3support network
TEMPLATE	= lib
CONFIG		+= plugin release static warn_on thread
TARGET		= $$qtLibraryTarget(FreeSoftwareQtiPlotExportEMF)
DESTDIR		= ../tmp/qtiplot
