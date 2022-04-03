/* ======================================================================
    This file is part of ffDiaporama
    ffDiaporama is a tools to make diaporama as video
    Copyright (C) 2011-2014 Dominique Levray <domledom@laposte.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
   ====================================================================== */

#include "cCustomIcon.h"

//*****************************************************************************************************************************************

cCustomIcon::cCustomIcon() {
}

void cCustomIcon::LoadIcons(QString FileName) {
    QImage Icon(FileName);
    if (Icon.isNull()) ToLog(LOGMSG_CRITICAL,QString("Loading %1 Error").arg(FileName)); else {
        if (Icon.width()>Icon.height()) {
            Icon16 =Icon.scaledToWidth(16,Qt::SmoothTransformation);
            Icon100=Icon.scaledToWidth(100,Qt::SmoothTransformation);
        } else {
            Icon16 =Icon.scaledToHeight(16,Qt::SmoothTransformation);
            Icon100=Icon.scaledToHeight(100,Qt::SmoothTransformation);
        }
    }
}

//====================================================================================================================

void cCustomIcon::LoadIcons(cCustomIcon *CustomIcon) {
    Icon16 =CustomIcon->Icon16.copy();
    Icon100=CustomIcon->Icon100.copy();
}

//====================================================================================================================

void cCustomIcon::LoadIconsFromIMG(QString FileName) {
    Icon16.load (":/img/MediaIcons/16x16/"+FileName);     if (Icon16.isNull())  ToLog(LOGMSG_CRITICAL,QString("Loading img/MediaIcons/16x16/%1 Error").arg(FileName));
    Icon100.load(":/img/MediaIcons/100x100/"+FileName);   if (Icon100.isNull()) ToLog(LOGMSG_CRITICAL,QString("Loading img/MediaIcons/100x100/%1 Error").arg(FileName));
}

//====================================================================================================================

void cCustomIcon::LoadIconsFromLinux(QString LinuxPath,QString FileName) {
    Icon16.load(LinuxPath+"16x16/"+FileName);           if (Icon16.isNull())  ToLog(LOGMSG_CRITICAL,QString("Loading %116x16/%2 Error").arg(LinuxPath).arg(FileName));
    Icon100=QImage(LinuxPath+"128x128/"+FileName);      if (Icon100.isNull()) ToLog(LOGMSG_CRITICAL,QString("Loading %1128x128/%2 Error").arg(LinuxPath).arg(FileName));
    if (!Icon100.isNull()) Icon100=Icon100.scaledToHeight(100,Qt::SmoothTransformation);
}

//====================================================================================================================

void cCustomIcon::LoadIcons(QImage *Image) {
    if (Image->width()>Image->height()) {
        Icon16 =Image->scaledToWidth(16,Qt::SmoothTransformation);
        Icon100=Image->scaledToWidth(100,Qt::SmoothTransformation);
    } else {
        Icon16 =Image->scaledToHeight(16,Qt::SmoothTransformation);
        Icon100=Image->scaledToHeight(100,Qt::SmoothTransformation);
    }
}

//====================================================================================================================

void cCustomIcon::LoadIcons(QIcon Icon) {
    Icon16 =Icon.pixmap(16,16).toImage();
    Icon100=Icon.pixmap(100,100).toImage();
    //if ((Icon100.height()<100)&&(Icon100.width()<100)) {
    if (((Icon100.height()<100)&&(Icon100.width()<100))||(Icon100.height()>100)||(Icon100.width()>100)) {
        if (Icon100.height()>Icon100.width()) Icon100=Icon100.scaledToHeight(100,Qt::SmoothTransformation);
            else Icon100=Icon100.scaledToWidth(100,Qt::SmoothTransformation);
    }
}

//====================================================================================================================

QIcon cCustomIcon::GetIcon() {
    QIcon Ret=QIcon(QPixmap().fromImage(Icon16));
    Ret.addPixmap(QPixmap().fromImage(Icon100));
    return Ret;
}

//====================================================================================================================

QImage *cCustomIcon::GetIcon(IconSize Size) {
    switch (Size) {
        case ICON16:  return &Icon16;
        case ICON100: return &Icon100;
        default:      return &Icon16;
    }
}
