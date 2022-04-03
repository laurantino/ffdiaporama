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
   ======================================================================
    THIS FILE MUST ABSOLUTELY BE REFERENCED AT FIRST IN ALL .h FILES OF
    THE PROJECT
   ====================================================================== */


#ifndef _BASICDEFINES_H
#define _BASICDEFINES_H

//============================================
// Activate standard stdint macro
//============================================

#ifdef _STDINT_H
    #undef _STDINT_H            // Remove previous inclusion (if exist)
#endif

#define __STDC_CONSTANT_MACROS  // Activate macro for stdint
#include <stdint.h>             // Include stdint with macro activated
#include <stdlib.h>
#include <iostream>

#define AVRCAST (AVRational)        // mingw need CAST in struct constant definition
#include <unistd.h>

//============================================
// Minimum QT inclusions needed by all files
//============================================
#include <QtCore>
#include <QApplication>
#include <QtDebug>
#include <QList>
#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QtConcurrent>

//====================================================================
// Math
//====================================================================

#include <cmath>
#define _USE_MATH_DEFINES   // for msvc
#ifndef M_PI
    // Some cmath doesn't define it,try from the older source
    #include "math.h"
#endif

//====================================================================
// Internal log defines and functions
//====================================================================

// Log level for message
#define LOGMSG_DEBUGTRACE                   1
#define LOGMSG_INFORMATION                  2
#define LOGMSG_WARNING                      3
#define LOGMSG_CRITICAL                     4

const QEvent::Type BaseAppEvent = (QEvent::Type)2000;   // The custom event will be send to EventReceiver (if EventReceiver not null)
#define EVENT_GeneralLogChanged             1           // General internal event code to display log message

extern int          LogMsgLevel;                        // Level from wich debug message was print to stdout
extern QStringList  EventList;                          // Internal event queue
extern QObject      *EventReceiver;                     // Windows wich receive event

void    PostEvent(int EventType,QString EventParam="");
void    ToLog(int MessageType,QString Message,QString Source="internal",bool AddBreak=true);
void    QTMessageOutput(QtMsgType type,const QMessageLogContext &,const QString &msg);

//====================================================================
// Standard project geometry definition
//====================================================================

enum ffd_GEOMETRY {
    GEOMETRY_4_3,
    GEOMETRY_16_9,
    GEOMETRY_40_17,
    GEOMETRY_THUMBNAIL,
    GEOMETRY_SQUARE,
    GEOMETRY_NONE
};

//============================================
// Media object types
//============================================

enum OBJECTTYPE {
    OBJECTTYPE_UNMANAGED,
    OBJECTTYPE_MANAGED,
    OBJECTTYPE_FOLDER,
    OBJECTTYPE_FFDFILE,
    OBJECTTYPE_IMAGEFILE,
    OBJECTTYPE_VIDEOFILE,
    OBJECTTYPE_MUSICFILE,
    OBJECTTYPE_THUMBNAIL,
    OBJECTTYPE_IMAGEVECTOR,
    OBJECTTYPE_IMAGECLIPBOARD,
    OBJECTTYPE_GMAPSMAP
};

#define PREVIEWMAXHEIGHT    720         // Max height for preview image

//============================================
// Utility functions
//============================================

double  GetDoubleValue(QDomElement CorrectElement,QString Name);                                        // Load a double value from an XML element
double  GetDoubleValue(QString sValue);                                                                 // Load a double value from a string

#endif // _BASICDEFINES_H
