# ======================================================================
#  This file is part of ffDiaporama
#  ffDiaporama is a tools to make diaporama as video
#  Copyright (C) 2011-2014 Dominique Levray <domledom@laposte.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# ======================================================================

#-------------------------------------------------------------
# SYNTAXE IS :
#   QMAKE PREFIX=xxx ffDiaporama_texturemate.pro
#       xxx could be /usr, /usr/local or /opt
#--------------------------------------------------------------

isEmpty(PREFIX) {
    PREFIX = /usr
}

QMAKE_STRIP             = echo
APPFOLDER               = ffDiaporama
TEMPLATE                = aux

#--------------------------------------------------------------
# INSTALLATION
#--------------------------------------------------------------

Asphalt.path        = $$PREFIX/share/$$APPFOLDER/background/texturemate/Asphalt
Asphalt.files       = Asphalt/*
Brick.path          = $$PREFIX/share/$$APPFOLDER/background/texturemate/Brick
Brick.files         = Brick/*
Cement.path         = $$PREFIX/share/$$APPFOLDER/background/texturemate/Cement
Cement.files        = Cement/*
Clay.path           = $$PREFIX/share/$$APPFOLDER/background/texturemate/Clay
Clay.files          = Clay/*
Clouds.path         = $$PREFIX/share/$$APPFOLDER/background/texturemate/Clouds
Clouds.files        = Clouds/*
Cobblestone.path    = $$PREFIX/share/$$APPFOLDER/background/texturemate/Cobblestone
Cobblestone.files   = Cobblestone/*
Digital.path        = $$PREFIX/share/$$APPFOLDER/background/texturemate/Digital
Digital.files       = Digital/*
Fabric.path         = $$PREFIX/share/$$APPFOLDER/background/texturemate/Fabric
Fabric.files        = Fabric/*
Glass.path          = $$PREFIX/share/$$APPFOLDER/background/texturemate/Glass
Glass.files         = Glass/*
Granite.path        = $$PREFIX/share/$$APPFOLDER/background/texturemate/Granite
Granite.files       = Granite/*
Metal.path          = $$PREFIX/share/$$APPFOLDER/background/texturemate/Metal
Metal.files         = Metal/*
Paint.path          = $$PREFIX/share/$$APPFOLDER/background/texturemate/Paint
Paint.files         = Paint/*
Paper.path          = $$PREFIX/share/$$APPFOLDER/background/texturemate/Paper
Paper.files         = Paper/*
Plants.path         = $$PREFIX/share/$$APPFOLDER/background/texturemate/Plants
Plants.files        = Plants/*
Wall.path           = $$PREFIX/share/$$APPFOLDER/background/texturemate/Wall
Wall.files          = Wall/*
Wood.path           = $$PREFIX/share/$$APPFOLDER/background/texturemate/Wood
Wood.files          = Wood/*

INSTALLS += Asphalt Brick Cement Clay Clouds Cobblestone Digital Fabric Glass Granite Metal Paint Paper Plants Wall Wood
