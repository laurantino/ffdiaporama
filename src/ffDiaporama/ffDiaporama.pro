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

isEmpty(PREFIX) {
    PREFIX = /usr
}

CONFIG += qt thread

QT += widgets concurrent help multimedia core gui xml svg sql

QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-overloaded-virtual

QMAKE_STRIP  = echo
APPFOLDER    = ffDiaporama
TARGET       = ffDiaporama
TEMPLATE     = app

#--------------------------------------------------------------
# Add link to ffDiaporama_lib
#--------------------------------------------------------------

INCLUDEPATH += ../ffDiaporama_lib
LIBS        += -lffDiaporama_lib

#--------------------------------------------------------------
# DEFINES $$DESTDIR DIRECTORIES, COMMON INCLUDES AND COMMON LIBS
#--------------------------------------------------------------

DEFINES +=SHARE_DIR=\\\"$$PREFIX\\\"

LIBS   += -L../ffDiaporama_lib -lswresample -lavformat -lavcodec -lavutil -lswscale -lavfilter -lexiv2

HARDWARE_PLATFORM = $$system(uname -m)
contains(HARDWARE_PLATFORM,x86_64) {
	DEFINES+=Q_OS_LINUX64
	message("Linux x86_64 build")
} else {
        DEFINES+=Q_OS_LINUX32
	message("Linux x86 build")
}

OTHER_FILES += ffDiaporama.rc

#--------------------------------------------------------------
# PROJECT FILES
#--------------------------------------------------------------

# Ressource files
RESOURCES   += RSCffDiaporama.qrc

# Source files
SOURCES +=  MainWindow/cCustomSlideTable.cpp \
            MainWindow/mainwindow.cpp \
            DlgRenderVideo/DlgRenderVideo.cpp \
            DlgManageStyle/DlgManageStyle.cpp \
            DlgAbout/DlgAbout.cpp \
            DlgTransition/DlgTransitionProperties.cpp \
            DlgMusic/DlgMusicProperties.cpp \
            DlgMusic/DlgEditMusic.cpp \
            DlgMusic/DlgAdjustToSound.cpp \
            DlgBackground/DlgBackgroundProperties.cpp \
            DlgAppSettings/DlgManageDevices/DlgManageDevices.cpp \
            DlgAppSettings/DlgApplicationSettings.cpp \
            DlgImage/wgt_QEditImage/cImgInteractiveZone.cpp \
            DlgImage/wgt_QEditImage/wgt_QEditImage.cpp \
            DlgImage/wgt_QEditVideo/wgt_QEditVideo.cpp \
            DlgImage/DlgImageCorrection.cpp \
            DlgSlide/cCustomBlockTable.cpp \
            DlgSlide/DlgSlideProperties.cpp \
            DlgSlide/cInteractiveZone.cpp \
            DlgSlide/cCustomShotTable.cpp \
            DlgSlide/DlgRuler/DlgRulerDef.cpp \
            DlgSlide/cShotComposer.cpp \
            DlgSlide/DlgSlideDuration.cpp \
            DlgSlide/DlgImageComposer.cpp \
            DlgText/cCustomTextEdit.cpp \
            DlgText/DlgTextEdit.cpp \
            DlgCheckConfig/DlgCheckConfig.cpp \
            DlgInfoFile/DlgInfoFile.cpp \
            DlgffDPjrProperties/DlgffDPjrProperties.cpp \
            DlgManageFavorite/DlgManageFavorite.cpp \
            DlgWorkingTask/DlgWorkingTask.cpp \
            DlgTransition/DlgTransitionDuration.cpp \
            DlgFileExplorer/DlgFileExplorer.cpp \
            DlgChapter/DlgChapter.cpp \
            DlgAutoTitleSlide/cCustomTitleModelTable.cpp \
            DlgAutoTitleSlide/DlgAutoTitleSlide.cpp \
            DlgExportProject/DlgExportProject.cpp \
            HelpPopup/HelpPopup.cpp \
            HelpPopup/HelpBrowser.cpp \
            engine/_GlobalDefines.cpp \
            engine/cApplicationConfig.cpp \
            engine/cDeviceModelDef.cpp \
            engine/cSoundBlockList.cpp \
            engine/cBaseMediaFile.cpp \
            engine/cBrushDefinition.cpp \
            engine/cDriveList.cpp \
            engine/_Transition.cpp \
            engine/_EncodeVideo.cpp \
            engine/_StyleDefinitions.cpp \
            engine/_Diaporama.cpp \
            engine/_Variables.cpp \
            engine/_Model.cpp \
            CustomCtrl/_QCustomDialog.cpp \
            CustomCtrl/cCFramingComboBox.cpp \
            CustomCtrl/cCShapeComboBox.cpp \
            CustomCtrl/cThumbnailComboBox.cpp \
            CustomCtrl/cQDateTimeEdit.cpp \
            CustomCtrl/QCustomRuler.cpp \
            CustomCtrl/QMovieLabel.cpp \
            wgt_QMultimediaBrowser/QCustomFolderTable.cpp \
            wgt_QMultimediaBrowser/QCustomFolderTree.cpp \
            wgt_QMultimediaBrowser/wgt_QMultimediaBrowser.cpp \
            wgt_QVideoPlayer/wgt_QVideoPlayer.cpp \
            HelpPopup/HelpContent.cpp \
            main.cpp

