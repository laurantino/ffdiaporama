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

#include "BasicDefines.h"

//======================================================================
// Internal log defines and functions
//======================================================================

int         LogMsgLevel=LOGMSG_INFORMATION;     // Level from wich debug message was print to stdout
QStringList EventList;                          // Internal event queue
QObject     *EventReceiver=NULL;                // Windows wich receive event

//====================================================================================================================

void PostEvent(int EventType,QString EventParam) {
    EventList.append(QString("%1###;###%2").arg(EventType).arg(EventParam));
    if (EventReceiver!=NULL) QApplication::postEvent(EventReceiver,new QEvent(BaseAppEvent));
}

//====================================================================================================================

bool    PreviousBreak=true;
QMutex  LogMutex;

void ToLog(int MessageType,QString Message,QString Source,bool AddBreak) {
    LogMutex.lock();
    if ((MessageType>=LogMsgLevel)&&(PreviousBreak)) {
        QString DateTime=QTime::currentTime().toString("hh:mm:ss.zzz");
        QString MSG="";
        if ((Message!="FFMPEG: No accelerated colorspace conversion found from yuv422p to rgb24.")&&
            (Message!="FFMPEG: Increasing reorder buffer to 1")&&
            (!Message.startsWith("FFMPEG: Reference"))&&
            (!Message.startsWith("FFMPEG: error while decoding MB"))&&
            (!Message.startsWith("FFMPEG: left block unavailable for requested"))&&
            (!Message.startsWith("FFMPEG: max_analyze_duration reached"))&&
            (!(Message.startsWith("FFMPEG: mode:") && Message.contains("parity:") && (Message.contains("auto_enable:")||Message.contains("deint:"))))&&
            (!(Message.startsWith("FFMPEG: w:") && Message.contains("h:") && Message.contains("pixfmt:")))
           ) {
            switch (MessageType) {
                case LOGMSG_DEBUGTRACE:    MSG=QString("["+DateTime+":DEBUG]\t"     +Message+(AddBreak?"\n":""));  break;
                case LOGMSG_INFORMATION:   MSG=QString("["+DateTime+":INFO]\t"      +Message+(AddBreak?"\n":""));  break;
                case LOGMSG_WARNING:       MSG=QString("["+DateTime+":WARNING]\t"   +Message+(AddBreak?"\n":""));  break;
                case LOGMSG_CRITICAL:      MSG=QString("["+DateTime+":ERROR]\t"     +Message+(AddBreak?"\n":""));  break;
            }
        }
        if (!MSG.isEmpty()) {
            if ((MessageType!=LOGMSG_DEBUGTRACE)&&(EventReceiver!=NULL)) PostEvent(EVENT_GeneralLogChanged,QString("%1###:###%2###:###%3").arg((int)MessageType).arg(Message).arg(Source));
            std::cout << MSG.toLocal8Bit().constData() << std::flush;
        }
        PreviousBreak=((AddBreak)||(Message.endsWith("\n")));
    } else if (MessageType>=LogMsgLevel) {
        std::cout << Message.toLocal8Bit().constData() << std::flush;
    }
    LogMutex.unlock();
}

void QTMessageOutput(QtMsgType type,const QMessageLogContext &,const QString &msg) {
    switch (type) {
      case QtInfoMsg:     ToLog(LOGMSG_INFORMATION,msg);  break;
      case QtDebugMsg:    ToLog(LOGMSG_INFORMATION,msg);  break;
      case QtWarningMsg:  ToLog(LOGMSG_INFORMATION,msg);  break;
      case QtCriticalMsg: ToLog(LOGMSG_CRITICAL,msg);     break;
      case QtFatalMsg:    ToLog(LOGMSG_CRITICAL,msg);     break;
    }
}

//====================================================================================================================

double GetDoubleValue(QDomElement CorrectElement,QString Name) {
    QString sValue=CorrectElement.attribute(Name);
    bool    IsOk=true;
    double  dValue=sValue.toDouble(&IsOk);
    if (!IsOk) {
        for (int i=0;i<sValue.length();i++) if (sValue[i]==',') sValue[i]='.';
        //sValue=sValue.replace(",",".");
        dValue=sValue.toDouble(&IsOk);
    }
    return dValue;
}

double GetDoubleValue(QString sValue) {
    bool    IsOk=true;
    double  dValue=sValue.toDouble(&IsOk);
    if (!IsOk) {
        sValue=sValue.replace(",",".");
        dValue=sValue.toDouble(&IsOk);
    }
    return dValue;
}
