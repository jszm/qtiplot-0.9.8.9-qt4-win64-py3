#!/usr/bin/python

############################################################################
#                                                                          #
# File                 : python-includepath.py                             #
# Project              : QtiPlot                                           #
# Description          : dump -I paths for Python and SIP                  #
# Copyright            : (C) 2007-2009 Knut Franke (knut.franke*gmx.de)    #
#                        (replace * with @ in the email address)           #
#                                                                          #
############################################################################
#                                                                          #
#  This program is free software; you can redistribute it and/or modify    #
#  it under the terms of the GNU General Public License as published by    #
#  the Free Software Foundation; either version 2 of the License, or       #
#  (at your option) any later version.                                     #
#                                                                          #
#  This program is distributed in the hope that it will be useful,         #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of          #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
#  GNU General Public License for more details.                            #
#                                                                          #
#   You should have received a copy of the GNU General Public License      #
#   along with this program; if not, write to the Free Software            #
#   Foundation, Inc., 51 Franklin Street, Fifth Floor,                     #
#   Boston, MA  02110-1301  USA                                            #
#                                                                          #
############################################################################

from __future__ import print_function

import os
from distutils import sysconfig


def normalize(path):
    return path.replace("\\", "/")


paths = [sysconfig.get_python_inc()]

pyqt_prefix = os.environ.get("QTIPLOT_PYQT_PREFIX")
if not pyqt_prefix:
    try:
        from PyQt4 import pyqtconfig
        config = pyqtconfig.Configuration()
        pyqt_prefix = os.path.dirname(os.path.dirname(config.sip_bin))
    except Exception:
        pyqt_prefix = None

if pyqt_prefix:
    sip_inc = os.path.join(pyqt_prefix, "include")
    if os.path.exists(os.path.join(sip_inc, "sip.h")):
        paths.append(sip_inc)

print(" ".join(normalize(path) for path in paths))