# Header files
HEADERS  += MainWindow/cCustomSlideTable.h \
            MainWindow/mainwindow.h \
            DlgRenderVideo/DlgRenderVideo.h \
            DlgManageStyle/DlgManageStyle.h \
            DlgAbout/DlgAbout.h \
            DlgTransition/DlgTransitionProperties.h \
            DlgMusic/DlgMusicProperties.h \
            DlgMusic/DlgEditMusic.h \
            DlgMusic/DlgAdjustToSound.h \
            DlgBackground/DlgBackgroundProperties.h \
            DlgAppSettings/DlgManageDevices/DlgManageDevices.h \
            DlgAppSettings/DlgApplicationSettings.h \
            DlgImage/wgt_QEditImage/cImgInteractiveZone.h \
            DlgImage/wgt_QEditImage/wgt_QEditImage.h \
            DlgImage/wgt_QEditVideo/wgt_QEditVideo.h \
            DlgImage/DlgImageCorrection.h \
            DlgSlide/DlgSlideProperties.h \
            DlgSlide/cCustomBlockTable.h \
            DlgSlide/cInteractiveZone.h \
            DlgSlide/cCustomShotTable.h \
            DlgSlide/DlgRuler/DlgRulerDef.h \
            DlgSlide/cShotComposer.h \
            DlgSlide/DlgSlideDuration.h \
            DlgSlide/DlgImageComposer.h \
            DlgText/cCustomTextEdit.h \
            DlgText/DlgTextEdit.h \
            DlgCheckConfig/DlgCheckConfig.h \
            DlgInfoFile/DlgInfoFile.h \
            DlgffDPjrProperties/DlgffDPjrProperties.h \
            DlgManageFavorite/DlgManageFavorite.h \
            DlgWorkingTask/DlgWorkingTask.h \
            DlgTransition/DlgTransitionDuration.h \
            DlgFileExplorer/DlgFileExplorer.h \
            DlgChapter/DlgChapter.h \
            DlgAutoTitleSlide/cCustomTitleModelTable.h \
            DlgAutoTitleSlide/DlgAutoTitleSlide.h \
            DlgExportProject/DlgExportProject.h \
            HelpPopup/HelpPopup.h \
            HelpPopup/HelpBrowser.h \
            engine/cApplicationConfig.h \
            engine/cDeviceModelDef.h \
            engine/_GlobalDefines.h \
            engine/cSoundBlockList.h \
            engine/cBaseMediaFile.h \
            engine/cBrushDefinition.h \
            engine/cDriveList.h \
            engine/_Transition.h \
            engine/_EncodeVideo.h \
            engine/_StyleDefinitions.h \
            engine/_Diaporama.h \
            engine/_Variables.h \
            engine/_Model.h \
            CustomCtrl/_QCustomDialog.h \
            CustomCtrl/cCFramingComboBox.h \
            CustomCtrl/cCShapeComboBox.h \
            CustomCtrl/cThumbnailComboBox.h \
            CustomCtrl/cQDateTimeEdit.h \
            CustomCtrl/QCustomRuler.h \
            CustomCtrl/QMovieLabel.h \
            wgt_QMultimediaBrowser/QCustomFolderTable.h \
            wgt_QMultimediaBrowser/QCustomFolderTree.h \
            wgt_QMultimediaBrowser/wgt_QMultimediaBrowser.h \
            wgt_QVideoPlayer/wgt_QVideoPlayer.h \
            HelpPopup/HelpContent.h

