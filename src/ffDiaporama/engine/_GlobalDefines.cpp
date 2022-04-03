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

// Basic inclusions (common to all files)
#include <QDir>
#include "_GlobalDefines.h"

QString CurrentAppName;                         // Application name (including devel, beta, ...)
QString CurrentAppVersion;                      // Application version read from BUILDVERSION.txt
double  ScreenFontAdjust=1;                     // System Font adjustement
double  ScaleFontAdjust=0;
int     SCALINGTEXTFACTOR=700;                  // 700 instead of 400 (ffD 1.0/1.1/1.2) to keep similar display from plaintext to richtext

//====================================================================================================================

QString UpInitials(QString Source) {
    for (int i=0;i<Source.length();i++)
        if ((i==0)||(Source.at(i-1)==' ')) Source[i]=Source.at(i).toUpper();
    return Source;
}

//====================================================================================================================

QString FormatLongDate(QDate EventDate) {
    QLocale loc = QLocale::system();
    return UpInitials(EventDate.toString(loc.dateFormat(QLocale::LongFormat)));
}

//====================================================================================================================

QString GetInformationValue(QString ValueToSearch,QStringList *InformationList) {
    if (!InformationList) return "";
    int i=0;
    while ((i<InformationList->count())&&(!InformationList->at(i).startsWith(ValueToSearch+"##"))) i++;
    if ((i<InformationList->count())&&(InformationList->at(i).startsWith(ValueToSearch))) {
        QStringList Values=InformationList->at(i).split("##");
        if (Values.count()==2) return ((QString)Values[1]).trimmed();
    }
    return "";
}

//====================================================================================================================

QString GetCumulInfoStr(QStringList *InformationList,QString Key1,QString Key2) {
    int     Num     =0;
    QString TrackNum="";
    QString Value   ="";
    QString Info    ="";
    do {
        TrackNum=QString("%1").arg(Num);
        while (TrackNum.length()<3) TrackNum="0"+TrackNum;
        TrackNum=Key1+"_"+TrackNum+":";
        Value=GetInformationValue(TrackNum+Key2,InformationList);
        if (Value!="") Info=Info+((Num>0)?",":"")+Value;
        // Next
        Num++;
    } while (Value!="");
    return Info;
}

//====================================================================================================================

QString ito2a(int val) {
    QString Ret=QString("%1").arg(val);
    while (Ret.length()<2) Ret="0"+Ret;
    return Ret;
}

QString ito3a(int val) {
    QString Ret=QString("%1").arg(val);
    while (Ret.length()<3) Ret="0"+Ret;
    return Ret;
}

//====================================================================================================================

QString GetTextSize(int64_t Size) {
    ToLog(LOGMSG_DEBUGTRACE,"IN:GetTextSize");

    QString UnitStr="";
    int     Unit   =0;

    while ((Size>1024*1024)&&(Unit<2)) {
        Unit++;
        Size=Size/1024;
    }
    switch (Unit) {
        case 0 : UnitStr=QApplication::translate("QCustomFolderTree","Kb","Unit Kb");   break;
        case 1 : UnitStr=QApplication::translate("QCustomFolderTree","Mb","Unit Mb");   break;
        case 2 : UnitStr=QApplication::translate("QCustomFolderTree","Gb","Unit Gb");   break;
        case 3 : UnitStr=QApplication::translate("QCustomFolderTree","Tb","Unit Tb");   break;
    }
    if (Size==0) return "0";
    else if (double(Size)/double(1024)>0.1) return QString("%1").arg(double(Size)/double(1024),8,'f',1).trimmed()+" "+UnitStr;
    else return "<0.1"+UnitStr;
}

//====================================================================================================================

//functions used to retrieve number of processor
//Thanks to : Stuart Nixon
//See : http://lists.trolltech.com/qt-interest/2006-05/thread00922-0.html
int getCpuCount() {
    ToLog(LOGMSG_DEBUGTRACE,"IN:getCpuCount");
    int cpuCount= sysconf(_SC_NPROCESSORS_ONLN);
    if(cpuCount<1) cpuCount=1;
    return cpuCount;
}

//====================================================================================================================
// UTILITY FUNCTIONS
//====================================================================================================================

QAction *CreateMenuAction(QImage *Icon,QString Text,int Data,bool Checkable,bool IsCheck,QWidget *Parent) {
    QAction *Action;
    if (Icon) Action=new QAction(QIcon(QPixmap().fromImage(*Icon)),Text,Parent);
        else Action=new QAction(Text,Parent);
    Action->setIconVisibleInMenu(true);
    Action->setCheckable(Checkable);
    Action->setFont(QFont("Sans Serif",9));
    if (Checkable) Action->setChecked(IsCheck);
    Action->setData(QVariant(Data));
    return Action;
}

//====================================================================================================================

QAction *CreateMenuAction(QIcon Icon,QString Text,int Data,bool Checkable,bool IsCheck,QWidget *Parent) {
    QAction *Action;
    Action=new QAction(Icon,Text,Parent);
    Action->setIconVisibleInMenu(true);
    Action->setCheckable(Checkable);
    Action->setFont(QFont("Sans Serif",9));
    if (Checkable) Action->setChecked(IsCheck);
    Action->setData(QVariant(Data));
    return Action;
}