# Forms files
FORMS    += MainWindow/mainwindow.ui \
            DlgRenderVideo/DlgRenderVideo.ui \
            DlgManageStyle/DlgManageStyle.ui \
            DlgAbout/DlgAbout.ui \
            DlgTransition/DlgTransitionProperties.ui \
            DlgMusic/DlgMusicProperties.ui \
            DlgMusic/DlgEditMusic.ui \
            DlgBackground/DlgBackgroundProperties.ui \
            DlgAppSettings/DlgManageDevices/DlgManageDevices.ui \
            DlgAppSettings/DlgApplicationSettings.ui \
            DlgImage/wgt_QEditImage/wgt_QEditImageimage.ui \
            DlgImage/wgt_QEditVideo/wgt_QEditVideo.ui \
            DlgImage/DlgImageCorrection.ui \
            DlgSlide/DlgSlideProperties.ui \
            DlgSlide/DlgRuler/DlgRulerDef.ui \
            DlgSlide/DlgImageComposer.ui \
            DlgText/DlgTextEdit.ui \
            DlgCheckConfig/DlgCheckConfig.ui \
            DlgInfoFile/DlgInfoFile.ui \
            DlgffDPjrProperties/DlgffDPjrProperties.ui \ 
            DlgManageFavorite/DlgManageFavorite.ui \
            DlgWorkingTask/DlgWorkingTask.ui \
            DlgTransition/DlgTransitionDuration.ui \
            DlgSlide/DlgSlideDuration.ui \
            DlgFileExplorer/DlgFileExplorer.ui \
            DlgChapter/DlgChapter.ui \
            DlgAutoTitleSlide/DlgAutoTitleSlide.ui \
            DlgExportProject/DlgExportProject.ui \
            wgt_QMultimediaBrowser/wgt_QMultimediaBrowser.ui \
            wgt_QVideoPlayer/wgt_QVideoPlayer.ui \
            HelpPopup/HelpPopup.ui \
            DlgMusic/DlgAdjustToSound.ui

#--------------------------------------------------------------
# INSTALLATION
#--------------------------------------------------------------

TARGET.path         = $$PREFIX/bin
TARGET.files        = $$TARGET
INSTALLS 	   += TARGET

Licences.path       = $$PREFIX/share/$$APPFOLDER
Licences.files      = ../../authors.txt \
                      ../../LICENSE
INSTALLS           += Licences

XMLConfig.path      = $$PREFIX/share/$$APPFOLDER
XMLConfig.files     = ../../Devices.xml \
                      ../../ffDiaporama.xml
INSTALLS           += XMLConfig

General.path        = $$PREFIX/share/$$APPFOLDER
General.files       = ../../BUILDVERSION.txt
INSTALLS           += General

ico.path            = $$PREFIX/share/icons/hicolor/32x32/apps
ico.files           = ../../ffdiaporama.png
INSTALLS 	   += ico

desktop.path        = $$PREFIX/share/applications
desktop.files       = ../../ffDiaporama.desktop
INSTALLS 	   += desktop

mimefile.path       = $$PREFIX/share/mime/packages
mimefile.files      = ../../ffDiaporama-mime.xml
INSTALLS 	   += mimefile

translation.path    = $$PREFIX/share/$$APPFOLDER/locale
translation.files   = ../../locale/*.qm ../../locale/*.qch ../../locale/*.qhc
INSTALLS 	   += translation

