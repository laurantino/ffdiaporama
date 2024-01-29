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

// Include some common various class
#include "cApplicationConfig.h"

// Include some additional standard class
#include "CustomCtrl/_QCustomDialog.h"
#include <QFileDialog>
#include <QPainter>

// Include some additional standard class
#include "cBaseMediaFile.h"
#include "_Diaporama.h"

//****************************************************************************************************************************************************************
// EXIV2 PART
//****************************************************************************************************************************************************************
#include <exiv2/exif.hpp>
#include <exiv2/exiv2.hpp>
bool Exiv2WithPreview=true;
int  Exiv2MajorVersion=EXIV2_MAJOR_VERSION;
int  Eviv2MinorVersion=EXIV2_MINOR_VERSION;
int  Exiv2PatchVersion=EXIV2_PATCH_VERSION;

//****************************************************************************************************************************************************************

#define FFD_APPLICATION_ROOTNAME    "Project"           // Name of root node in the project xml file
#define MaxAudioLenDecoded          AVCODEC_MAX_AUDIO_FRAME_SIZE*4

#ifndef INT64_MAX
    #define 	INT64_MAX   0x7fffffffffffffffLL
    #define 	INT64_MIN   (-INT64_MAX - 1LL)
#endif

#define VC_ERROR        0x00000001
#define VC_BUFFER       0x00000002
#define VC_PICTURE      0x00000004
#define VC_USERDATA     0x00000008
#define VC_FLUSHED      0x00000010

#define PIXFMT          AV_PIX_FMT_RGB24
#define QTPIXFMT        QImage::Format_RGB888

//****************************************************************************************************************************************************************

// from Google music manager (see:http://code.google.com/p/gogglesmm/source/browse/src/gmutils.cpp?spec=svn6c3dbecbad40ee49736b9ff7fe3f1bfa6ca18c13&r=6c3dbecbad40ee49736b9ff7fe3f1bfa6ca18c13)
bool gm_decode_base64(uchar *buffer,uint &len) {
    static const unsigned char base64[256]={
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x3e,0x80,0x80,0x80,0x3f,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
    0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x80,0x80,0x80,0x80,0x80,
    0x80,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};

    uint  pos=0;
    uchar v;
    for (uint i=0,b=0;i<len;i++) {
        v=base64[buffer[i]];
        if (v!=0x80) {
          switch(b) {
            case 0: buffer[pos]=(v<<2);
                    b++;
                    break;
            case 1: buffer[pos++]|=(v>>4);
                    buffer[pos]=(v<<4);
                    b++;
                    break;
            case 2: buffer[pos++]|=(v>>2);
                    buffer[pos]=(v<<6);
                    b++;
                    break;
            case 3: buffer[pos++]|=v;
                    b=0;
                    break;
            }
        } else {
            if (buffer[i]=='=' && b>1) {
                len=pos;
                return true;
            } else return false;
        }
    }
    len=pos;
    return true;
}

//====================================================================================================================

cReplaceObjectList::cReplaceObjectList() {
}

void cReplaceObjectList::SearchAppendObject(QString SourceFileName) {
    int i=0;
    while ((i<List.count())&&(List[i].SourceFileName!=SourceFileName)) i++;
    if ((i<List.count())&&(List[i].SourceFileName==SourceFileName)) {
        // Object already define

    } else {
        // Object not yet define
        QString DestFileName=QFileInfo(SourceFileName).baseName()+"."+QFileInfo(SourceFileName).completeSuffix();
        // Search if DestFileName already exist
        bool Cont=true;
        int  Num =0;
        while (Cont) {
            Cont=false;
            int j=0;
            while ((j<List.count())&&(List[j].DestFileName!=DestFileName)) j++;
            if ((j<List.count())&&(List[j].DestFileName==DestFileName)) {
                // DestFileName already define
                Cont=true;
                Num++;
                DestFileName=QFileInfo(SourceFileName).baseName()+QString("-%1").arg(Num)+"."+QFileInfo(SourceFileName).completeSuffix();
            }
        }
        List.append(cReplaceObject(SourceFileName,DestFileName));
    }
}

QString cReplaceObjectList::GetDestinationFileName(QString SourceFileName) {
    for (int i=0;i<List.count();i++) if (List[i].SourceFileName==SourceFileName) return List[i].DestFileName;
    return SourceFileName;
}

//*********************************************************************************************************************************************
// Base class object
//*********************************************************************************************************************************************

cBaseMediaFile::cBaseMediaFile(cApplicationConfig *TheApplicationConfig) {
    ApplicationConfig   = TheApplicationConfig;
    ObjectType          = OBJECTTYPE_UNMANAGED;
    ObjectName          = "NoName";
    Reset();
}

void cBaseMediaFile::Reset() {
    FileKey             = -1;
    FolderKey           = -1;
    RessourceKey        = -1;
    IsValide            = false;                                    // if true then object if initialise
    IsInformationValide = false;                                    // if true then information list if fuly initialise
    ObjectGeometry      = IMAGE_GEOMETRY_UNKNOWN;                   // Image geometry
    FileSize            = 0;
    ImageWidth          = 0;                                        // Widht of normal image
    ImageHeight         = 0;                                        // Height of normal image
    CreatDateTime       = QDateTime(QDate(0,0,0),QTime(0,0,0));     // Original date/time
    ModifDateTime       = QDateTime(QDate(0,0,0),QTime(0,0,0));     // Last modified date/time
    AspectRatio         = 1;
    ImageOrientation    = -1;

    // Analyse
    GivenDuration           = QTime(0,0,0);
    RealAudioDuration       = QTime(0,0,0);
    RealVideoDuration       = QTime(0,0,0);
    IsComputedAudioDuration = false;
    IsComputedVideoDuration = false;
    SoundLevel              = -1;
}

//====================================================================================================================

cBaseMediaFile::~cBaseMediaFile() {
}

//====================================================================================================================

QTime cBaseMediaFile::GetRealDuration() {
    if (IsComputedAudioDuration && IsComputedVideoDuration) return RealAudioDuration<RealVideoDuration?RealAudioDuration:RealVideoDuration;
        else if (IsComputedAudioDuration) return RealAudioDuration;
        else if (IsComputedVideoDuration) return RealVideoDuration;
        else                              return GivenDuration;
}

QTime cBaseMediaFile::GetRealAudioDuration() {
    if (IsComputedAudioDuration) return RealAudioDuration; else return GivenDuration;
}

QTime cBaseMediaFile::GetRealVideoDuration() {
    if (IsComputedVideoDuration) return RealVideoDuration; else return GivenDuration;
}

QTime cBaseMediaFile::GetGivenDuration() {
    return GivenDuration;
}

//====================================================================================================================

void cBaseMediaFile::SetGivenDuration(QTime GivenDuration) {
    this->GivenDuration=GivenDuration;
}

void cBaseMediaFile::SetRealAudioDuration(QTime RealDuration) {
    IsComputedAudioDuration=true;
    RealAudioDuration      =RealDuration;
}

void cBaseMediaFile::SetRealVideoDuration(QTime RealDuration) {
    IsComputedVideoDuration=true;
    RealVideoDuration      =RealDuration;
}

//====================================================================================================================

QString cBaseMediaFile::FileName() {
    if (CachedFileName.isEmpty()) CachedFileName=ApplicationConfig->FoldersTable->GetFolderPath(FolderKey)+ApplicationConfig->FilesTable->GetShortName(FileKey);
    return CachedFileName;
}

//====================================================================================================================

QString cBaseMediaFile::ShortName() {
     if (CachedFileName.isEmpty()) CachedFileName=ApplicationConfig->FoldersTable->GetFolderPath(FolderKey)+ApplicationConfig->FilesTable->GetShortName(FileKey);
    return QFileInfo(CachedFileName).fileName();
}

//====================================================================================================================

bool cBaseMediaFile::LoadAnalyseSound(QList<qreal> *Peak,QList<qreal> *Moyenne) {
    int64_t RealAudioDuration,RealVideoDuration;
    bool    IsOk=ApplicationConfig->FilesTable->GetAnalyseSound(FileKey,Peak,Moyenne,&RealAudioDuration,ObjectType==OBJECTTYPE_VIDEOFILE?&RealVideoDuration:NULL,&SoundLevel);
    if (IsOk) {
        SetRealAudioDuration(QTime(0,0,0,0).addMSecs(RealAudioDuration));
        if (ObjectType==OBJECTTYPE_VIDEOFILE) SetRealVideoDuration(QTime(0,0,0,0).addMSecs(RealVideoDuration));
    }
    return IsOk;
}

//====================================================================================================================

void cBaseMediaFile::SaveAnalyseSound(QList<qreal> *Peak,QList<qreal> *Moyenne,qreal MaxMoyenneValue) {
    int64_t RealVDuration=(ObjectType==OBJECTTYPE_VIDEOFILE)?QTime(0,0,0,0).msecsTo(GetRealVideoDuration()):0;
    SoundLevel=MaxMoyenneValue;
    ApplicationConfig->FilesTable->SetAnalyseSound(FileKey,Peak,Moyenne,QTime(0,0,0,0).msecsTo(GetRealAudioDuration()),(ObjectType==OBJECTTYPE_VIDEOFILE?&RealVDuration:NULL),SoundLevel);
}

//====================================================================================================================

QImage cBaseMediaFile::GetIcon(cCustomIcon::IconSize Size,bool useDelayed) {
    QImage Icon16,Icon100;
    ApplicationConfig->FilesTable->GetThumbs(FileKey,&Icon16,&Icon100);
    if (Size==cCustomIcon::ICON16) {
        if (Icon16.isNull()) {
            if (useDelayed) Icon16=ApplicationConfig->DefaultDelayedIcon.GetIcon(cCustomIcon::ICON16)->copy();
                else Icon16=GetDefaultTypeIcon(cCustomIcon::ICON16)->copy();
        }
        return Icon16;
    } else {
        if (Icon100.isNull()) {
            if (useDelayed) Icon100=ApplicationConfig->DefaultDelayedIcon.GetIcon(cCustomIcon::ICON100)->copy();
                else Icon100=GetDefaultTypeIcon(cCustomIcon::ICON100)->copy();
        }
        return Icon100;
    }
}

//====================================================================================================================

bool cBaseMediaFile::GetFullInformationFromFile(bool IsPartial) {
    cCustomIcon Icon;
    QStringList ExtendedProperties;
    IsInformationValide=ApplicationConfig->FilesTable->GetExtendedProperties(FileKey,&ExtendedProperties)&&ApplicationConfig->FilesTable->GetThumbs(FileKey,&Icon.Icon16,&Icon.Icon100);
    if (!IsInformationValide) {
        IsInformationValide=GetChildFullInformationFromFile(IsPartial,&Icon,&ExtendedProperties);
        if (IsInformationValide) {
            QDomDocument domDocument;
            QDomElement  root=domDocument.createElement("BasicProperties");
            domDocument.appendChild(root);
            SaveBasicInformationToDatabase(&root,"","",false,NULL,NULL,false);
            IsInformationValide=ApplicationConfig->FilesTable->SetBasicProperties(FileKey,domDocument.toString())&&
                                ApplicationConfig->FilesTable->SetExtendedProperties(FileKey,&ExtendedProperties)&&
                                ApplicationConfig->FilesTable->SetThumbs(FileKey,&Icon.Icon16,&Icon.Icon100);
        }
    }
    return IsInformationValide;
}

//====================================================================================================================

bool cBaseMediaFile::GetInformationFromFile(QString FileName,QStringList *AliasList,bool *ModifyFlag,qlonglong GivenFolderKey) {
    if (ModifyFlag) *ModifyFlag=false;
    if ((!CachedFileName.isEmpty())&&(CachedFileName!=FileName)) CachedFileName="";

    // Use aliaslist
    if ((AliasList)&&(!QFileInfo(FileName).exists())) {
        // First test : seach for a new path+filename for this filename
        int i;
        for (i=0;(i<AliasList->count())&&(!AliasList->at(i).startsWith(FileName));i++);
        if ((i<AliasList->count())&&(AliasList->at(i).startsWith(FileName))) {
            FileName=AliasList->at(i);
            if (FileName.indexOf("####")>0) FileName=FileName.mid(FileName.indexOf("####")+QString("####").length());
        } else {
            // Second test : use each remplacement folder to try to find find
            i=0;
            QString NewFileName=QFileInfo(FileName).absoluteFilePath();
            while ((i<AliasList->count())&&(!QFileInfo(NewFileName).exists())) {
                QString OldName=AliasList->at(i);
                QString NewName=OldName.mid(OldName.indexOf("####")+QString("####").length());
                OldName=OldName.left(OldName.indexOf("####"));
                OldName=OldName.left(OldName.lastIndexOf(QDir::separator()));
                NewName=NewName.left(NewName.lastIndexOf(QDir::separator()));
                NewFileName=NewName+QDir::separator()+QFileInfo(FileName).fileName();
                i++;
            }
            if (QFileInfo(NewFileName).exists()) {
                FileName=NewFileName;
                if (AliasList) AliasList->append(FileName+"####"+NewFileName);
                if (ModifyFlag) *ModifyFlag=true;
            }
        }
    }

    bool Continue=true;
    while ((Continue)&&(!QFileInfo(FileName).exists())) {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        if (CustomMessageBox(NULL,QMessageBox::Question,QApplication::translate("cBaseMediaFile","Open file"),
            QApplication::translate("cBaseMediaFile","Impossible to open file ")+FileName+"\n"+QApplication::translate("cBaseMediaFile","Do you want to select another file ?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)!=QMessageBox::Yes)
            Continue=false;
        else {
            QString NewFileName=QFileDialog::getOpenFileName(ApplicationConfig->TopLevelWindow,QApplication::translate("cBaseMediaFile","Select another file for ")+QFileInfo(FileName).fileName(),
                    ApplicationConfig->RememberLastDirectories?QDir::toNativeSeparators(ApplicationConfig->SettingsTable->GetTextValue(QString("%1_path").arg(BrowserTypeDef[ObjectType==OBJECTTYPE_IMAGEFILE?BROWSER_TYPE_IMAGEONLY:ObjectType==OBJECTTYPE_VIDEOFILE?BROWSER_TYPE_VIDEOONLY:BROWSER_TYPE_SOUNDONLY].BROWSERString),DefaultMediaPath)):DefaultMediaPath,
                    ApplicationConfig->GetFilterForMediaFile(ObjectType==OBJECTTYPE_IMAGEFILE?IMAGEFILE:
                                                             ObjectType==OBJECTTYPE_IMAGEVECTOR?IMAGEVECTORFILE:
                                                             ObjectType==OBJECTTYPE_VIDEOFILE?VIDEOFILE:
                                                             MUSICFILE));
            if (NewFileName!="") {
                if (AliasList) AliasList->append(FileName+"####"+NewFileName);
                FileName=NewFileName;
                if (ApplicationConfig->RememberLastDirectories) ApplicationConfig->SettingsTable->SetTextValue(QString("%1_path").arg(BrowserTypeDef[ObjectType==OBJECTTYPE_IMAGEFILE?BROWSER_TYPE_IMAGEONLY:ObjectType==OBJECTTYPE_VIDEOFILE?BROWSER_TYPE_VIDEOONLY:BROWSER_TYPE_SOUNDONLY].BROWSERString),QFileInfo(FileName).absolutePath());     // Keep folder for next use
                if (ModifyFlag) *ModifyFlag=true;
            } else Continue=false;
        }
        QApplication::restoreOverrideCursor();
    }
    if (!Continue) {
        ToLog(LOGMSG_CRITICAL,QApplication::translate("cBaseMediaFile","Impossible to open file %1").arg(FileName));
        return false;
    }

    FileName =QFileInfo(FileName).absoluteFilePath();
    if (FolderKey==-1)  FolderKey=GivenFolderKey>=0?GivenFolderKey:ApplicationConfig->FoldersTable->GetFolderKey(QFileInfo(FileName).absolutePath());
    if (FileKey==-1)    FileKey  =ApplicationConfig->FilesTable->GetFileKey(FolderKey,QFileInfo(FileName).fileName(),ObjectType);

    QString BasicInfo;
    if (ApplicationConfig->FilesTable->GetBasicProperties(FileKey,&BasicInfo,FileName,&FileSize,&CreatDateTime,&ModifDateTime)) {
        QDomDocument    domDocument;

        QString         errorStr;
        int             errorLine,errorColumn;
        if ((domDocument.setContent(BasicInfo,true,&errorStr,&errorLine,&errorColumn))&&
            (domDocument.elementsByTagName("BasicProperties").length()>0)&&
            (domDocument.elementsByTagName("BasicProperties").item(0).isElement()==true)
            ) {
            QDomElement Element=domDocument.elementsByTagName("BasicProperties").item(0).toElement();
            IsValide=LoadBasicInformationFromDatabase(&Element,"","",AliasList,ModifyFlag,NULL,false);
            return IsValide;
        }
    }

    FileSize     =QFileInfo(FileName).size();
    ModifDateTime=QFileInfo(FileName).metadataChangeTime();
    CreatDateTime=QFileInfo(FileName).lastModified();

    IsValide=true;
    return true;
}

//====================================================================================================================

QString cBaseMediaFile::GetImageGeometryStr() {
    switch (ObjectGeometry) {
        case IMAGE_GEOMETRY_3_2     : return "3:2";
        case IMAGE_GEOMETRY_2_3     : return "2:3";
        case IMAGE_GEOMETRY_4_3     : return "4:3";
        case IMAGE_GEOMETRY_3_4     : return "3:4";
        case IMAGE_GEOMETRY_16_9    : return "16:9";
        case IMAGE_GEOMETRY_9_16    : return "9:16";
        case IMAGE_GEOMETRY_40_17   : return "40:17";
        case IMAGE_GEOMETRY_17_40   : return "17:40";
        default                     : return "";        //QApplication::translate("cBaseMediaFile","ns","Non standard image geometry");
    }
}

//====================================================================================================================

QString cBaseMediaFile::GetFileSizeStr() {
    return GetTextSize(FileSize);
}

//====================================================================================================================

QString cBaseMediaFile::GetFileDateTimeStr(bool Created) {
    if (Created) return CreatDateTime.toString("dd/MM/yyyy hh:mm:ss");
        else return ModifDateTime.toString("dd/MM/yyyy hh:mm:ss");
}

//====================================================================================================================

QString cBaseMediaFile::GetImageSizeStr(ImageSizeFmt Fmt) {
    QString SizeInfo="";
    QString FmtInfo ="";
    QString GeoInfo ="";

    if ((ImageWidth>0)&&(ImageHeight>0)) {
        // Compute MPix
        double MPix=double(double(ImageWidth)*double(ImageHeight))/double(1000000);
        SizeInfo=QString("%1x%2").arg(ImageWidth).arg(ImageHeight);

        // now search if size is referenced in DefImageFormat
        for (int i=0;i<2;i++) for (int j=0;j<3;j++) for (int k=0;k<NBR_SIZEDEF;k++) if ((DefImageFormat[i][j][k].Width==ImageWidth)&&(DefImageFormat[i][j][k].Height==ImageHeight)) {
            FmtInfo=QString(DefImageFormat[i][j][k].Name).left(QString(DefImageFormat[i][j][k].Name).indexOf(" -"));
            break;
        }
        if ((FmtInfo=="")&&(MPix>=1)) FmtInfo=QString("%1").arg(MPix,8,'f',1).trimmed()+QApplication::translate("cBaseMediaFile","MPix");
        else switch (ImageHeight) {
            case 240:   FmtInfo="QVGA";     break;
            case 320:   FmtInfo="HVGA";     break;
            case 480:   FmtInfo="WVGA";     break;
            case 576:   FmtInfo="DVD";      break;
            case 600:   FmtInfo="SVGA";     break;
            case 720:   FmtInfo="720p";     break;
            case 768:   FmtInfo="XGA";      break;
            case 1080:  FmtInfo="1080p";    break;
            default:    FmtInfo="ns";       break;
        }
    }
    GeoInfo=GetImageGeometryStr();
    switch (Fmt) {
        case FULLWEB  : return SizeInfo+((FmtInfo+GeoInfo)!=""?"("+FmtInfo+(FmtInfo!=""?"-":"")+GeoInfo+")":"");
        case SIZEONLY : return SizeInfo;
        case FMTONLY  : return FmtInfo;
        case GEOONLY  : return GeoInfo;
        default       : return "";
    }
}

//====================================================================================================================
// return 3 lines to display Summary of media file in dialog box which need them

QStringList cBaseMediaFile::GetSummaryText(QStringList *ExtendedProperties) {
    QStringList SummaryText;
    SummaryText.append(ShortName()+"("+GetFileSizeStr()+")");
    SummaryText.append(GetImageSizeStr(cBaseMediaFile::FULLWEB));
    if (ObjectType==OBJECTTYPE_IMAGEFILE) {
        SummaryText.append(GetInformationValue("composer",ExtendedProperties));
        if (GetInformationValue("Photo.ExposureTime",   ExtendedProperties)!="") SummaryText[2]=SummaryText[2]+(SummaryText[2]!=""?"-":"")+GetInformationValue("Photo.ExposureTime",   ExtendedProperties);
        if (GetInformationValue("Photo.ApertureValue",  ExtendedProperties)!="") SummaryText[2]=SummaryText[2]+(SummaryText[2]!=""?"-":"")+GetInformationValue("Photo.ApertureValue",  ExtendedProperties);
        if (GetInformationValue("Photo.ISOSpeedRatings",ExtendedProperties)!="") SummaryText[2]=SummaryText[2]+(SummaryText[2]!=""?"-":"")+GetInformationValue("Photo.ISOSpeedRatings",ExtendedProperties)+" ISO";
    } else SummaryText.append(QApplication::translate("DlgSlideProperties","Duration:")+GetRealDuration().toString("HH:mm:ss.zzz"));
    return SummaryText;
}

//*********************************************************************************************************************************************
// Unmanaged File
//*********************************************************************************************************************************************

cUnmanagedFile::cUnmanagedFile(cApplicationConfig *ApplicationConfig):cBaseMediaFile(ApplicationConfig) {
    ObjectType  =OBJECTTYPE_UNMANAGED;
    IsInformationValide=true;
}

//====================================================================================================================

QString cUnmanagedFile::GetFileTypeStr() {
    return QApplication::translate("cBaseMediaFile","Unmanaged","File type");
}

//*********************************************************************************************************************************************
// Folder
//*********************************************************************************************************************************************

cFolder::cFolder(cApplicationConfig *ApplicationConfig):cBaseMediaFile(ApplicationConfig) {
    ObjectType  =OBJECTTYPE_FOLDER;
}

//====================================================================================================================

bool cFolder::GetChildFullInformationFromFile(bool,cCustomIcon *Icon,QStringList *) {
    QString AdjustedFileName=FileName();
    if (!AdjustedFileName.endsWith(QDir::separator())) AdjustedFileName=AdjustedFileName+QDir::separator();

    // Check if a folder.jpg file exist
    if ((Icon->Icon16.isNull())||(Icon->Icon100.isNull())) {
        QFileInfoList Directorys=QDir(FileName()).entryInfoList(QDir::Files);
        for (int j=0;j<Directorys.count();j++) if (Directorys[j].fileName().toLower()=="folder.jpg") {
            QString FileName=AdjustedFileName+Directorys[j].fileName();
            QImage Final(":img/FolderMask_200.png");
            QImage Img(FileName);
            QImage ImgF;
            if (double(Img.height())/double(Img.width())*double(Img.width())<=162) ImgF=Img.scaledToWidth(180,Qt::SmoothTransformation);
                else ImgF=Img.scaledToHeight(162,Qt::SmoothTransformation);
            QPainter Painter;
            Painter.begin(&Final);
            Painter.drawImage(QRect((Final.width()-ImgF.width())/2,195-ImgF.height(),ImgF.width(),ImgF.height()),ImgF);
            Painter.end();
            Icon->LoadIcons(&Final);
        }
    }

    // Check if there is an desktop.ini ==========> WINDOWS EXTENSION
    if ((Icon->Icon16.isNull())||(Icon->Icon100.isNull())) {
        QFileInfoList Directorys=QDir(FileName()).entryInfoList(QDir::Files|QDir::Hidden);
        for (int j=0;j<Directorys.count();j++) if (Directorys[j].fileName().toLower()=="desktop.ini") {
            QFile   FileIO(AdjustedFileName+Directorys[j].fileName());
            QString IconFile ="";
            if (FileIO.open(QIODevice::ReadOnly/*|QIODevice::Text*/)) {
                // Sometimes this kind of files have incorrect line terminator : nor \r\n nor \n
                QTextStream FileST(&FileIO);
                QString     AllInfo=FileST.readAll();
                QString     Line="";
                while (AllInfo!="") {
                    int j=0;
                    while ((j<AllInfo.length())&&((AllInfo[j]>=char(32))||(AllInfo[j]==9))) j++;
                    if (j<AllInfo.length()) {
                        Line=AllInfo.left(j);
                        while ((j<AllInfo.length())&&(AllInfo[j]<=char(32))) j++;
                        if (j<AllInfo.length()) AllInfo=AllInfo.mid(j); else AllInfo="";
                    } else {
                        Line=AllInfo;
                        AllInfo="";
                    }
                    if ((Line.toUpper().startsWith("ICONFILE"))&&(Line.indexOf("=")!=-1)) {
                        Line=Line.mid(Line.indexOf("=")+1).trimmed();
                        // Replace all variables like %systemroot%
                        while (Line.indexOf("%")!=-1) {
                            QString Var=Line.mid(Line.indexOf("%")+1);  Var=Var.left(Var.indexOf("%"));
                            QString Value=getenv(Var.toLocal8Bit());
                            Line.replace("%"+Var+"%",Value,Qt::CaseInsensitive);
                        }
                        if (QFileInfo(Line).isRelative()) IconFile=QDir::toNativeSeparators(AdjustedFileName+Line);
                            else IconFile=QDir::toNativeSeparators(QFileInfo(Line).absoluteFilePath());
                    }
                }
                FileIO.close();
            }
            if (IconFile.toLower().endsWith(".jpg") || IconFile.toLower().endsWith(".png") || IconFile.toLower().endsWith(".ico")) Icon->LoadIcons(IconFile);
        }
    }

    // if no icon then load default for type
    if ((Icon->Icon16.isNull())||(Icon->Icon100.isNull())) Icon->LoadIcons(&ApplicationConfig->DefaultFOLDERIcon);
    return true;
}

//====================================================================================================================

QString cFolder::GetFileTypeStr() {
    return QApplication::translate("cBaseMediaFile","Folder","File type");
}

//*********************************************************************************************************************************************
// ffDiaporama project file
//*********************************************************************************************************************************************

cffDProjectFile::cffDProjectFile(cApplicationConfig *ApplicationConfig):cBaseMediaFile(ApplicationConfig) {
    ObjectType      =OBJECTTYPE_FFDFILE;
    NbrSlide        =0;
    NbrChapters     =0;

    InitDefaultValues();
}

//====================================================================================================================

void cffDProjectFile::InitDefaultValues() {
    Title           =QApplication::translate("cModelList","Project title");;
    Author          =QApplication::translate("cModelList","Project author");
    Album           =QApplication::translate("cModelList","Project album");
    OverrideDate    =false;
    EventDate       =QDate::currentDate();
    LongDate        =(OverrideDate?LongDate:FormatLongDate(EventDate));
    Comment         =QApplication::translate("cModelList","Project comment");
    Composer        ="";
    DefaultLanguage =ApplicationConfig->DefaultLanguage;
    ffDRevision     =CurrentAppVersion;
}

//====================================================================================================================

bool cffDProjectFile::LoadBasicInformationFromDatabase(QDomElement *ParentElement) {
    return LoadFromXML(ParentElement);
}

//====================================================================================================================

void cffDProjectFile::SaveBasicInformationToDatabase(QDomElement *ParentElement) {
    SaveToXML(ParentElement);

    QDomDocument    DomDocument;
    QDomElement     Element=DomDocument.createElement("Project");
    Element.setAttribute("ImageGeometry",ObjectGeometry==IMAGE_GEOMETRY_16_9?GEOMETRY_16_9:ObjectGeometry==IMAGE_GEOMETRY_40_17?GEOMETRY_40_17:GEOMETRY_4_3);
    Element.setAttribute("ObjectNumber",NbrSlide);
    ParentElement->appendChild(Element);
}

//====================================================================================================================

void cffDProjectFile::SaveToXML(QDomElement *ParentElement) {
    QDomDocument    DomDocument;
    QDomElement     Element=DomDocument.createElement("ffDiaporamaProjectProperties");
    Element.setAttribute("Title",Title);
    Element.setAttribute("Author",Author);
    Element.setAttribute("Album",Album);
    Element.setAttribute("LongDate",LongDate);
    Element.setAttribute("EventDate",EventDate.toString(Qt::ISODate));
    Element.setAttribute("OverrideDate",OverrideDate?1:0);
    Element.setAttribute("Comment",Comment);
    Element.setAttribute("Composer",Composer);
    Element.setAttribute("Duration",qlonglong(QTime(0,0,0,0).msecsTo(GetRealDuration())));
    Element.setAttribute("ffDRevision",ffDRevision);
    Element.setAttribute("DefaultLanguage",DefaultLanguage);
    Element.setAttribute("ChaptersNumber",NbrChapters);
    for (int i=0;i<NbrChapters;i++) {
        QString     ChapterNum=QString("%1").arg(i); while (ChapterNum.length()<3) ChapterNum="0"+ChapterNum;
        QDomElement SubElement=DomDocument.createElement("Chapter_"+ChapterNum);
        SubElement.setAttribute("Start",    GetInformationValue("Chapter_"+ChapterNum+":Start",&ChaptersProperties));
        SubElement.setAttribute("End",      GetInformationValue("Chapter_"+ChapterNum+":End",&ChaptersProperties));
        SubElement.setAttribute("Duration", GetInformationValue("Chapter_"+ChapterNum+":Duration",&ChaptersProperties));
        SubElement.setAttribute("title",    GetInformationValue("Chapter_"+ChapterNum+":title",&ChaptersProperties));
        SubElement.setAttribute("InSlide",  GetInformationValue("Chapter_"+ChapterNum+":InSlide",&ChaptersProperties));
        Element.appendChild(SubElement);
    }
    ParentElement->appendChild(Element);
}

//====================================================================================================================

bool cffDProjectFile::LoadFromXML(QDomElement *ParentElement) {
    InitDefaultValues();
    bool IsOk=false;
    if ((ParentElement->elementsByTagName("ffDiaporamaProjectProperties").length()>0)&&(ParentElement->elementsByTagName("ffDiaporamaProjectProperties").item(0).isElement()==true)) {
        QDomElement Element=ParentElement->elementsByTagName("ffDiaporamaProjectProperties").item(0).toElement();
        if (Element.hasAttribute("Title"))              Title=Element.attribute("Title");
        if (Element.hasAttribute("Author"))             Author=Element.attribute("Author");
        if (Element.hasAttribute("Album"))              Album=Element.attribute("Album");
        if (Element.hasAttribute("EventDate"))          EventDate=EventDate.fromString(Element.attribute("EventDate"),Qt::ISODate);
            else if (Element.hasAttribute("Year"))      EventDate.setDate(Element.attribute("Year").toInt(),1,1);
        if (Element.hasAttribute("OverrideDate"))       OverrideDate=Element.attribute("OverrideDate")=="1";
        if (!OverrideDate)                              LongDate=FormatLongDate(EventDate);
            else if (Element.hasAttribute("LongDate"))  LongDate=Element.attribute("LongDate");
        if (Element.hasAttribute("Comment"))            Comment=Element.attribute("Comment");
        if (Element.hasAttribute("ffDRevision"))        ffDRevision=Element.attribute("ffDRevision");
        if (Element.hasAttribute("Composer"))           Composer=Element.attribute("Composer");
        if (Element.hasAttribute("DefaultLanguage"))    DefaultLanguage=Element.attribute("DefaultLanguage");
        if (Element.hasAttribute("Duration"))           SetRealDuration(QTime(0,0,0,0).addMSecs(Element.attribute("Duration").toLongLong()));

        if (Element.hasAttribute("ChaptersNumber")) {
            NbrChapters=Element.attribute("ChaptersNumber").toInt();
            for (int i=0;i<NbrChapters;i++) {
                QString     ChapterNum=QString("%1").arg(i); while (ChapterNum.length()<3) ChapterNum="0"+ChapterNum;
                if ((ParentElement->elementsByTagName("Chapter_"+ChapterNum).length()>0)&&(ParentElement->elementsByTagName("Chapter_"+ChapterNum).item(0).isElement()==true)) {
                    QDomElement SubElement=ParentElement->elementsByTagName("Chapter_"+ChapterNum).item(0).toElement();
                    QString     Start="";
                    QString     End="";
                    QString     Duration="";
                    QString     Title="";
                    QString     InSlide="";
                    if (SubElement.hasAttribute("Start"))       Start=SubElement.attribute("Start");
                    if (SubElement.hasAttribute("End"))         End=SubElement.attribute("End");
                    if (SubElement.hasAttribute("Duration"))    Duration=SubElement.attribute("Duration");
                    if (SubElement.hasAttribute("title"))       Title=SubElement.attribute("title");
                    if (SubElement.hasAttribute("InSlide"))     InSlide=SubElement.attribute("InSlide");

                    ChaptersProperties.append("Chapter_"+ChapterNum+":Start"   +QString("##")+Start);
                    ChaptersProperties.append("Chapter_"+ChapterNum+":End"     +QString("##")+End);
                    ChaptersProperties.append("Chapter_"+ChapterNum+":Duration"+QString("##")+Duration);
                    ChaptersProperties.append("Chapter_"+ChapterNum+":title"   +QString("##")+Title);
                    ChaptersProperties.append("Chapter_"+ChapterNum+":InSlide" +QString("##")+InSlide);
                }
            }
        }
        IsOk=true;
    }
    if ((ParentElement->elementsByTagName("Project").length()>0)&&(ParentElement->elementsByTagName("Project").item(0).isElement()==true)) {
        QDomElement Element=ParentElement->elementsByTagName("Project").item(0).toElement();
        if (Element.hasAttribute("ImageGeometry")) {
            switch (Element.attribute("ImageGeometry").toInt()) {
                case GEOMETRY_16_9:  ObjectGeometry=IMAGE_GEOMETRY_16_9;   break;
                case GEOMETRY_40_17: ObjectGeometry=IMAGE_GEOMETRY_40_17;  break;
                case GEOMETRY_4_3:
                default:             ObjectGeometry=IMAGE_GEOMETRY_4_3;    break;
            }
        }
        if (Element.hasAttribute("ObjectNumber"))
            NbrSlide=Element.attribute("ObjectNumber").toInt();
    }
    return IsOk;
}

//====================================================================================================================

bool cffDProjectFile::GetChildFullInformationFromFile(cCustomIcon *Icon,QStringList *) {
    Icon->LoadIcons(&ApplicationConfig->DefaultFFDIcon);

    QFile           file(FileName());
    QDomDocument    domDocument;
    QDomElement     root;
    QString         errorStr;
    int             errorLine,errorColumn;
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream InStream(&file);
        QString     ffDPart;
        bool        EndffDPart=false;
        InStream.setCodec("UTF-8");
        while (!InStream.atEnd()) {
            QString Line=InStream.readLine();
            if (!EndffDPart) {
                ffDPart.append(Line);
                if (Line=="</Project>") EndffDPart=true;
            }
        }
        file.close();
        // Now import ffDPart
        if (domDocument.setContent(ffDPart,true,&errorStr,&errorLine,&errorColumn)) {
            root = domDocument.documentElement();
            // Load project properties
            if (root.tagName()==FFD_APPLICATION_ROOTNAME) LoadFromXML(&root);
        }
        file.close();
    }
    return true;
}

//====================================================================================================================

QString cffDProjectFile::GetTechInfo(QStringList *) {
    QString Info="";
    if (Composer!="")   Info=Info+(Info!=""?" - ":"")+Composer+" ("+ffDRevision+")";
    if (GetImageSizeStr(cBaseMediaFile::GEOONLY)!="")   Info=Info+(Info!=""?" - ":"")+GetImageSizeStr(cBaseMediaFile::GEOONLY);
    if (NbrSlide>0)                                     Info=Info+(Info!=""?" - ":"")+QString("%1").arg(NbrSlide)   +" "+QApplication::translate("cBaseMediaFile","Slides");
    if (NbrChapters>0)                                  Info=Info+(Info!=""?" - ":"")+QString("%1").arg(NbrChapters)+" "+QApplication::translate("cBaseMediaFile","Chapters");
    return Info;
}

//====================================================================================================================

QString cffDProjectFile::GetTAGInfo(QStringList *) {
    QString Info=Title;
    if (LongDate!="")       Info=Info+(Info!=""?" - ":"")+LongDate; else Info=Info+(Info!=""?" - ":"")+EventDate.toString(ApplicationConfig->ShortDateFormat);
    if (Album!="")          Info=Info+(Info!=""?" - ":"")+Album;
    if (Author!="")         Info=Info+(Info!=""?" - ":"")+Author;
    return Info;
}

//====================================================================================================================

QString cffDProjectFile::GetFileTypeStr() {
    return QApplication::translate("cBaseMediaFile","ffDiaporama","File type");
}

//*********************************************************************************************************************************************
// Image file
//*********************************************************************************************************************************************

cImageFile::cImageFile(cApplicationConfig *ApplicationConfig):cBaseMediaFile(ApplicationConfig) {
    ObjectType  =OBJECTTYPE_IMAGEFILE;  // coul be turn later to OBJECTTYPE_THUMBNAIL
    NoExifData  =false;
}

//====================================================================================================================

cImageFile::~cImageFile() {
}

//====================================================================================================================

QString cImageFile::GetFileTypeStr() {
    if (ObjectType==OBJECTTYPE_IMAGEFILE)               return QApplication::translate("cBaseMediaFile","Image","File type");
        else if (ObjectType==OBJECTTYPE_IMAGEVECTOR)    return QApplication::translate("cBaseMediaFile","Vector image","File type");
        else                                            return QApplication::translate("cBaseMediaFile","Thumbnail","File type");
}

//====================================================================================================================

bool cImageFile::LoadBasicInformationFromDatabase(QDomElement *ParentElement,QString,QString,QStringList *,bool *,QList<cSlideThumbsTable::TRResKeyItem> *,bool) {
    ImageWidth      =ParentElement->attribute("ImageWidth").toInt();
    ImageHeight     =ParentElement->attribute("ImageHeight").toInt();
    ImageOrientation=ParentElement->attribute("ImageOrientation").toInt();
    ObjectGeometry  =ParentElement->attribute("ObjectGeometry").toInt();
    AspectRatio     =GetDoubleValue(*ParentElement,"AspectRatio");
    return true;
}

//====================================================================================================================

void cImageFile::SaveBasicInformationToDatabase(QDomElement *ParentElement,QString,QString,bool,cReplaceObjectList *,QList<qlonglong> *,bool) {
    ParentElement->setAttribute("ImageWidth",         ImageWidth);
    ParentElement->setAttribute("ImageHeight",        ImageHeight);
    ParentElement->setAttribute("ImageOrientation",   ImageOrientation);
    ParentElement->setAttribute("ObjectGeometry",     ObjectGeometry);
    ParentElement->setAttribute("AspectRatio",        QString("%1").arg(AspectRatio,0,'f'));
}

//====================================================================================================================

bool cImageFile::CheckFormatValide(QWidget *Window) {
    bool IsOk=GetFullInformationFromFile();

    // Try to load an image to ensure all is ok
    if (IsOk) {
        QImage *Image=ImageAt(true);
        if (Image) {
            delete Image;
        } else {
            QString ErrorMessage=QApplication::translate("MainWindow","Impossible to read an image from the file","Error message");
            CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
            IsOk=false;
        }
    }

    return IsOk;
}

//====================================================================================================================

bool cImageFile::GetInformationFromFile(QString FileName,QStringList *AliasList,bool *ModifyFlag,qlonglong GivenFolderKey) {
    if (QFileInfo(FileName).suffix().toLower()=="svg") ObjectType=OBJECTTYPE_IMAGEVECTOR;
    return cBaseMediaFile::GetInformationFromFile(FileName,AliasList,ModifyFlag,GivenFolderKey);
}

//====================================================================================================================

bool cImageFile::GetChildFullInformationFromFile(bool,cCustomIcon *Icon,QStringList *ExtendedProperties) {
    ImageOrientation    =-1;
    bool                ExifOk=false;

    if (ObjectType==OBJECTTYPE_IMAGEVECTOR) {
            // Vector image file
            QSvgRenderer SVGImg(FileName());
            if (SVGImg.isValid()) {
                ImageOrientation=0;
                ImageWidth      =SVGImg.viewBox().width();
                ImageHeight     =SVGImg.viewBox().height();

                QPainter Painter;
                QImage   Img;
                qreal    RatioX=(ImageWidth>ImageHeight?1:qreal(ImageWidth)/qreal(ImageHeight));
                qreal    RatioY=(ImageWidth<ImageHeight?1:qreal(ImageHeight)/qreal(ImageWidth));

                // 16x16 icon
                Img=QImage(qreal(16)*RatioX,qreal(16)*RatioY,QImage::Format_ARGB32);
                Painter.begin(&Img);
                Painter.setCompositionMode(QPainter::CompositionMode_Source);
                Painter.fillRect(QRect(0,0,Img.width(),Img.height()),Qt::transparent);
                Painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                SVGImg.render(&Painter);
                Painter.end();
                Icon->Icon16=QImage(16,16,QImage::Format_ARGB32_Premultiplied);
                Painter.begin(&Icon->Icon16);
                Painter.setCompositionMode(QPainter::CompositionMode_Source);
                Painter.fillRect(QRect(0,0,Icon->Icon16.width(),Icon->Icon16.height()),Qt::transparent);
                Painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                Painter.drawImage(QPoint((16-Img.width())/2,(16-Img.height())/2),Img);
                Painter.end();

                // 100x100 icon
                Img=QImage(qreal(100)*RatioX,qreal(100)*RatioY,QImage::Format_ARGB32);
                Painter.begin(&Img);
                Painter.setCompositionMode(QPainter::CompositionMode_Source);
                Painter.fillRect(QRect(0,0,Img.width(),Img.height()),Qt::transparent);
                Painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                SVGImg.render(&Painter);
                Painter.end();
                Icon->Icon100=QImage(100,100,QImage::Format_ARGB32_Premultiplied);
                Painter.begin(&Icon->Icon100);
                Painter.setCompositionMode(QPainter::CompositionMode_Source);
                Painter.fillRect(QRect(0,0,Icon->Icon100.width(),Icon->Icon100.height()),Qt::transparent);
                Painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                Painter.drawImage(QPoint((100-Img.width())/2,(100-Img.height())/2),Img);
                Painter.end();

                ExtendedProperties->append(QString("Photo.PixelXDimension")+QString("##")+QString("%1").arg(ImageWidth));
                ExtendedProperties->append(QString("Photo.PixelYDimension")+QString("##")+QString("%1").arg(ImageHeight));
            }

    } else if (NoExifData) {

    } else if (!NoExifData) {

        // ******************************************************************************************************
        // Try to load EXIF information using library exiv2
        // ******************************************************************************************************
        std::unique_ptr<Exiv2::Image> ImageFile;
        try {
            ImageFile=Exiv2::ImageFactory::open(FileName().toUtf8().data());
            ExifOk=true;
        }
        catch( Exiv2::Error& ) {
            //ToLog(LOGMSG_INFORMATION,QApplication::translate("cBaseMediaFile","Image don't have EXIF metadata %1").arg(FileName));
            NoExifData=true;
        }
        if ((ExifOk)&&(ImageFile->good())) {
            ImageFile->readMetadata();
            // Read data
            Exiv2::ExifData &exifData = ImageFile->exifData();
            if (!exifData.empty()) {
                Exiv2::ExifData::const_iterator end = exifData.end();
                for (Exiv2::ExifData::const_iterator CurrentData=exifData.begin();CurrentData!=end;++CurrentData) {

                    if ((QString().fromStdString(CurrentData->key())=="Exif.Image.Orientation")&&(CurrentData->tag()==274))
                        ImageOrientation=QString().fromStdString(CurrentData->value().toString()).toInt();

                    if ((CurrentData->typeId()!=Exiv2::undefined)&&
                        (!(((CurrentData->typeId()==Exiv2::unsignedByte)||(CurrentData->typeId()==Exiv2::signedByte))&&(CurrentData->size()>64)))) {
                        QString Key  =QString().fromStdString(CurrentData->key());
                        QString Value=QString().fromUtf8(CurrentData->print(&exifData).c_str());
                        if (Key.startsWith("Exif.")) Key=Key.mid(QString("Exif.").length());
                        ExtendedProperties->append(Key+QString("##")+Value);
                    }
                }
            }

            // Append ExtendedProperties
            if (GetInformationValue("Image.Artist",ExtendedProperties)!="") ExtendedProperties->append(QString("artist")+QString("##")+GetInformationValue("Image.Artist",ExtendedProperties));
            if (GetInformationValue("Image.Model",ExtendedProperties)!="")  {
                if (GetInformationValue("Image.Model",ExtendedProperties).contains(GetInformationValue("Image.Make",ExtendedProperties),Qt::CaseInsensitive)) ExtendedProperties->append(QString("composer")+QString("##")+GetInformationValue("Image.Model",ExtendedProperties));
                    else ExtendedProperties->append(QString("composer")+QString("##")+GetInformationValue("Image.Make",ExtendedProperties)+" "+GetInformationValue("Image.Model",ExtendedProperties));
            }
            // Get size information
            ImageWidth =ImageFile->pixelWidth();
            ImageHeight=ImageFile->pixelHeight();

            // switch ImageWidth and ImageHeight if image was rotated
            if ((ImageOrientation==6)||(ImageOrientation==8)) {
                int IW=ImageWidth;
                ImageWidth=ImageHeight;
                ImageHeight=IW;
            }

            // Read preview image
            if ((Exiv2WithPreview)&&(Icon->Icon16.isNull() || Icon->Icon100.isNull())) {
                Exiv2::PreviewManager *Manager=new Exiv2::PreviewManager(*ImageFile);
                if (Manager) {
                    Exiv2::PreviewPropertiesList Properties=Manager->getPreviewProperties();
                    if (!Properties.empty()) {
                        Exiv2::PreviewImage Image=Manager->getPreviewImage(Properties[Properties.size()-1]);      // Get the latest image (biggest)
                        QImage *IconImage=new QImage();
                        if (IconImage->loadFromData(QByteArray((const char*)Image.pData(),Image.size()))) {
                            if (ImageOrientation==8) {          // Rotating image anti-clockwise by 90 degrees...'
                                QTransform matrix;
                                matrix.rotate(-90);
                                QImage *NewImage=new QImage(IconImage->transformed(matrix,Qt::SmoothTransformation));
                                delete IconImage;
                                IconImage=NewImage;
                            } else if (ImageOrientation==3) {   // Rotating image clockwise by 180 degrees...'
                                QTransform matrix;
                                matrix.rotate(180);
                                QImage *NewImage=new QImage(IconImage->transformed(matrix,Qt::SmoothTransformation));
                                delete IconImage;
                                IconImage=NewImage;
                            } else if (ImageOrientation==6) {   // Rotating image clockwise by 90 degrees...'
                                QTransform matrix;
                                matrix.rotate(90);
                                QImage *NewImage=new QImage(IconImage->transformed(matrix,Qt::SmoothTransformation));
                                delete IconImage;
                                IconImage=NewImage;
                            }

                            // Sometimes, Icon have black bar : try to remove them
                            if ((double(IconImage->width())/double(IconImage->height()))!=(double(ImageWidth)/double(ImageHeight))) {
                                if (ImageWidth>ImageHeight) {
                                    int RealHeight=int((double(IconImage->width())*double(ImageHeight))/double(ImageWidth));
                                    int Delta     =IconImage->height()-RealHeight;
                                    QImage *NewImage=new QImage(IconImage->copy(0,Delta/2,IconImage->width(),IconImage->height()-Delta));
                                    delete IconImage;
                                    IconImage=NewImage;
                                    // if preview Icon have a really small size, then don't use it
                                    if (IconImage->width()>=MinimumEXIFHeight) Icon->LoadIcons(IconImage);
                                } else {
                                    int RealWidth=int((double(IconImage->height())*double(ImageWidth))/double(ImageHeight));
                                    int Delta     =IconImage->width()-RealWidth;
                                    QImage *NewImage=new QImage(IconImage->copy(Delta/2,0,IconImage->width()-Delta,IconImage->height()));
                                    delete IconImage;
                                    IconImage=NewImage;
                                    // if preview Icon have a really small size, then don't use it
                                    if (IconImage->height()>=MinimumEXIFHeight) Icon->LoadIcons(IconImage);
                                }
                            }

                        }
                        delete IconImage;
                    }
                    delete Manager;
                }
            }
        }

        //************************************************************************************
        // If no exif preview image (of image too small) then load/create thumbnail
        //************************************************************************************
        if (Icon->Icon16.isNull() || Icon->Icon100.isNull()) {
            cLuLoImageCacheObject *ImageObject=ApplicationConfig->ImagesCache.FindObject(RessourceKey,FileKey,ModifDateTime,ImageOrientation,ApplicationConfig->Smoothing,true);
            if (ImageObject==NULL) {
                ToLog(LOGMSG_CRITICAL,"Error in cImageFile::GetFullInformationFromFile : FindObject return NULL for thumbnail creation !");
            } else {
                QImageReader ImgReader(FileName());
                if (ImgReader.canRead()) {
                    QSize Size=ImgReader.size();
                    if ((Size.width()>=100)||(Size.height()>=100)) {
                        if ((qreal(Size.height())/qreal(Size.width()))*100<=100) {
                            Size.setHeight((qreal(Size.height())/qreal(Size.width()))*100);
                            Size.setWidth(100);
                        } else {
                            Size.setWidth((qreal(Size.width())/qreal(Size.height()))*100);
                            Size.setHeight(100);
                        }
                        ImgReader.setScaledSize(Size);
                    }
                    QImage Image=ImgReader.read();
                    if (Image.isNull()) ToLog(LOGMSG_CRITICAL,"QImageReader.read return error in GetFullInformationFromFile");
                        else Icon->LoadIcons(&Image);
                }
            }
        }

        //************************************************************************************
        // if no information about size then load image
        //************************************************************************************
        if ((ImageWidth==0)||(ImageHeight==0)) {
            cLuLoImageCacheObject *ImageObject=ApplicationConfig->ImagesCache.FindObject(RessourceKey,FileKey,ModifDateTime,ImageOrientation,ApplicationConfig->Smoothing,true);
            if (ImageObject==NULL) {
                ToLog(LOGMSG_CRITICAL,"Error in cImageFile::GetFullInformationFromFile : FindObject return NULL for size computation !");
            } else {
                QImageReader Img(FileName());
                if (Img.canRead()) {
                    QSize Size =Img.size();
                    ImageWidth =Size.width();
                    ImageHeight=Size.height();
                    ExtendedProperties->append(QString("Photo.PixelXDimension")+QString("##")+QString("%1").arg(ImageWidth));
                    ExtendedProperties->append(QString("Photo.PixelYDimension")+QString("##")+QString("%1").arg(ImageHeight));
                }
            }
        }

    }

    //************************************************************************************
    // End process by computing some values ....
    //************************************************************************************

    // Sort ExtendedProperties
    ExtendedProperties->sort();

    // Now we have image size then compute image geometry
    ObjectGeometry=IMAGE_GEOMETRY_UNKNOWN;
    double RatioHW=double(ImageWidth)/double(ImageHeight);
    if ((RatioHW>=1.45)&&(RatioHW<=1.55))           ObjectGeometry=IMAGE_GEOMETRY_3_2;
    else if ((RatioHW>=0.65)&&(RatioHW<=0.67))      ObjectGeometry=IMAGE_GEOMETRY_2_3;
    else if ((RatioHW>=1.32)&&(RatioHW<=1.34))      ObjectGeometry=IMAGE_GEOMETRY_4_3;
    else if ((RatioHW>=0.74)&&(RatioHW<=0.76))      ObjectGeometry=IMAGE_GEOMETRY_3_4;
    else if ((RatioHW>=1.77)&&(RatioHW<=1.79))      ObjectGeometry=IMAGE_GEOMETRY_16_9;
    else if ((RatioHW>=0.56)&&(RatioHW<=0.58))      ObjectGeometry=IMAGE_GEOMETRY_9_16;
    else if ((RatioHW>=2.34)&&(RatioHW<=2.36))      ObjectGeometry=IMAGE_GEOMETRY_40_17;
    else if ((RatioHW>=0.42)&&(RatioHW<=0.44))      ObjectGeometry=IMAGE_GEOMETRY_17_40;

    // if Icon16 stil null then load default icon
    if (Icon->Icon16.isNull() || Icon->Icon100.isNull()) Icon->LoadIcons(&ApplicationConfig->DefaultIMAGEIcon);
    return true;
}

//====================================================================================================================

QString cImageFile::GetTechInfo(QStringList *ExtendedProperties) {
    QString Info=GetImageSizeStr(FULLWEB);
    if (GetInformationValue("artist",ExtendedProperties)!="")              Info=Info+(Info!=""?"-":"")+GetInformationValue("artist",ExtendedProperties);
    if (GetInformationValue("composer",ExtendedProperties)!="")            Info=Info+(Info!=""?"-":"")+GetInformationValue("composer",ExtendedProperties);
    if (GetInformationValue("Image.Orientation",ExtendedProperties)!="")   Info=Info+(Info!=""?"-":"")+GetInformationValue("Image.Orientation",ExtendedProperties);
    return Info;
}

//====================================================================================================================

QString cImageFile::GetTAGInfo(QStringList *ExtendedProperties) {
    QString Info=GetInformationValue("Photo.ExposureTime",ExtendedProperties);
    if (GetInformationValue("Photo.ApertureValue",ExtendedProperties)!="")    Info=Info+(Info!=""?"-":"")+GetInformationValue("Photo.ApertureValue",ExtendedProperties);
    if (GetInformationValue("Photo.ISOSpeedRatings",ExtendedProperties)!="")  Info=Info+(Info!=""?"-":"")+GetInformationValue("Photo.ISOSpeedRatings",ExtendedProperties)+" ISO";
    if (GetInformationValue("CanonCs.LensType",ExtendedProperties)!="")       Info=Info+(Info!=""?"-":"")+GetInformationValue("CanonCs.LensType",ExtendedProperties);                // Canon version
    if (GetInformationValue("NikonLd3.LensIDNumber",ExtendedProperties)!="")  Info=Info+(Info!=""?"-":"")+GetInformationValue("NikonLd3.LensIDNumber",ExtendedProperties);           // Nikon version
    if (GetInformationValue("Photo.Flash",ExtendedProperties)!="")            Info=Info+(Info!=""?"-":"")+GetInformationValue("Photo.Flash",ExtendedProperties);
    if (GetInformationValue("CanonCs.FlashMode",ExtendedProperties)!="")      Info=Info+(Info!=""?"-":"")+GetInformationValue("CanonCs.FlashMode",ExtendedProperties);               // Canon version
    if (GetInformationValue("Nikon3.FlashMode",ExtendedProperties)!="")       Info=Info+(Info!=""?"-":"")+GetInformationValue("Nikon3.FlashMode",ExtendedProperties);                // Nikon version
    return Info;
}

//====================================================================================================================

QImage *cImageFile::ImageAt(bool PreviewMode) {
    if (!IsValide)            return NULL;
    //if (!IsInformationValide) GetFullInformationFromFile();

    QImage *RetImage=NULL;
    if (ObjectType==OBJECTTYPE_IMAGEVECTOR) {
        // Vector image file
        QSvgRenderer SVGImg(FileName());
        if (SVGImg.isValid()) {
            if ((ImageWidth==0)||(ImageHeight==0)) {
                ImageWidth =SVGImg.defaultSize().width();
                ImageHeight=SVGImg.defaultSize().height();
            }
            RetImage=new QImage(ImageWidth,ImageHeight,QImage::Format_ARGB32_Premultiplied);
            QPainter Painter;
            Painter.begin(RetImage);
            Painter.setCompositionMode(QPainter::CompositionMode_Source);
            Painter.fillRect(QRect(0,0,RetImage->width(),RetImage->height()),Qt::transparent);
            Painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            Painter.setClipping(true);
            Painter.setClipRect(QRect(0,0,RetImage->width(),RetImage->height()));
            SVGImg.render(&Painter,QRectF(0,0,RetImage->width(),RetImage->height()));
            Painter.end();
        }
    } else {
        cLuLoImageCacheObject *ImageObject=ApplicationConfig->ImagesCache.FindObject(RessourceKey,FileKey,ModifDateTime,ImageOrientation,(!PreviewMode || ApplicationConfig->Smoothing),true);

        if (!ImageObject) {
            ToLog(LOGMSG_CRITICAL,"Error in cImageFile::ImageAt : FindObject return NULL !");
            return NULL;  // There is an error !!!!!
        }

        if (PreviewMode) RetImage=ImageObject->ValidateCachePreviewImage();
            else         RetImage=ImageObject->ValidateCacheRenderImage();

        if (RetImage==NULL) ToLog(LOGMSG_CRITICAL,"Error in cImageFile::ImageAt : ValidateCacheImage return NULL !");
    }
    // return wanted image
    return RetImage;
}

//*********************************************************************************************************************************************
// Image from clipboard
//*********************************************************************************************************************************************

cImageClipboard::cImageClipboard(cApplicationConfig *ApplicationConfig):cImageFile(ApplicationConfig) {
    ObjectType      =OBJECTTYPE_IMAGECLIPBOARD;
    ObjectName      ="ImageClipboard";
    NoExifData      =true;
    ImageOrientation=0;
}

//====================================================================================================================

cImageClipboard::~cImageClipboard() {
}

//====================================================================================================================

bool cImageClipboard::LoadBasicInformationFromDatabase(QDomElement *ParentElement,QString,QString,QStringList *,bool *,QList<cSlideThumbsTable::TRResKeyItem> *,bool) {
    ImageWidth      =ParentElement->attribute("ImageWidth").toInt();
    ImageHeight     =ParentElement->attribute("ImageHeight").toInt();
    ImageOrientation=ParentElement->attribute("ImageOrientation").toInt();
    ObjectGeometry  =ParentElement->attribute("ObjectGeometry").toInt();
    AspectRatio     =GetDoubleValue(*ParentElement,"AspectRatio");
    CreatDateTime.fromString(ParentElement->attribute("CreatDateTime"),Qt::ISODate);
    return true;
}

//====================================================================================================================

void cImageClipboard::SaveBasicInformationToDatabase(QDomElement *ParentElement,QString,QString,bool,cReplaceObjectList *,QList<qlonglong> *,bool) {
    ParentElement->setAttribute("ImageWidth",        ImageWidth);
    ParentElement->setAttribute("ImageHeight",       ImageHeight);
    ParentElement->setAttribute("ImageOrientation",  ImageOrientation);
    ParentElement->setAttribute("ObjectGeometry",    ObjectGeometry);
    ParentElement->setAttribute("AspectRatio",       QString("%1").arg(AspectRatio,0,'f'));
    ParentElement->setAttribute("CreatDateTime",     CreatDateTime.toString(Qt::ISODate));
}

//====================================================================================================================

bool cImageClipboard::GetInformationFromFile(QString,QStringList *,bool *,qlonglong) {
    QImage ImageClipboard;
    ApplicationConfig->SlideThumbsTable->GetThumbs(&RessourceKey,&ImageClipboard);
    if (!ImageClipboard.isNull()) {
        ImageWidth=ImageClipboard.width();
        ImageHeight=ImageClipboard.height();
        ImageOrientation=0;
        AspectRatio=double(ImageHeight)/double(ImageWidth);
        // Now we have image size then compute image geometry
        ObjectGeometry=IMAGE_GEOMETRY_UNKNOWN;
        double RatioHW=double(ImageWidth)/double(ImageHeight);
        if ((RatioHW>=1.45)&&(RatioHW<=1.55))           ObjectGeometry=IMAGE_GEOMETRY_3_2;
        else if ((RatioHW>=0.65)&&(RatioHW<=0.67))      ObjectGeometry=IMAGE_GEOMETRY_2_3;
        else if ((RatioHW>=1.32)&&(RatioHW<=1.34))      ObjectGeometry=IMAGE_GEOMETRY_4_3;
        else if ((RatioHW>=0.74)&&(RatioHW<=0.76))      ObjectGeometry=IMAGE_GEOMETRY_3_4;
        else if ((RatioHW>=1.77)&&(RatioHW<=1.79))      ObjectGeometry=IMAGE_GEOMETRY_16_9;
        else if ((RatioHW>=0.56)&&(RatioHW<=0.58))      ObjectGeometry=IMAGE_GEOMETRY_9_16;
        else if ((RatioHW>=2.34)&&(RatioHW<=2.36))      ObjectGeometry=IMAGE_GEOMETRY_40_17;
        else if ((RatioHW>=0.42)&&(RatioHW<=0.44))      ObjectGeometry=IMAGE_GEOMETRY_17_40;
        IsValide=true;
    } else IsValide=false;
    return IsValide;
}

//====================================================================================================================

bool cImageClipboard::GetChildFullInformationFromFile(bool,cCustomIcon *Icon,QStringList *ExtendedProperties) {
    if (Icon) {
        if (Icon->Icon16.isNull() || Icon->Icon100.isNull()) Icon->LoadIcons(&ApplicationConfig->DefaultIMAGEIcon);
    }
    if (ExtendedProperties) {
        ExtendedProperties->append(QString("Photo.PixelXDimension")+QString("##")+QString("%1").arg(ImageWidth));
        ExtendedProperties->append(QString("Photo.PixelYDimension")+QString("##")+QString("%1").arg(ImageHeight));
    }
    return true;
}

//====================================================================================================================

QStringList cImageClipboard::GetSummaryText(QStringList *) {
    QStringList SummaryText;
    SummaryText.append(GetFileTypeStr());
    SummaryText.append(GetImageSizeStr(cBaseMediaFile::FULLWEB));
    SummaryText.append("");
    return SummaryText;
}

//====================================================================================================================

bool cImageClipboard::LoadFromXML(QDomElement *ParentElement,QString ElementName,QString PathForRelativPath,QStringList *AliasList,bool *ModifyFlag,QList<cSlideThumbsTable::TRResKeyItem> *ResKeyList,bool DuplicateRes) {
    if ((DuplicateRes)&&(ObjectType==OBJECTTYPE_IMAGECLIPBOARD)) DuplicateRes=false;    // Never duplicate an image clipboard (but allow it for child)
    if ((ParentElement->elementsByTagName(ObjectName).length()>0)&&(ParentElement->elementsByTagName(ObjectName).item(0).isElement()==true)) {
        QDomElement SubElement=ParentElement->elementsByTagName(ObjectName).item(0).toElement();
        if (LoadBasicInformationFromDatabase(&SubElement,ElementName,PathForRelativPath,AliasList,ModifyFlag,ResKeyList,DuplicateRes)) {
            if (ResKeyList) {
                RessourceKey=SubElement.attribute("RessourceKey").toLongLong();
                for (int ResNum=0;ResNum<ResKeyList->count();ResNum++) if (RessourceKey==ResKeyList->at(ResNum).OrigKey) {
                    RessourceKey=ResKeyList->at(ResNum).NewKey;
                    break;
                }
            } else RessourceKey=SubElement.attribute("RessourceKey").toLongLong();
            // if DuplicateRes (for exemple during a paste operation)
            if ((DuplicateRes)&&(RessourceKey!=-1)) {
                QImage Image;
                ApplicationConfig->SlideThumbsTable->GetThumbs(&RessourceKey,&Image);
                RessourceKey=-1;
                ApplicationConfig->SlideThumbsTable->SetThumbs(&RessourceKey,Image);
            }
            return true;
        } else return false;
    } else return false;
}

//====================================================================================================================

void cImageClipboard::SaveToXML(QDomElement *ParentElement,QString ElementName,QString PathForRelativPath,bool ForceAbsolutPath,cReplaceObjectList *ReplaceList,QList<qlonglong> *ResKeyList,bool IsModel) {
    QDomDocument    DomDocument;
    QDomElement     SubElement=DomDocument.createElement(ObjectName);
    SaveBasicInformationToDatabase(&SubElement,ElementName,PathForRelativPath,ForceAbsolutPath,ReplaceList,ResKeyList,IsModel);
    SubElement.setAttribute("RessourceKey",RessourceKey);
    if (ResKeyList) {
        // Check if RessourceKey is already in the ResKeyList
        bool ToAppend=true;
        for (int i=0;i<ResKeyList->count();i++) if (ResKeyList->at(i)==RessourceKey) ToAppend=false;
        // If not found, then add it to the list
        if (ToAppend) ResKeyList->append(RessourceKey);
    }
    ParentElement->appendChild(SubElement);
}


/*************************************************************************************************************************************
    CLASS cVideoFile
*************************************************************************************************************************************/

cImageInCache::cImageInCache(int64_t Position,AVFrame *FiltFrame,AVFrame *FrameBufferYUV) {
    this->Position      =Position;
    this->FiltFrame     =FiltFrame;
    this->FrameBufferYUV=FrameBufferYUV;
}

cImageInCache::~cImageInCache() {
    if (FiltFrame) {
        av_frame_unref(FiltFrame);
        av_frame_free(&FiltFrame);
    }
    av_frame_free(&FrameBufferYUV);
}

//*************

cVideoFile::cVideoFile(cApplicationConfig *ApplicationConfig):cBaseMediaFile(ApplicationConfig) {
    Reset(OBJECTTYPE_VIDEOFILE);
}

void cVideoFile::Reset(OBJECTTYPE TheWantedObjectType) {
    cBaseMediaFile::Reset();

    MusicOnly               =(TheWantedObjectType==OBJECTTYPE_MUSICFILE);
    ObjectType              =TheWantedObjectType;
    IsOpen                  =false;
    StartPos                =QTime(0,0,0,0);   // Start position
    EndPos                  =QTime(0,0,0,0);   // End position
    FFMPEGstartTime         =0;

    // Video part
    IsMTS                   =false;
    FFMPEGvideoFile         =NULL;
    VideoDecoderCodec       =NULL;
    VideoDecoderCodecCtx    =NULL;
    VideoStreamNumber       =0;
    FrameBufferYUV          =NULL;
    FrameBufferYUVReady     =false;
    FrameBufferYUVPosition  =0;
    VideoCodecInfo          ="";
    VideoTrackNbr           =0;
    VideoStreamNumber       =-1;
    NbrChapters             =0;

    // Audio part
    FFMPEGaudioFile         =NULL;
    AudioDecoderCodec       =NULL;
    AudioDecoderCodecCtx    =NULL;
    LastAudioReadedPosition =-1;
    IsVorbis                =false;
    AudioCodecInfo          ="";
    AudioTrackNbr           =0;
    AudioStreamNumber       =-1;

    // Audio resampling
    RSC                     =NULL;
    RSC_InChannels          =2;
    RSC_OutChannels         =2;
    RSC_InSampleRate        =48000;
    RSC_OutSampleRate       =48000;
    RSC_InChannelLayout     =AV_CHANNEL_LAYOUT_STEREO;
    RSC_OutChannelLayout    =AV_CHANNEL_LAYOUT_STEREO;
    RSC_InSampleFmt         =AV_SAMPLE_FMT_S16;
    RSC_OutSampleFmt        =AV_SAMPLE_FMT_S16;

    // Filter part
    VideoFilterGraph        =NULL;
    VideoFilterIn           =NULL;
    VideoFilterOut          =NULL;
}

//====================================================================================================================

cVideoFile::~cVideoFile() {
    // Close LibAVFormat and LibAVCodec contexte for the file
    CloseCodecAndFile();
}

//====================================================================================================================

bool cVideoFile::DoAnalyseSound(QList<qreal> *Peak,QList<qreal> *Moyenne,bool *CancelFlag,qreal *Analysed) {
    bool IsAnalysed=LoadAnalyseSound(Peak,Moyenne);
    if (!IsAnalysed) {
        qint64          NewPosition=0,Position=-1;
        qint64          Duration=QTime(0,0,0,0).msecsTo(GetRealDuration());
        int             WantedValues;
        QList<qreal>    Values;
        int16_t         *Block=NULL,*CurData=NULL;
        cSoundBlockList AnalyseMusic;

        AnalyseMusic.SetFPS(2000,2,1000,AV_SAMPLE_FMT_S16);
        WantedValues=(Duration/2000);
        Peak->clear();
        Moyenne->clear();

        //*******************************************************************************************
        // Load music and compute music count, max value, 2000 peak and 2000 moyenne values
        // decibels=decibels>0?0.02*log10(decibels/32768.0):0;    // PCM S16 is from -48db to +48db
        //*******************************************************************************************
        while ((!*CancelFlag)&&(Position!=NewPosition)) {
            *Analysed=qreal(NewPosition)/qreal(Duration);
            QApplication::processEvents();
            Position=NewPosition;
            ReadFrame(true,Position*1000,true,false,&AnalyseMusic,1);
            NewPosition+=qreal(AnalyseMusic.ListCount())*AnalyseMusic.dDuration*qreal(1000);
            while (AnalyseMusic.ListCount()>0) {
                Block=AnalyseMusic.DetachFirstPacket();
                if (Block) {
                    CurData=Block;
                    for (int j=0;j<AnalyseMusic.SoundPacketSize/4;j++) {
                        int16_t sample16Bit =*CurData++;
                        double  decibels1=sample16Bit<0?-sample16Bit:sample16Bit;
                        sample16Bit =*CurData++;
                        double  decibels2=sample16Bit<0?-sample16Bit:sample16Bit;
                        double  decibels=(decibels1+decibels2)/2;
                        Values.append(decibels);
                        if (Values.count()==WantedValues) {
                            qreal vPeak=0,vMoyenne=0;
                            foreach (qreal V,Values) {
                                if (vPeak<V) vPeak=V;
                                vMoyenne=vMoyenne+V;
                            }
                            vMoyenne=vMoyenne/Values.count();
                            Peak->append(vPeak);
                            Moyenne->append(vMoyenne);
                            Values.clear();
                        }
                    }
                    av_free(Block);
                }
            }
        }
        // tempdata
        CurData=(int16_t *)AnalyseMusic.TempData;
        for (int j=0;j<AnalyseMusic.CurrentTempSize/4;j++) {
            int16_t sample16Bit =*CurData++;
            double  decibels1=sample16Bit<0?-sample16Bit:sample16Bit;
            sample16Bit =*CurData++;
            double  decibels2=sample16Bit<0?-sample16Bit:sample16Bit;
            double  decibels=(decibels1+decibels2)/2;
            Values.append(decibels);
            if (Values.count()==WantedValues) {
                qreal vPeak=0,vMoyenne=0;
                foreach (qreal V,Values) {
                    if (vPeak<V) vPeak=V;
                    vMoyenne=vMoyenne+V;
                }
                vMoyenne=vMoyenne/Values.count();
                Peak->append(vPeak);
                Moyenne->append(vMoyenne);
                Values.clear();
            }
        }
        if (Values.count()>0) {
            qreal vPeak=0,vMoyenne=0;
            foreach (qreal V,Values) {
                if (vPeak<V) vPeak=V;
                vMoyenne=vMoyenne+V;
            }
            vMoyenne=vMoyenne/Values.count();
            Peak->append(vPeak);
            Moyenne->append(vMoyenne);
            Values.clear();
        }

        // Compute MaxSoundValue as 90% of the max peak value
        QList<qreal> MaxVal;
        foreach (qreal Value,*Peak) MaxVal.append(Value);
        std::sort(MaxVal.begin(),MaxVal.end());
        qreal MaxSoundValue=MaxVal.count()>0?MaxVal[MaxVal.count()*0.9]:1;
        if( MaxSoundValue == 0 )
            MaxSoundValue = 1;

        // Adjust Peak and Moyenne values by transforming them as % of the max value
        for (int i=0;i<Peak->count();i++) {
            (*Peak)[i]   =(*Peak)[i]/MaxSoundValue;
            (*Moyenne)[i]=(*Moyenne)[i]/MaxSoundValue;
        }
        MaxVal.clear();
        foreach (qreal Value,*Moyenne) MaxVal.append(Value);
        std::sort(MaxVal.begin(),MaxVal.end());
        MaxSoundValue=MaxVal.count()>0?MaxVal[MaxVal.count()*0.9]:1;

        //**************************
        // End analyse
        //**************************
        IsAnalysed=true;
        SaveAnalyseSound(Peak,Moyenne,MaxSoundValue);
        if (EndPos>GetRealDuration()) EndPos=GetRealDuration();
    }
    return IsAnalysed;
}

//====================================================================================================================

bool cVideoFile::LoadBasicInformationFromDatabase(QDomElement *ParentElement,QString,QString,QStringList *,bool *,QList<cSlideThumbsTable::TRResKeyItem> *,bool) {
    ImageWidth       =ParentElement->attribute("ImageWidth").toInt();
    ImageHeight      =ParentElement->attribute("ImageHeight").toInt();
    ImageOrientation =ParentElement->attribute("ImageOrientation").toInt();
    ObjectGeometry   =ParentElement->attribute("ObjectGeometry").toInt();
    AspectRatio      =GetDoubleValue(*ParentElement,"AspectRatio");
    NbrChapters      =ParentElement->attribute("NbrChapters").toInt();
    VideoStreamNumber=ParentElement->attribute("VideoStreamNumber").toInt();
    VideoTrackNbr    =ParentElement->attribute("VideoTrackNbr").toInt();
    AudioStreamNumber=ParentElement->attribute("AudioStreamNumber").toInt();
    AudioTrackNbr    =ParentElement->attribute("AudioTrackNbr").toInt();
    if (ParentElement->hasAttribute("Duration"))            SetGivenDuration(QTime(0,0,0,0).addMSecs(ParentElement->attribute("Duration").toLongLong()));
    if (ParentElement->hasAttribute("RealDuration"))        SetRealAudioDuration(QTime(0,0,0,0).addMSecs(ParentElement->attribute("RealDuration").toLongLong()));
    if (ParentElement->hasAttribute("RealAudioDuration"))   SetRealAudioDuration(QTime(0,0,0,0).addMSecs(ParentElement->attribute("RealAudioDuration").toLongLong()));
    if (ParentElement->hasAttribute("RealVideoDuration"))   SetRealVideoDuration(QTime(0,0,0,0).addMSecs(ParentElement->attribute("RealVideoDuration").toLongLong()));
    if (ParentElement->hasAttribute("SoundLevel"))          SetSoundLevel(GetDoubleValue(*ParentElement,"SoundLevel"));
    if (ParentElement->hasAttribute("IsComputedAudioDuration"))  IsComputedAudioDuration=ParentElement->attribute("IsComputedAudioDuration")=="1";
    if (EndPos==QTime(0,0,0,0)) EndPos=GetRealDuration();
    return true;
}

//====================================================================================================================

void cVideoFile::SaveBasicInformationToDatabase(QDomElement *ParentElement,QString,QString,bool,cReplaceObjectList *,QList<qlonglong> *,bool) {
    ParentElement->setAttribute("ImageWidth",        ImageWidth);
    ParentElement->setAttribute("ImageHeight",       ImageHeight);
    ParentElement->setAttribute("ImageOrientation",  ImageOrientation);
    ParentElement->setAttribute("ObjectGeometry",    ObjectGeometry);
    ParentElement->setAttribute("AspectRatio",       QString("%1").arg(AspectRatio,0,'f'));
    ParentElement->setAttribute("Duration",          QTime(0,0,0,0).msecsTo(GetGivenDuration()));
    ParentElement->setAttribute("RealAudioDuration", QTime(0,0,0,0).msecsTo(GetRealAudioDuration()));
    if (ObjectType==OBJECTTYPE_VIDEOFILE) ParentElement->setAttribute("RealVideoDuration", QTime(0,0,0,0).msecsTo(GetRealVideoDuration()));
    ParentElement->setAttribute("SoundLevel",        GetSoundLevel());
    ParentElement->setAttribute("IsComputedAudioDuration",IsComputedAudioDuration?"1":"0");
    ParentElement->setAttribute("NbrChapters",       NbrChapters);
    ParentElement->setAttribute("VideoStreamNumber", VideoStreamNumber);
    ParentElement->setAttribute("VideoTrackNbr",     VideoTrackNbr);
    ParentElement->setAttribute("AudioStreamNumber", AudioStreamNumber);
    ParentElement->setAttribute("AudioTrackNbr",     AudioTrackNbr);
}

//====================================================================================================================
// Overloaded function use to dertermine if format of media file is correct

bool cVideoFile::CheckFormatValide(QWidget *Window) {
    bool IsOk=IsValide;

    // try to open file
    if (!OpenCodecAndFile()) {
        QString ErrorMessage =QApplication::translate("MainWindow","Format not supported","Error message");
        CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
        IsOk=false;
    }

    // check if file have at least one sound track compatible
    if ((IsOk)&&(AudioStreamNumber!=-1)) {
        if (!((FFMPEGaudioFile->streams[AudioStreamNumber]->codecpar->format!=AV_SAMPLE_FMT_S16)||(FFMPEGaudioFile->streams[AudioStreamNumber]->codecpar->format!=AV_SAMPLE_FMT_U8))) {
            QString ErrorMessage="\n"+QApplication::translate("MainWindow","This application support only audio track with unsigned 8 bits or signed 16 bits sample format","Error message");
            CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
            IsOk=false;
        }
    }

    // Try to load an image to ensure all is ok
    if (IsOk) {
        QImage *Image=ImageAt(true,0,NULL,true,1,false,false);
        if (Image) {
            delete Image;
        } else {
            QString ErrorMessage="\n"+QApplication::translate("MainWindow","Impossible to read one image from the file","Error message");
            CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
            IsOk=false;
        }
    }

    // close file if it was opened
    CloseCodecAndFile();

    return IsOk;
}

//====================================================================================================================

// Overloaded function use to dertermine if media file correspond to WantedObjectType
//      WantedObjectType could be OBJECTTYPE_VIDEOFILE or OBJECTTYPE_MUSICFILE
//      if AudioOnly was set to true in constructor then ignore all video track and set WantedObjectType to OBJECTTYPE_MUSICFILE else set it to OBJECTTYPE_VIDEOFILE
//      return true if WantedObjectType=OBJECTTYPE_VIDEOFILE and at least one video track is present
//      return true if WantedObjectType=OBJECTTYPE_MUSICFILE and at least one audio track is present

bool cVideoFile::GetChildFullInformationFromFile(bool,cCustomIcon *Icon,QStringList *ExtendedProperties) {
    bool            Continu=true;
    AVFormatContext *FFMPEGfile=NULL;
    QString         sFileName=FileName();

    //*********************************************************************************************************
    // Open file and get a LibAVFormat context and an associated LibAVCodec decoder
    //*********************************************************************************************************
    char filename[512];
    strcpy(filename,sFileName.toLocal8Bit());
    if (avformat_open_input(&FFMPEGfile,filename,NULL,NULL)!=0) {
        FFMPEGfile=NULL;
        return false;
    }
    ExtendedProperties->append(QString("Short Format##")+QString(FFMPEGfile->iformat->name));
    ExtendedProperties->append(QString("Long Format##")+QString(FFMPEGfile->iformat->long_name));
    FFMPEGfile->flags|=AVFMT_FLAG_GENPTS;       // Generate missing pts even if it requires parsing future NbrFrames.

    //*********************************************************************************************************
    // Search stream in file
    //*********************************************************************************************************
    if (avformat_find_stream_info(FFMPEGfile,NULL)<0) {
        avformat_close_input(&FFMPEGfile);
        FFMPEGfile=NULL;
        Continu=false;
    }

    if (Continu) {
        //*********************************************************************************************************
        // Get metadata
        //*********************************************************************************************************
        AVDictionaryEntry *tag=NULL;
        while ((tag=av_dict_get(FFMPEGfile->metadata,"",tag,AV_DICT_IGNORE_SUFFIX))) {
            QString Value=QString().fromUtf8(tag->value);
            if (Value.endsWith("\n")) Value=Value.left(Value.lastIndexOf("\n"));
            ExtendedProperties->append(QString().fromUtf8(tag->key).toLower()+QString("##")+Value);
        }

        //*********************************************************************************************************
        // Get chapters
        //*********************************************************************************************************
        NbrChapters=0;
        for (uint i=0;i<FFMPEGfile->nb_chapters;i++) {
            AVChapter   *ch=FFMPEGfile->chapters[i];
            QString     ChapterNum=QString("%1").arg(NbrChapters);
            while (ChapterNum.length()<3) ChapterNum="0"+ChapterNum;
            int64_t Start=double(ch->start)*(double(av_q2d(ch->time_base))*1000);     // Lib AV use 1/1 000 000 000 sec and we want msec !
            int64_t End  =double(ch->end)*(double(av_q2d(ch->time_base))*1000);       // Lib AV use 1/1 000 000 000 sec and we want msec !

            // Special case if it's first chapter and start!=0 => add a chapter 0
            if ((NbrChapters==0)&&(FFMPEGfile->chapters[i]->start>0)) {
                ExtendedProperties->append("Chapter_"+ChapterNum+":Start"   +QString("##")+QTime(0,0,0,0).toString("hh:mm:ss.zzz"));
                ExtendedProperties->append("Chapter_"+ChapterNum+":End"     +QString("##")+QTime(0,0,0,0).addMSecs(Start).toString("hh:mm:ss.zzz"));
                ExtendedProperties->append("Chapter_"+ChapterNum+":Duration"+QString("##")+QTime(0,0,0,0).addMSecs(Start).toString("hh:mm:ss.zzz"));
                if (GetInformationValue("title",ExtendedProperties)!="") ExtendedProperties->append("Chapter_"+ChapterNum+":title##"+GetInformationValue("title",ExtendedProperties));
                    else ExtendedProperties->append("Chapter_"+ChapterNum+":title##"+QFileInfo(sFileName).baseName());
                NbrChapters++;
                ChapterNum=QString("%1").arg(NbrChapters);
                while (ChapterNum.length()<3) ChapterNum="0"+ChapterNum;
            }

            ExtendedProperties->append("Chapter_"+ChapterNum+":Start"   +QString("##")+QTime(0,0,0,0).addMSecs(Start).toString("hh:mm:ss.zzz"));
            ExtendedProperties->append("Chapter_"+ChapterNum+":End"     +QString("##")+QTime(0,0,0,0).addMSecs(End).toString("hh:mm:ss.zzz"));
            ExtendedProperties->append("Chapter_"+ChapterNum+":Duration"+QString("##")+QTime(0,0,0,0).addMSecs(End-Start).toString("hh:mm:ss.zzz"));
            // Chapter metadata
            while ((tag=av_dict_get(ch->metadata,"",tag,AV_DICT_IGNORE_SUFFIX)))
                ExtendedProperties->append("Chapter_"+ChapterNum+":"+QString().fromUtf8(tag->key).toLower()+QString("##")+QString().fromUtf8(tag->value));

            NbrChapters++;
        }

        //*********************************************************************************************************
        // Get information about duration
        //*********************************************************************************************************
        int64_t ms=FFMPEGfile->duration/1000;
        int     ss=ms/1000;
        int     mm=ss/60;
        int     hh=mm/60;
        mm      =mm-(hh*60);
        ss      =ss-(ss/60)*60;
        ms      =ms-(ms/1000)*1000;
        SetGivenDuration(QTime(hh,mm,ss,ms));
        EndPos  =GetRealDuration();

        //*********************************************************************************************************
        // Get information from track
        //*********************************************************************************************************
        for (int Track=0;Track<(int)FFMPEGfile->nb_streams;Track++) {

            // Find codec
            const AVCodec *Codec=avcodec_find_decoder(FFMPEGfile->streams[Track]->codecpar->codec_id);
            AVCodecContext *CodecCtx=avcodec_alloc_context3(Codec);
            avcodec_parameters_to_context(CodecCtx,FFMPEGfile->streams[Track]->codecpar);

            //*********************************************************************************************************
            // Audio track
            //*********************************************************************************************************
            if (CodecCtx->codec_type==AVMEDIA_TYPE_AUDIO) {
                // Keep this as default track
                if (AudioStreamNumber==-1) AudioStreamNumber=Track;

                // Compute TrackNum
                QString TrackNum=QString("%1").arg(AudioTrackNbr);
                while (TrackNum.length()<3) TrackNum="0"+TrackNum;
                TrackNum="Audio_"+TrackNum+":";

                // General
                ExtendedProperties->append(TrackNum+QString("Track")+QString("##")+QString("%1").arg(Track));
                if (Codec) ExtendedProperties->append(TrackNum+QString("Codec")+QString("##")+QString(Codec->name));

                // Channels and Sample format
                QString SampleFMT="";
                switch (CodecCtx->sample_fmt) {
                    case AV_SAMPLE_FMT_U8  : SampleFMT="-U8";   ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"unsigned 8 bits");          break;
                    case AV_SAMPLE_FMT_S16 : SampleFMT="-S16";  ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"signed 16 bits");           break;
                    case AV_SAMPLE_FMT_S32 : SampleFMT="-S32";  ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"signed 32 bits");           break;
                    case AV_SAMPLE_FMT_FLT : SampleFMT="-FLT";  ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"float");                    break;
                    case AV_SAMPLE_FMT_DBL : SampleFMT="-DBL";  ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"double");                   break;
                    case AV_SAMPLE_FMT_U8P : SampleFMT="-U8P";  ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"unsigned 8 bits, planar");  break;
                    case AV_SAMPLE_FMT_S16P: SampleFMT="-S16P"; ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"signed 16 bits, planar");   break;
                    case AV_SAMPLE_FMT_S32P: SampleFMT="-S32P"; ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"signed 32 bits, planar");   break;
                    case AV_SAMPLE_FMT_FLTP: SampleFMT="-FLTP"; ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"float, planar");            break;
                    case AV_SAMPLE_FMT_DBLP: SampleFMT="-DBLP"; ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"double, planar");           break;
                    default                : SampleFMT="-?";    ExtendedProperties->append(TrackNum+QString("Sample format")+QString("##")+"Unknown");                  break;
                }
                if (CodecCtx->ch_layout.nb_channels==1)
                  ExtendedProperties->append(TrackNum+QString("Channels")+QString("##")+QApplication::translate("cBaseMediaFile","Mono","Audio channels mode")+SampleFMT);
                else if (CodecCtx->ch_layout.nb_channels==2)
                  ExtendedProperties->append(TrackNum+QString("Channels")+QString("##")+QApplication::translate("cBaseMediaFile","Stereo","Audio channels mode")+SampleFMT);
                else
                  ExtendedProperties->append(TrackNum+QString("Channels")+QString("##")+QString("%1").arg(CodecCtx->ch_layout.nb_channels)+SampleFMT);
                // Frequency
                if (int(CodecCtx->sample_rate/1000)*1000>0) {
                    if (int(CodecCtx->sample_rate/1000)*1000==CodecCtx->sample_rate)
                         ExtendedProperties->append(TrackNum+QString("Frequency")+QString("##")+QString("%1").arg(int(CodecCtx->sample_rate/1000))+"Khz");
                    else ExtendedProperties->append(TrackNum+QString("Frequency")+QString("##")+QString("%1").arg(double(CodecCtx->sample_rate)/1000,8,'f',1).trimmed()+"Khz");
                }

                // Bitrate
                if (int(CodecCtx->bit_rate/1000)>0) ExtendedProperties->append(TrackNum+QString("Bitrate")+QString("##")+QString("%1").arg(int(CodecCtx->bit_rate/1000))+"Kb/s");

                // Stream metadata
                while ((tag=av_dict_get(FFMPEGfile->streams[Track]->metadata,"",tag,AV_DICT_IGNORE_SUFFIX))) {
                    // OGV container affect TAG to audio stream !
                    QString Key=QString().fromUtf8(tag->key).toLower();
                    if ((sFileName.toLower().endsWith(".ogv"))&&((Key=="title")||(Key=="artist")||(Key=="album")||(Key=="comment")||(Key=="date")||(Key=="composer")||(Key=="encoder")))
                             ExtendedProperties->append(Key+QString("##")+QString().fromUtf8(tag->value));
                        else ExtendedProperties->append(TrackNum+Key+QString("##")+QString().fromUtf8(tag->value));
                }

                // Ensure language exist (Note : AVI and FLV container own language at container level instead of track level)
                if (GetInformationValue(TrackNum+"language",ExtendedProperties)=="") {
                    QString Lng=GetInformationValue("language",ExtendedProperties);
                    ExtendedProperties->append(TrackNum+QString("language##")+(Lng==""?"und":Lng));
                }

                // Next
                AudioTrackNbr++;

            //*********************************************************************************************************
            // Video track
            //*********************************************************************************************************
            } else if (!MusicOnly && (CodecCtx->codec_type==AVMEDIA_TYPE_VIDEO)) {
                // Compute TrackNum
                QString TrackNum=QString("%1").arg(VideoTrackNbr);
                while (TrackNum.length()<3) TrackNum="0"+TrackNum;
                TrackNum="Video_"+TrackNum+":";

                // General
                ExtendedProperties->append(TrackNum+QString("Track")+QString("##")+QString("%1").arg(Track));
                if (Codec) ExtendedProperties->append(TrackNum+QString("Codec")+QString("##")+QString(Codec->name));

                // Bitrate
                if (CodecCtx->bit_rate>0) ExtendedProperties->append(TrackNum+QString("Bitrate")+QString("##")+QString("%1").arg(int(CodecCtx->bit_rate/1000))+"Kb/s");

                // Frame rate
                if (int(double(FFMPEGfile->streams[Track]->avg_frame_rate.num)/double(FFMPEGfile->streams[Track]->avg_frame_rate.den))>0) {
                    if (int(double(FFMPEGfile->streams[Track]->avg_frame_rate.num)/double(FFMPEGfile->streams[Track]->avg_frame_rate.den))==double(FFMPEGfile->streams[Track]->avg_frame_rate.num)/double(FFMPEGfile->streams[Track]->avg_frame_rate.den))
                         ExtendedProperties->append(TrackNum+QString("Frame rate")+QString("##")+QString("%1").arg(int(double(FFMPEGfile->streams[Track]->avg_frame_rate.num)/double(FFMPEGfile->streams[Track]->avg_frame_rate.den)))+" FPS");
                    else ExtendedProperties->append(TrackNum+QString("Frame rate")+QString("##")+QString("%1").arg(double(double(FFMPEGfile->streams[Track]->avg_frame_rate.num)/double(FFMPEGfile->streams[Track]->avg_frame_rate.den)),8,'f',3).trimmed()+" FPS");
                }

                // Stream metadata
                while ((tag=av_dict_get(FFMPEGfile->streams[Track]->metadata,"",tag,AV_DICT_IGNORE_SUFFIX)))
                    ExtendedProperties->append(TrackNum+QString(tag->key)+QString("##")+QString().fromUtf8(tag->value));

                // Ensure language exist (Note : AVI ‘AttachedPictureFrame’and FLV container own language at container level instead of track level)
                if (GetInformationValue(TrackNum+"language",ExtendedProperties)=="") {
                    QString Lng=GetInformationValue("language",ExtendedProperties);
                    ExtendedProperties->append(TrackNum+QString("language##")+(Lng==""?"und":Lng));
                }

                // Keep this as default track
                if (VideoStreamNumber==-1) {
                    QImage  *Img=NULL;
                    AVFrame *FrameBufYUV=NULL;


                    // Search if a jukebox mode thumbnail (jpg file with same name as video) exist
                    QFileInfo   File(sFileName);
                    QString     JPegFile=File.absolutePath()+(File.absolutePath().endsWith(QDir::separator())?"":QString(QDir::separator()))+File.completeBaseName()+".jpg";
                    if (QFileInfo(JPegFile).exists()) Icon->LoadIcons(JPegFile);

                    VideoStreamNumber=Track;
                    IsMTS=(sFileName.toLower().endsWith(".mts",Qt::CaseInsensitive) || sFileName.toLower().endsWith(".m2ts",Qt::CaseInsensitive) || sFileName.toLower().endsWith(".mod",Qt::CaseInsensitive));
                    FFMPEGfile->flags|=AVFMT_FLAG_GENPTS;       // Generate missing pts even if it requires parsing future NbrFrames.
                    FFMPEGfile->streams[VideoStreamNumber]->discard=AVDISCARD_DEFAULT;  // Setup STREAM options

                    // Setup decoder options
                    CodecCtx->debug            =0;                    // Debug level (0=nothing)
                    CodecCtx->workaround_bugs  =1;                    // Work around bugs in encoders which sometimes cannot be detected automatically : 1=autodetection
                    CodecCtx->idct_algo        =FF_IDCT_AUTO;         // IDCT algorithm, 0=auto
                    CodecCtx->skip_frame       =AVDISCARD_DEFAULT;    // ???????
                    CodecCtx->skip_idct        =AVDISCARD_DEFAULT;    // ???????
                    CodecCtx->skip_loop_filter =AVDISCARD_DEFAULT;    // ???????
                    CodecCtx->error_concealment=3;
                    CodecCtx->thread_count     =getCpuCount();
                    CodecCtx->thread_type      =getThreadFlags(CodecCtx->codec_id);

                    // Hack to correct wrong frame rates that seem to be generated by some codecs
                    if (CodecCtx->time_base.num>1000 && CodecCtx->time_base.den==1) CodecCtx->time_base.den=1000;

                    if (avcodec_open2(CodecCtx,Codec,NULL)>=0) {
                        // Get Aspect Ratio

                        if (CodecCtx->sample_aspect_ratio.num!=0)
                            AspectRatio=double(CodecCtx->sample_aspect_ratio.num)/double(CodecCtx->sample_aspect_ratio.den);
                        else if (FFMPEGfile->streams[VideoStreamNumber]->sample_aspect_ratio.num!=0)
                            AspectRatio=double(FFMPEGfile->streams[VideoStreamNumber]->sample_aspect_ratio.num)/double(FFMPEGfile->streams[VideoStreamNumber]->sample_aspect_ratio.den);
                        else
                            AspectRatio=1;

                        // Special case for DVD mode video without PAR
                        if ((AspectRatio==1)&&(CodecCtx->coded_width==720)&&((CodecCtx->coded_height==576)||(CodecCtx->coded_height==480)))
                            AspectRatio=double((CodecCtx->coded_height/3)*4)/720;

                        // Try to load one image of the file to be sure we can make something with this file
                        // and use this image as thumbnail
                        int64_t   Position =0;
                        double    dEndFile =double(QTime(0,0,0,0).msecsTo(GetRealDuration()))/1000;    // End File Position in double format
                        if (dEndFile!=0) {
                            // Allocate structure for YUV image

                            FrameBufYUV=av_frame_alloc();

                            if (FrameBufYUV!=NULL) {

                                AVStream    *VideoStream    =FFMPEGfile->streams[VideoStreamNumber];
                                AVPacket    *StreamPacket   =NULL;
                                bool        Continue        =true;
                                bool        IsVideoFind     =false;
                                double      FrameTimeBase   =av_q2d(VideoStream->time_base);
                                double      FramePosition   =0;
                                
                                while (Continue) {
                                    StreamPacket=av_packet_alloc();
                                    StreamPacket->flags|=AV_PKT_FLAG_KEY;  // HACK for CorePNG to decode as normal PNG by default
                                    if (av_read_frame(FFMPEGfile,StreamPacket)==0) {
                                        if (StreamPacket->stream_index==VideoStreamNumber) {
                                            if (avcodec_send_packet(CodecCtx, StreamPacket)<0)
                                                ToLog(LOGMSG_INFORMATION,"IN:cVideoFile::OpenCodecAndFile : avcodec_send_packet return an error");
                                            if (avcodec_receive_frame(CodecCtx, FrameBufYUV)>=0) {
                                                int64_t pts=AV_NOPTS_VALUE;
                                                if ((FrameBufYUV->pkt_dts==(int64_t)AV_NOPTS_VALUE)&&(FrameBufYUV->pts!=(int64_t)AV_NOPTS_VALUE)) pts=FrameBufYUV->pts; else pts=FrameBufYUV->pkt_dts;
                                                if (pts==(int64_t)AV_NOPTS_VALUE) pts=0;
                                                FramePosition         =double(pts)*FrameTimeBase;
                                                Img                   =ConvertYUVToRGB(false,FrameBufYUV);      // Create Img from YUV Buffer
                                                IsVideoFind           =(Img!=NULL)&&(!Img->isNull());
                                                ObjectGeometry        =IMAGE_GEOMETRY_UNKNOWN;
                                            }
                                        }
                                        // Check if we need to continue loop
                                        Continue=(IsVideoFind==false)&&(FramePosition<dEndFile);
                                    } else {
                                        // if error in av_read_frame(...) then may be we have reach the end of file !
                                        Continue=false;
                                    }
                                    // Continue with a new one
                                    if (StreamPacket!=NULL) {
                                        av_packet_unref(StreamPacket); // Free the StreamPacket that was allocated by previous call to av_read_frame
                                        delete StreamPacket;
                                        StreamPacket=NULL;
                                    }
                                }
                                if ((!IsVideoFind)&&(!Img)) {
                                    ToLog(LOGMSG_CRITICAL,QString("No video image return for position %1 => return black frame").arg(Position));
                                    Img=new QImage(CodecCtx->width,CodecCtx->height,QImage::Format_ARGB32_Premultiplied);
                                    Img->fill(0);
                                }
                                av_frame_free(&FrameBufYUV);

                            } else ToLog(LOGMSG_CRITICAL,"Error in cVideoFile::OpenCodecAndFile : Impossible to allocate FrameBufYUV");
                        } else ToLog(LOGMSG_CRITICAL,"Error in cVideoFile::OpenCodecAndFile : dEndFile=0 ?????");
                    }
                    if (Img) {
                        // Get information about size image
                        ImageWidth =Img->width();
                        ImageHeight=Img->height();
                        // Compute image geometry
                        ObjectGeometry=IMAGE_GEOMETRY_UNKNOWN;
                        double RatioHW=double(ImageWidth)/double(ImageHeight);
                        if ((RatioHW>=1.45)&&(RatioHW<=1.55))           ObjectGeometry=IMAGE_GEOMETRY_3_2;
                        else if ((RatioHW>=0.65)&&(RatioHW<=0.67))      ObjectGeometry=IMAGE_GEOMETRY_2_3;
                        else if ((RatioHW>=1.32)&&(RatioHW<=1.34))      ObjectGeometry=IMAGE_GEOMETRY_4_3;
                        else if ((RatioHW>=0.74)&&(RatioHW<=0.76))      ObjectGeometry=IMAGE_GEOMETRY_3_4;
                        else if ((RatioHW>=1.77)&&(RatioHW<=1.79))      ObjectGeometry=IMAGE_GEOMETRY_16_9;
                        else if ((RatioHW>=0.56)&&(RatioHW<=0.58))      ObjectGeometry=IMAGE_GEOMETRY_9_16;
                        else if ((RatioHW>=2.34)&&(RatioHW<=2.36))      ObjectGeometry=IMAGE_GEOMETRY_40_17;
                        else if ((RatioHW>=0.42)&&(RatioHW<=0.44))      ObjectGeometry=IMAGE_GEOMETRY_17_40;
                        // Icon
                        if (Icon->Icon16.isNull()) {
                            QImage Final=(Video_ThumbWidth==162?ApplicationConfig->VideoMask_162:Video_ThumbWidth==150?ApplicationConfig->VideoMask_150:ApplicationConfig->VideoMask_120).copy();
                            QImage ImgF;
                            if (Img->width()>Img->height()) ImgF=Img->scaledToWidth(Video_ThumbWidth-2,Qt::SmoothTransformation);
                                else                        ImgF=Img->scaledToHeight(Video_ThumbHeight*0.7,Qt::SmoothTransformation);
                            QPainter Painter;
                            Painter.begin(&Final);
                            Painter.drawImage(QRect((Final.width()-ImgF.width())/2,(Final.height()-ImgF.height())/2,ImgF.width(),ImgF.height()),ImgF);
                            Painter.end();
                            Icon->LoadIcons(&Final);
                        }
                        delete Img;
                    }

                }

                // Next
                VideoTrackNbr++;

            }

            // Free the codec context
            avcodec_free_context(&CodecCtx);

            //*********************************************************************************************************
            // Thumbnails (since lavf 54.2.0 - avformat.h)
            //*********************************************************************************************************
            if (FFMPEGfile->streams[Track]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                AVStream *ThumbStream=FFMPEGfile->streams[Track];
                AVPacket pkt         =ThumbStream->attached_pic;
                AVFrame  *FrameYUV=av_frame_alloc();
                if (FrameYUV) {

                    Codec=avcodec_find_decoder(ThumbStream->codecpar->codec_id);
                    CodecCtx=avcodec_alloc_context3(Codec);
                    avcodec_parameters_to_context(CodecCtx,ThumbStream->codecpar);

                    // Setup decoder options
                    CodecCtx->debug            =0;                    // Debug level (0=nothing)
                    CodecCtx->workaround_bugs  =1;                    // Work around bugs in encoders which sometimes cannot be detected automatically : 1=autodetection
                    CodecCtx->idct_algo        =FF_IDCT_AUTO;         // IDCT algorithm, 0=auto
                    CodecCtx->skip_frame       =AVDISCARD_DEFAULT;    // ???????
                    CodecCtx->skip_idct        =AVDISCARD_DEFAULT;    // ???????
                    CodecCtx->skip_loop_filter =AVDISCARD_DEFAULT;    // ???????
                    CodecCtx->error_concealment=3;
                    CodecCtx->thread_count     =getCpuCount();
                    CodecCtx->thread_type      =getThreadFlags(ThumbStream->codecpar->codec_id);
                    if (avcodec_open2(CodecCtx,Codec,NULL)>=0) {
                        if (avcodec_send_packet(CodecCtx, &pkt)>=0) {
                            if (avcodec_receive_frame(CodecCtx, FrameYUV)>=0) {
                                int     W=FrameYUV->width, RealW=(W/8)*8;    if (RealW<W) RealW+=8;
                                int     H=FrameYUV->height,RealH=(H/8)*8;    if (RealH<H) RealH+=8;;
                                
                                QImage  Thumbnail(RealW,RealH,QTPIXFMT);
                                AVFrame *FrameRGB=av_frame_alloc();
                                if ((FrameRGB)&&(!Thumbnail.isNull())) {
                                    av_image_fill_arrays(FrameRGB->data,FrameRGB->linesize,Thumbnail.bits(),PIXFMT,RealW,RealH,1);
                                    struct SwsContext *img_convert_ctx=sws_getContext(FrameYUV->width,FrameYUV->height,(AVPixelFormat)FrameYUV->format,RealW,RealH,PIXFMT,SWS_FAST_BILINEAR,NULL,NULL,NULL);
                                    if (img_convert_ctx!=NULL) {
                                        int ret = sws_scale(img_convert_ctx,FrameYUV->data,FrameYUV->linesize,0,FrameYUV->height,FrameRGB->data,FrameRGB->linesize);
                                        if (ret>0) {
                                            // sws_scaler truncate the width of the images to a multiple of 8. So cut resulting image to comply a multiple of 8
                                            Thumbnail=Thumbnail.copy(0,0,W,H);
                                            Icon->LoadIcons(&Thumbnail);
                                        }
                                        sws_freeContext(img_convert_ctx);
                                    }
                                }
                                if (FrameRGB) av_frame_free(&FrameRGB);
                            }
                        }
                    }
                    avcodec_free_context(&CodecCtx);
                }
                if (FrameYUV) av_frame_free(&FrameYUV);
            }
        }

        // if no icon then load default for type
        if (Icon->Icon16.isNull() || Icon->Icon100.isNull())
            Icon->LoadIcons(ObjectType==OBJECTTYPE_VIDEOFILE?&ApplicationConfig->DefaultVIDEOIcon:&ApplicationConfig->DefaultMUSICIcon);
    }

    // Close the ffmpeg file
    if (FFMPEGfile!=NULL) {
        avformat_close_input(&FFMPEGfile);
        FFMPEGfile=NULL;
    }

    return Continu;
}

//====================================================================================================================

QString cVideoFile::GetFileTypeStr() {
    if (MusicOnly || (ObjectType==OBJECTTYPE_MUSICFILE)) return QApplication::translate("cBaseMediaFile","Music","File type");
        else return QApplication::translate("cBaseMediaFile","Video","File type");
}

//====================================================================================================================

QImage *cVideoFile::GetDefaultTypeIcon(cCustomIcon::IconSize Size) {
    if (MusicOnly || (ObjectType==OBJECTTYPE_MUSICFILE)) return ApplicationConfig->DefaultMUSICIcon.GetIcon(Size);
        else return ApplicationConfig->DefaultVIDEOIcon.GetIcon(Size);
}

//====================================================================================================================

QString cVideoFile::GetTechInfo(QStringList *ExtendedProperties) {
    QString Info="";
    if (ObjectType==OBJECTTYPE_MUSICFILE) {
        Info=GetCumulInfoStr(ExtendedProperties,"Audio","Codec");
        if (GetCumulInfoStr(ExtendedProperties,"Audio","Channels")!="")       Info=Info+(Info!=""?"-":"")+GetCumulInfoStr(ExtendedProperties,"Audio","Channels");
        if (GetCumulInfoStr(ExtendedProperties,"Audio","Bitrate")!="")        Info=Info+(Info!=""?"-":"")+GetCumulInfoStr(ExtendedProperties,"Audio","Bitrate");
        if (GetCumulInfoStr(ExtendedProperties,"Audio","Frequency")!="")      Info=Info+(Info!=""?"-":"")+GetCumulInfoStr(ExtendedProperties,"Audio","Frequency");
    } else {
        Info=GetImageSizeStr();
        if (GetCumulInfoStr(ExtendedProperties,"Video","Codec")!="")          Info=Info+(Info!=""?"-":"")+GetCumulInfoStr(ExtendedProperties,"Video","Codec");
        if (GetCumulInfoStr(ExtendedProperties,"Video","Frame rate")!="")     Info=Info+(Info!=""?"-":"")+GetCumulInfoStr(ExtendedProperties,"Video","Frame rate");
        if (GetCumulInfoStr(ExtendedProperties,"Video","Bitrate")!="")        Info=Info+(Info!=""?"-":"")+GetCumulInfoStr(ExtendedProperties,"Video","Bitrate");

        int     Num     =0;
        QString TrackNum="";
        QString Value   ="";
        QString SubInfo ="";
        do {
            TrackNum=QString("%1").arg(Num);
            while (TrackNum.length()<3) TrackNum="0"+TrackNum;
            TrackNum="Audio_"+TrackNum+":";
            Value=GetInformationValue(TrackNum+"language",ExtendedProperties);
            if (Value!="") {
                if (Num==0) Info=Info+"-"; else Info=Info+"/";
                SubInfo=GetInformationValue(TrackNum+"Codec",ExtendedProperties);
                if (GetInformationValue(TrackNum+"Channels",ExtendedProperties)!="")  SubInfo=SubInfo+(Info!=""?"-":"")+GetInformationValue(TrackNum+"Channels",ExtendedProperties);
                if (GetInformationValue(TrackNum+"Bitrate",ExtendedProperties)!="")   SubInfo=SubInfo+(Info!=""?"-":"")+GetInformationValue(TrackNum+"Bitrate",ExtendedProperties);
                if (GetInformationValue(TrackNum+"Frequency",ExtendedProperties)!="") SubInfo=SubInfo+(Info!=""?"-":"")+GetInformationValue(TrackNum+"Frequency",ExtendedProperties);
                Info=Info+Value+"("+SubInfo+")";
            }
            // Next
            Num++;
        } while (Value!="");
    }
    return Info;
}

//====================================================================================================================

QString cVideoFile::GetTAGInfo(QStringList *ExtendedProperties) {
    QString Info=GetInformationValue("track",ExtendedProperties);
    if (GetInformationValue("title",ExtendedProperties)!="")          Info=Info+(Info!=""?"-":"")+GetInformationValue("title",ExtendedProperties);
    if (GetInformationValue("artist",ExtendedProperties)!="")         Info=Info+(Info!=""?"-":"")+GetInformationValue("artist",ExtendedProperties);
    if (GetInformationValue("album",ExtendedProperties)!="")          Info=Info+(Info!=""?"-":"")+GetInformationValue("album",ExtendedProperties);
    if (GetInformationValue("date",ExtendedProperties)!="")           Info=Info+(Info!=""?"-":"")+GetInformationValue("date",ExtendedProperties);
    if (GetInformationValue("genre",ExtendedProperties)!="")          Info=Info+(Info!=""?"-":"")+GetInformationValue("genre",ExtendedProperties);
    return Info;
}

//====================================================================================================================
// Close LibAVFormat and LibAVCodec contexte for the file
//====================================================================================================================

void cVideoFile::CloseCodecAndFile() {
    while (CacheImage.count()>0) delete(CacheImage.takeLast());

    // Close the resampling context
    CloseResampler();

    // Close the filter context
    if (VideoFilterGraph)
        VideoFilter_Close();

    // Close the video codec context
    if (VideoDecoderCodecCtx!=NULL) {
        avcodec_free_context(&VideoDecoderCodecCtx);
        VideoDecoderCodec=NULL;
    }

    // Close the audio codec
    if (AudioDecoderCodecCtx!=NULL) {
        avcodec_free_context(&AudioDecoderCodecCtx);
        AudioDecoderCodec=NULL;
    }

    // Close the ffmpeg files
    if (FFMPEGaudioFile!=NULL) {
        avformat_close_input(&FFMPEGaudioFile);
        FFMPEGaudioFile=NULL;
    }
    if (FFMPEGvideoFile!=NULL) {
        avformat_close_input(&FFMPEGvideoFile);
        FFMPEGvideoFile=NULL;
    }

    if (FrameBufferYUV!=NULL) {
        av_frame_free(&FrameBufferYUV);
    }
    FrameBufferYUVReady=false;

    IsOpen=false;
}

//*********************************************************************************************************************

void cVideoFile::CloseResampler() {
    if (RSC) {
        swr_free(&RSC);
        RSC=NULL;
    }
}

//*********************************************************************************************************************
void cVideoFile::CheckResampler(int RSC_InChannels,int RSC_OutChannels,AVSampleFormat RSC_InSampleFmt,AVSampleFormat RSC_OutSampleFmt,int RSC_InSampleRate,int RSC_OutSampleRate,AVChannelLayout RSC_InChannelLayout,AVChannelLayout RSC_OutChannelLayout) {
    if ((RSC!=NULL)&&
        ( (RSC_InChannels!=this->RSC_InChannels)    ||(RSC_OutChannels!=this->RSC_OutChannels)
        ||(RSC_InSampleFmt!=this->RSC_InSampleFmt)  ||(RSC_OutSampleFmt!=this->RSC_OutSampleFmt)
        ||(RSC_InSampleRate!=this->RSC_InSampleRate)||(RSC_OutSampleRate!=this->RSC_OutSampleRate)
       )) CloseResampler();
    if (!RSC) {
        this->RSC_InChannels=RSC_InChannels;
        this->RSC_OutChannels=RSC_OutChannels;
        this->RSC_InSampleFmt=RSC_InSampleFmt;
        this->RSC_OutSampleFmt=RSC_OutSampleFmt;
        this->RSC_InSampleRate=RSC_InSampleRate;
        this->RSC_OutSampleRate=RSC_OutSampleRate;
        av_channel_layout_copy(&this->RSC_InChannelLayout, &RSC_InChannelLayout);
        av_channel_layout_copy(&this->RSC_OutChannelLayout, &RSC_OutChannelLayout);
        RSC=swr_alloc();
        av_opt_set_chlayout(RSC,"in_chlayout",&this->RSC_InChannelLayout,0);
        av_opt_set_int(RSC,"in_sample_rate",RSC_InSampleRate,0);
        av_opt_set_chlayout(RSC,"out_chlayout",&this->RSC_OutChannelLayout,0);
        av_opt_set_int(RSC,"out_sample_rate",RSC_OutSampleRate,0);
        av_opt_set_int(RSC,"in_channel_count",RSC_InChannels,0);
        av_opt_set_int(RSC,"out_channel_count",RSC_OutChannels,0);
        av_opt_set_sample_fmt(RSC,"in_sample_fmt",RSC_InSampleFmt,0);
        av_opt_set_sample_fmt(RSC,"out_sample_fmt",RSC_OutSampleFmt,0);
        if ((RSC)&&(swr_init(RSC)<0)) {
            ToLog(LOGMSG_CRITICAL,QString("CheckResampler: swr_init failed"));
            swr_free(&RSC);
            RSC=NULL;
        }
        if (!RSC) ToLog(LOGMSG_CRITICAL,QString("CheckResampler: swr_alloc_set_opts failed"));
    }
}

//*********************************************************************************************************************
// VIDEO FILTER PART : This code was adapt from xbmc sources files
//*********************************************************************************************************************

int cVideoFile::VideoFilter_Open() {
    int result;

    if (VideoFilterGraph) VideoFilter_Close();

    if (!(VideoFilterGraph=avfilter_graph_alloc())) {
        ToLog(LOGMSG_CRITICAL,QString("Error in cVideoFile::VideoFilter_Open : unable to alloc filter graph"));
        return -1;
    }

    VideoFilterGraph->scale_sws_opts = av_strdup("flags=4");

    char args[512];
    snprintf(args, sizeof(args),"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->width,FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->height,
             FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->format,
             FFMPEGvideoFile->streams[VideoStreamNumber]->avg_frame_rate.num,FFMPEGvideoFile->streams[VideoStreamNumber]->avg_frame_rate.den,
             FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->sample_aspect_ratio.num,FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->sample_aspect_ratio.den
    );

    const AVFilter *srcFilter=avfilter_get_by_name("buffer");
    const AVFilter *outFilter=avfilter_get_by_name("buffersink");

    if ((result=avfilter_graph_create_filter(&VideoFilterIn,srcFilter,"in",args,NULL,VideoFilterGraph))<0) {
        ToLog(LOGMSG_CRITICAL,QString("Error in cVideoFile::VideoFilter_Open : avfilter_graph_create_filter: src"));
        return result;
    }
    if ((result=avfilter_graph_create_filter(&VideoFilterOut,outFilter,"out",NULL,NULL,VideoFilterGraph))<0) {
        ToLog(LOGMSG_CRITICAL,QString("Error in cVideoFile::VideoFilter_Open : avfilter_graph_create_filter: out"));
        return result;
    }
    AVFilterInOut *outputs=avfilter_inout_alloc();
    AVFilterInOut *inputs =avfilter_inout_alloc();

    outputs->name = av_strdup("in");
    outputs->filter_ctx = VideoFilterIn;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = VideoFilterOut;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((result=avfilter_graph_parse_ptr(VideoFilterGraph,QString("yadif=deint=interlaced:mode=send_frame:parity=auto").toLocal8Bit().constData(),&inputs,&outputs,NULL))<0) {
        ToLog(LOGMSG_CRITICAL,QString("Error in cVideoFile::VideoFilter_Open : avfilter_graph_parse"));
        return result;
    }

    if ((result=avfilter_graph_config(VideoFilterGraph,NULL))<0) {
        ToLog(LOGMSG_CRITICAL,QString("Error in cVideoFile::VideoFilter_Open : avfilter_graph_config"));
        return result;
    }
    return result;
}

//====================================================================================================================

void cVideoFile::VideoFilter_Close() {
    if (VideoFilterGraph) avfilter_graph_free(&VideoFilterGraph);
    VideoFilterGraph=NULL;
    VideoFilterIn =NULL;
    VideoFilterOut=NULL;
}

//====================================================================================================================

bool cVideoFile::SeekFile(AVStream *VideoStream,AVStream *AudioStream,int64_t Position) {
    bool            ret=true;
    AVFormatContext *FFMPEGfile  =NULL;
    int             StreamNumber=0;

    // Reset context variables and buffers
    if (AudioStream) {
        CloseResampler();
        FFMPEGfile=FFMPEGaudioFile;
        StreamNumber=AudioStreamNumber;
    } else if (VideoStream) {
        if (VideoFilterGraph) VideoFilter_Close();
        while (CacheImage.count()>0) delete(CacheImage.takeLast());
        FrameBufferYUVReady   =false;
        FrameBufferYUVPosition=0;
        FFMPEGfile=FFMPEGvideoFile;
        StreamNumber=VideoStreamNumber;
    }

    if (Position<0) Position=0;

    // Flush ffmpeg buffers
    if (VideoDecoderCodecCtx) avcodec_flush_buffers(VideoDecoderCodecCtx);
    if (AudioDecoderCodecCtx) avcodec_flush_buffers(AudioDecoderCodecCtx);
    
    int64_t seek_target=av_rescale_q(Position,AV_TIME_BASE_Q,FFMPEGfile->streams[StreamNumber]->time_base);
    if (seek_target<0) seek_target=0;
    int errcode=0;
    if ((SeekErrorCount>0)||((errcode=avformat_seek_file(FFMPEGfile,StreamNumber,INT64_MIN,seek_target,INT64_MAX,AVSEEK_FLAG_BACKWARD))<0)) {
        if (SeekErrorCount==0) ToLog(LOGMSG_DEBUGTRACE,GetAvErrorMessage(errcode));
        // Try in AVSEEK_FLAG_ANY mode
        if ((errcode=av_seek_frame(FFMPEGfile,StreamNumber,seek_target,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_ANY))<0) {
            ToLog(LOGMSG_DEBUGTRACE,GetAvErrorMessage(errcode));
            // Try with default stream if exist
            int DefaultStream=av_find_default_stream_index(FFMPEGfile);
            if ((DefaultStream==StreamNumber)||(Position>0)||(DefaultStream<0)||((errcode=av_seek_frame(FFMPEGfile,DefaultStream,0,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_BYTE)<0))) {
                ToLog(LOGMSG_DEBUGTRACE,GetAvErrorMessage(errcode));
                ToLog(LOGMSG_CRITICAL,"Error in cVideoFile::ReadFrame : Seek error");
                ret=false;
            }
        }
    }

    // read first packet to ensure we have correct position !
    // elsewhere, redo seek 5 times until exit with error
    if (AudioStream) {
        AVPacket *StreamPacket=av_packet_alloc();
        StreamPacket->flags|=AV_PKT_FLAG_KEY;
        while (av_read_frame(FFMPEGfile,StreamPacket)!=0);
        int64_t FramePts=StreamPacket->pts!=(int64_t)AV_NOPTS_VALUE?StreamPacket->pts:-1;
        if ((FramePts<(Position/1000)-500)||(FramePts>(Position/1000)+500)) {
            SeekErrorCount++;
            if (SeekErrorCount<5) ret=SeekFile(VideoStream,AudioStream,Position);
        }
    }
    return ret;
}

//====================================================================================================================

uint8_t *cVideoFile::Resample(AVFrame *Frame,int64_t *SizeDecoded,int DstSampleSize) {
    uint8_t *Data=(uint8_t *)av_malloc(MaxAudioLenDecoded);
    uint8_t *out[]={Data};
    if (Data) *SizeDecoded=swr_convert(RSC,out,MaxAudioLenDecoded/DstSampleSize,(const uint8_t **)Frame->data,Frame->nb_samples)*DstSampleSize;
    return Data;
}

//====================================================================================================================
// return duration of one frame
//====================================================================================================================

qreal cVideoFile::GetFPSDuration() {
    qreal FPSDuration;
    if ((VideoStreamNumber>=0)&&(FFMPEGvideoFile->streams[VideoStreamNumber]))
        FPSDuration=qreal(FFMPEGvideoFile->streams[VideoStreamNumber]->avg_frame_rate.den*(AV_TIME_BASE/1000))/qreal(FFMPEGvideoFile->streams[VideoStreamNumber]->avg_frame_rate.num);
    else FPSDuration=1;
    return FPSDuration;
}

//====================================================================================================================
// Read a frame from current stream
//====================================================================================================================
// maximum diff between asked image position and founded image position
#define ALLOWEDDELTA    250000
// diff between asked image position and current image position before exit loop and return black frame
#define MAXDELTA        2500000

// Remark: Position must use AV_TIMEBASE Unit
QImage *cVideoFile::ReadFrame(bool PreviewMode,int64_t Position,bool DontUseEndPos,bool Deinterlace,cSoundBlockList *SoundTrackBloc,double Volume,int NbrDuration) {
    // Ensure file was previously open
    if ((!IsOpen)&&(!OpenCodecAndFile())) return NULL;

    // Ensure file have an end file Position
    double dEndFile=double(QTime(0,0,0,0).msecsTo(DontUseEndPos?GetRealDuration():EndPos))/1000;
    if (dEndFile==0) {
        ToLog(LOGMSG_CRITICAL,"Error in cVideoFile::ReadFrame : dEndFile=0 ?????");
        return NULL;
    }
    if (Position<0) Position=0;

    AVStream *VideoStream =(VideoStreamNumber>=0)?FFMPEGvideoFile->streams[VideoStreamNumber]:NULL;

    cVideoFile::sAudioContext AudioContext;
    AudioContext.SoundTrackBloc =SoundTrackBloc;
    AudioContext.AudioStream    =((AudioStreamNumber>=0)&&(SoundTrackBloc)?FFMPEGaudioFile->streams[AudioStreamNumber]:NULL);
    AudioContext.FPSSize        =SoundTrackBloc?SoundTrackBloc->SoundPacketSize*SoundTrackBloc->NbrPacketForFPS:0;
    AudioContext.FPSDuration    =AudioContext.FPSSize?(double(AudioContext.FPSSize)/(SoundTrackBloc->Channels*SoundTrackBloc->SampleBytes*SoundTrackBloc->SamplingRate))*AV_TIME_BASE:0;
    AudioContext.TimeBase       =AudioContext.AudioStream?double(AudioContext.AudioStream->time_base.den)/double(AudioContext.AudioStream->time_base.num):0;
    AudioContext.DstSampleSize  =SoundTrackBloc?(SoundTrackBloc->SampleBytes*SoundTrackBloc->Channels):0;
    AudioContext.NeedResampling =false;
    AudioContext.AudioLenDecoded=0;
    AudioContext.Counter        =20; // Retry counter (when len>0 and avcodec_decode_audio4 fail to retreave frame, we retry counter time before to discard the packet)
    AudioContext.Volume         =Volume;
    AudioContext.dEndFile       =&dEndFile;
    AudioContext.NbrDuration    =NbrDuration;
    AudioContext.DontUseEndPos  =DontUseEndPos;

    if (!AudioContext.FPSDuration) {
        if (PreviewMode)            AudioContext.FPSDuration=double(AV_TIME_BASE)/((cApplicationConfig *)ApplicationConfig)->PreviewFPS;
            else if (VideoStream)   AudioContext.FPSDuration=double(VideoStream->avg_frame_rate.den*AV_TIME_BASE)/double(VideoStream->avg_frame_rate.num);
            else                    AudioContext.FPSDuration=double(AV_TIME_BASE)/double(SoundTrackBloc->SamplingRate);
    }

    if ((!AudioContext.AudioStream)&&(!VideoStream)) return NULL;

    Mutex.lock();

    // If position >= end of file : disable audio (only if IsComputedAudioDuration)
    double dPosition=double(Position)/AV_TIME_BASE;
    if ((dPosition>0)&&(dPosition>=dEndFile+1000)&&(IsComputedAudioDuration)) {
        AudioContext.AudioStream=NULL; // Disable audio
        // Check if last image is ready and correspond to end of file
        if ((!LastImage.isNull())&&(FrameBufferYUVReady)&&(FrameBufferYUVPosition>=dEndFile*AV_TIME_BASE-AudioContext.FPSDuration)) {
            Mutex.unlock();
            return new QImage(LastImage.copy());
        }
        // If not then change Position to end file - a FPS to prepare a last image
        Position=dEndFile*AV_TIME_BASE-AudioContext.FPSDuration;
        dPosition=double(Position)/AV_TIME_BASE;
        if (SoundTrackBloc) SoundTrackBloc->UseLatestData();
    }

    //================================================
    bool     ContinueVideo     =true;
    AudioContext.ContinueAudio=(AudioContext.AudioStream)&&(SoundTrackBloc);
    bool     ResamplingContinue=(Position!=0);
    AudioContext.AudioFramePosition=dPosition;
    //================================================

    if (AudioContext.ContinueAudio) {
        AudioContext.NeedResampling=((AudioContext.AudioStream->codecpar->format != AV_SAMPLE_FMT_S16)||
                                     (AudioContext.AudioStream->codecpar->ch_layout.nb_channels != SoundTrackBloc->Channels)||
                                     (AudioContext.AudioStream->codecpar->sample_rate != SoundTrackBloc->SamplingRate));

        // Calc if we need to seek to a position
        int64_t Start =SoundTrackBloc->CurrentPosition;
        int64_t End   =Start+SoundTrackBloc->GetDuration();
        int64_t Wanted=AudioContext.FPSDuration*AudioContext.NbrDuration;
        if ((Position>=Start)&&(Position+Wanted<=End)) AudioContext.ContinueAudio=false;
        if ((AudioContext.ContinueAudio)&&((Position==0)||(Start<0)||(LastAudioReadedPosition<0)/*||(Position<Start)*/||(Position>End+1500000))) {
            if (Position<0) Position=0;
            SoundTrackBloc->ClearList();                // Clear soundtrack list
            ResamplingContinue=false;
            LastAudioReadedPosition=0;
            SeekErrorCount=0;
            SeekFile(NULL,AudioContext.AudioStream,Position-SoundTrackBloc->WantedDuration*1000);        // Always seek one FPS before to ensure eventual filter have time to init
            AudioContext.AudioFramePosition=Position/AV_TIME_BASE;
        }

        // Prepare resampler
        if ((AudioContext.ContinueAudio)&&(AudioContext.NeedResampling)) {
            if (!ResamplingContinue) CloseResampler();
            AVChannelLayout out_channel_layout;
            av_channel_layout_default(&out_channel_layout, SoundTrackBloc->Channels);
            CheckResampler(AudioContext.AudioStream->codecpar->ch_layout.nb_channels,SoundTrackBloc->Channels,
                           (AVSampleFormat)AudioContext.AudioStream->codecpar->format,SoundTrackBloc->SampleFormat,
                           AudioContext.AudioStream->codecpar->sample_rate,SoundTrackBloc->SamplingRate,
                           AudioContext.AudioStream->codecpar->ch_layout,
                           out_channel_layout);
        }
    }

    QImage   *RetImage         =NULL;
    int64_t  RetImagePosition  =0;
    double   VideoFramePosition=dPosition;

    // Count number of image > position
    int Nbr=0;
    for (int CNbr=0;CNbr<CacheImage.count();CNbr++) if ((CacheImage[CNbr]->Position>=Position)&&(CacheImage[CNbr]->Position-Position<ALLOWEDDELTA)) Nbr++;
    bool IsVideoFind=Nbr>0;

    ContinueVideo=((VideoStream)&&(!IsVideoFind));
    if (ContinueVideo) {
        int64_t DiffTimePosition=-1000000;  // Compute difftime between asked position and previous end decoded position

        if (FrameBufferYUVReady)
            DiffTimePosition=Position-FrameBufferYUVPosition;

        // Calc if we need to seek to a position
        if ((Position==0)||(DiffTimePosition<0)||(DiffTimePosition>1500000)) {// Allow 1,5 sec diff (rounded double !)
            if (Position<0) Position=0;
            SeekErrorCount=0;
            SeekFile(VideoStream,NULL,Position);        // Always seek one FPS before to ensure eventual filter have time to init
            VideoFramePosition=Position/AV_TIME_BASE;
        }
    }

    //*************************************************************************************************************************************
    // Decoding process : Get StreamPacket until endposition is reach (if sound is wanted) or until image is ok (if image only is wanted)
    //*************************************************************************************************************************************

    // AUDIO PART
    while (AudioContext.ContinueAudio) {
        AVPacket *StreamPacket=av_packet_alloc();
        if (!StreamPacket) {
            AudioContext.ContinueAudio=false;
        } else {
            StreamPacket->flags|=AV_PKT_FLAG_KEY;
            if (av_read_frame(FFMPEGaudioFile,StreamPacket)<0) {
                // If error reading frame then we considere we have reach the end of file
                if (!IsComputedAudioDuration) {
                    dEndFile=qreal(SoundTrackBloc->CurrentPosition)/AV_TIME_BASE;
                    dEndFile=dEndFile+qreal(SoundTrackBloc->GetDuration())/1000;
                    if (dEndFile==double(QTime(0,0,0,0).msecsTo(EndPos))/1000) EndPos=QTime(0,0,0).addMSecs(dEndFile*1000);
                    SetRealAudioDuration(QTime(0,0,0,0).addMSecs(qlonglong(dEndFile*1000)));
                }
                AudioContext.ContinueAudio=false;

                // Use data in TempData to create a latest block
                SoundTrackBloc->UseLatestData();

            } else {
                DecodeAudio(&AudioContext,StreamPacket,Position);
                StreamPacket=NULL;
            }
        }
        // Continue with a new one
        if (StreamPacket!=NULL) {
            av_packet_unref(StreamPacket); // Free the StreamPacket that was allocated by previous call to av_read_frame
            delete StreamPacket;
            StreamPacket=NULL;
        }
    }

    // VIDEO PART
    if (VideoStream) {
        if (!ContinueVideo) {
            ToLog(LOGMSG_DEBUGTRACE,QString("Video image for position %1 => use image in cache").arg(Position));
        } else if (Position<FFMPEGstartTime) {
            ToLog(LOGMSG_CRITICAL,QString("Image position %1 is before video stream start => return black frame").arg(Position));
            RetImage =new QImage(FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->width,FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->height,QImage::Format_ARGB32_Premultiplied);
            RetImage->fill(0);
            RetImagePosition=Position;
        } else {
            bool ByPassFirstImage=(Deinterlace)&&(CacheImage.count()==0);
            int  MaxErrorCount   =20;
            bool FreeFrames      =false;

            while (ContinueVideo) {
                AVPacket *StreamPacket=av_packet_alloc();
                if (!StreamPacket) {
                    ContinueVideo=false;
                } else {
                    StreamPacket->flags|=AV_PKT_FLAG_KEY;  // HACK for CorePNG to decode as normal PNG by default

                    int errcode=0;
                    if ((errcode=av_read_frame(FFMPEGvideoFile,StreamPacket))<0) {
                        if (errcode==AVERROR_EOF) {
                            // We have reach the end of file
                            if (!IsComputedAudioDuration) {
                                dEndFile=VideoFramePosition;
                                if (dEndFile==double(QTime(0,0,0,0).msecsTo(EndPos))/1000) EndPos=QTime(0,0,0).addMSecs(dEndFile*1000);
                                SetRealVideoDuration(QTime(0,0,0,0).addMSecs(qlonglong(dEndFile*1000)));
                            }
                            ContinueVideo=false;

                            if ((!LastImage.isNull())&&(FrameBufferYUVReady)&&(FrameBufferYUVPosition>=(dEndFile-1.5)*AV_TIME_BASE)) {
                                if (!RetImage) {
                                    RetImage=new QImage(LastImage);
                                    RetImagePosition=FrameBufferYUVPosition;
                                }
                                IsVideoFind=true;
                                ContinueVideo=false;
                            }
                        } else {
                            ToLog(LOGMSG_CRITICAL,GetAvErrorMessage(errcode));
                            // If error reading frame
                            if (MaxErrorCount>0) {
                                // Files with stream could provoke this, so we ignore the first MaxErrorCount errors
                                MaxErrorCount--;
                            } else {
                                if ((!LastImage.isNull())&&(FrameBufferYUVReady)&&(FrameBufferYUVPosition>=(dEndFile-1.5)*AV_TIME_BASE)) {
                                    if (!RetImage) {
                                        RetImage=new QImage(LastImage);
                                        RetImagePosition=FrameBufferYUVPosition;
                                    }
                                    IsVideoFind=true;
                                    ContinueVideo=false;
                                } else {
                                    SeekErrorCount=0;
                                    ContinueVideo=SeekFile(VideoStream,NULL,Position-2*AudioContext.FPSDuration);
                                }
                            }
                        }
                    } else {
                        int64_t FramePts=StreamPacket->pts!=(int64_t)AV_NOPTS_VALUE?StreamPacket->pts:-1;
                        double  TimeBase=double(FFMPEGvideoFile->streams[StreamPacket->stream_index]->time_base.den)/double(FFMPEGvideoFile->streams[StreamPacket->stream_index]->time_base.num);
                        if (FramePts>=0) VideoFramePosition=(double(FramePts)/TimeBase);
                        if (StreamPacket->stream_index==VideoStreamNumber) {
                            // Allocate structures
                            if (FrameBufferYUV==NULL) FrameBufferYUV=av_frame_alloc();
                            if (FrameBufferYUV) {
                                LastFFMPEGmessageLevel=0;    // Clear LastFFMPEGmessageLevel : some decoder dont return error but display errors messages !
                                int Error=avcodec_send_packet(VideoDecoderCodecCtx,StreamPacket);
                                if ((Error<0)||(LastFFMPEGmessageLevel==LOGMSG_CRITICAL)) {
                                    if (MaxErrorCount>0) {
                                        if (VideoFramePosition*1000000<Position) {
                                            ToLog(LOGMSG_INFORMATION,QString("IN:cVideoFile::ReadFrame - Error decoding packet: try left %1").arg(MaxErrorCount));
                                        } else {
                                            ToLog(LOGMSG_INFORMATION,QString("IN:cVideoFile::ReadFrame - Error decoding packet: seek to backward and restart reading"));
                                            if (Position>1000000) {
                                                SeekErrorCount=0;
                                                SeekFile(VideoStream,NULL/*AudioStream*/,Position-1000000); // 1 sec before
                                            } else {
                                                SeekErrorCount=0;
                                                SeekFile(VideoStream,NULL,0);
                                            }
                                        }
                                        MaxErrorCount--;
                                    } else {
                                        ToLog(LOGMSG_CRITICAL,QString("IN:cVideoFile::ReadFrame - Error decoding packet: and no try left"));
                                        ContinueVideo=false;
                                    }
                                } else {
                                    Error=avcodec_receive_frame(VideoDecoderCodecCtx,FrameBufferYUV);
                                    if (!Error) {
                                        FrameBufferYUV->pts=FrameBufferYUV->best_effort_timestamp;
    
                                        // Video filter part
                                        if ((Deinterlace)&&(!VideoFilterGraph))           VideoFilter_Open();
                                            else if ((!Deinterlace)&&(VideoFilterGraph))  VideoFilter_Close();
    
                                        
                                        AVFrame *FiltFrame=NULL;
                                        if (VideoFilterGraph) {
                                            // push the decoded frame into the filtergraph
                                            if (av_buffersrc_write_frame(VideoFilterIn,FrameBufferYUV)<0) {
                                                ToLog(LOGMSG_INFORMATION,"IN:cVideoFile::ReadFrame : Error while feeding the filtergraph");
                                            } else {
                                                FiltFrame=av_frame_alloc();
                                                // pull filtered frames from the filtergraph
                                                int ret=av_buffersink_get_frame(VideoFilterOut,FiltFrame);
                                                if ((ret<0)||(ret==AVERROR(EAGAIN))||(ret==AVERROR_EOF)) {
                                                    ToLog(LOGMSG_INFORMATION,"IN:cVideoFile::ReadFrame : No image return by filter process");
                                                    av_frame_unref(FiltFrame);
                                                    av_frame_free(&FiltFrame);
                                                    FiltFrame=NULL;
                                                }
                                            }
                                        }
                                        if (ByPassFirstImage) {
                                            ByPassFirstImage=false;
                                            FreeFrames      =true;
                                        } else {
                                            int64_t pts=FrameBufferYUV->pts;
                                            if (pts==(int64_t)AV_NOPTS_VALUE) {
                                                if (FrameBufferYUV->pkt_dts!=(int64_t)AV_NOPTS_VALUE) {
                                                    pts=FrameBufferYUV->pkt_dts;
                                                    ToLog(LOGMSG_DEBUGTRACE,QString("IN:cVideoFile::ReadFrame : No PTS so use DTS %1 for position %2").arg(pts).arg(Position));
                                                } else {
                                                    pts=0;
                                                    ToLog(LOGMSG_DEBUGTRACE,QString("IN:cVideoFile::ReadFrame : No PTS and no DTS for position %1").arg(Position));
                                                }
                                            }
                                            FrameBufferYUVReady   =true;                                            // Keep actual value for FrameBufferYUV
                                            FrameBufferYUVPosition=int64_t((qreal(pts)*av_q2d(VideoStream->time_base))*AV_TIME_BASE);    // Keep actual value for FrameBufferYUV
                                            // Append this frame
                                            cImageInCache *ObjImage=new cImageInCache(FrameBufferYUVPosition,FiltFrame,FrameBufferYUV);
                                            FreeFrames=false;
                                            int ToIns=0;
                                            while ((ToIns<CacheImage.count())&&(CacheImage[ToIns]->Position<ObjImage->Position)) ToIns++;
                                            if (ToIns<CacheImage.count())
                                                CacheImage.insert(ToIns,ObjImage);
                                            else
                                                CacheImage.append(ObjImage);
                                                
                                            // Count number of image > position
                                            int Nbr=0;
                                            for (int CNbr=0;CNbr<CacheImage.count();CNbr++) if ((CacheImage[CNbr]->Position>=Position)&&(CacheImage[CNbr]->Position-Position<ALLOWEDDELTA)) Nbr++;
                                            IsVideoFind=Nbr>0;
                                        }
                                        if (FreeFrames) {
                                            if (FiltFrame) {
                                                av_frame_unref(FiltFrame);
                                                av_frame_free(&FiltFrame);
                                                av_frame_unref(FrameBufferYUV);
                                            }
                                            av_frame_free(&FrameBufferYUV);
                                        } else {
                                            FrameBufferYUV=NULL;
                                            FiltFrame     =NULL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // Check if we need to continue loop
                    // Note: FPSDuration*(!VideoStream?2:1) is to enhance preview speed
                    ContinueVideo=ContinueVideo && ((VideoStream)&&(!IsVideoFind)&&((VideoFramePosition*1000000<Position)||(VideoFramePosition*1000000-Position<MAXDELTA)));
                }

                // Continue with a new one
                if (StreamPacket!=NULL) {
                    av_packet_unref(StreamPacket); // Free the StreamPacket that was allocated by previous call to av_read_frame
                    delete StreamPacket;
                    StreamPacket=NULL;
                }
            }
        }
        if ((!RetImage)&&(CacheImage.count()>0)) {
            // search nearest image (allowed up to MAXDELTA, after return black frame)
            int i=-1,Nearest=MAXDELTA;
            for (int jj=0;jj<CacheImage.count();jj++) if ((CacheImage[jj]->Position>=Position)&&(CacheImage[jj]->Position-Position<MAXDELTA))
                if ((i==-1)||(CacheImage[jj]->Position-Position<Nearest)) {
                    i=jj;
                    Nearest=CacheImage[jj]->Position-Position;
                }
            if ((i>=0)&&(i<CacheImage.count())/*&&(CacheImage[i]->Position>=Position)&&(CacheImage[i]->Position-Position<100000)*/) {
                RetImage=ConvertYUVToRGB(PreviewMode,CacheImage[i]->FiltFrame?CacheImage[i]->FiltFrame:CacheImage[i]->FrameBufferYUV);
                RetImagePosition=CacheImage[i]->Position,
                ToLog(LOGMSG_DEBUGTRACE,QString("Video image for position %1 => return image at %2").arg(Position).arg(CacheImage[i]->Position));
            } else {
                ToLog(LOGMSG_CRITICAL,QString("No video image return for position %1 => return image at %2").arg(Position).arg(CacheImage[0]->Position));
                RetImage=ConvertYUVToRGB(PreviewMode,CacheImage[0]->FiltFrame?CacheImage[0]->FiltFrame:CacheImage[0]->FrameBufferYUV);
                RetImagePosition=CacheImage[0]->Position;
            }
        }

        if (!RetImage) {
            ToLog(LOGMSG_CRITICAL,QString("No video image return for position %1 => return black frame").arg(Position));
            RetImage =new QImage(FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->width,FFMPEGvideoFile->streams[VideoStreamNumber]->codecpar->height,QImage::Format_ARGB32_Premultiplied);
            RetImage->fill(0);
            RetImagePosition=Position;
        }
        int i=0;
        while (i<CacheImage.count()) {
            if (CacheImage[i]->Position<Position-50000) delete(CacheImage.takeAt(i));
                else i++;
        }
    }
    if ((AudioContext.AudioStream)&&(SoundTrackBloc)&&(CacheImage.count()>0))
        SoundTrackBloc->AdjustSoundPosition(RetImagePosition);

    Mutex.unlock();
    return RetImage;
}

//====================================================================================================================

void cVideoFile::DecodeAudio(sAudioContext *AudioContext,AVPacket *StreamPacket,int64_t Position) {
    qreal FramePts=StreamPacket->pts!=(int64_t)AV_NOPTS_VALUE?StreamPacket->pts*av_q2d(AudioContext->AudioStream->time_base):-1;
    if ((StreamPacket->stream_index==AudioStreamNumber) && (StreamPacket->size>0)) {
        // NOTE: the audio packet can contain several NbrFrames
        int errcode=0;
        while ((AudioContext->Counter>0)&&(AudioContext->ContinueAudio)&&((errcode>=0))) {
            errcode=avcodec_send_packet(AudioDecoderCodecCtx,StreamPacket);
            while (errcode==0) {
                AVFrame *Frame=av_frame_alloc();
                errcode=avcodec_receive_frame(AudioDecoderCodecCtx,Frame);
                if (errcode < 0) {
                    av_frame_free(&Frame);
                    AudioContext->Counter--;
                    //if (AudioContext->Counter==0) ToLog(LOGMSG_CRITICAL,QString("Impossible to decode audio frame: Discard it"));
                } else {
                    DecodeAudioFrame(AudioContext,&FramePts,Frame,Position);
                    Frame=NULL;
                }
            }
        }
    }
    // Continue with a new one
    if (StreamPacket!=NULL) {
        av_packet_unref(StreamPacket); // Free the StreamPacket that was allocated by previous call to av_read_frame
        delete StreamPacket;
        StreamPacket=NULL;
    }
    // Check if we need to continue loop
    AudioContext->ContinueAudio=((AudioContext->ContinueAudio)&&(AudioContext->Counter>0)&&
                                 (AudioContext->AudioStream)&&(AudioContext->SoundTrackBloc)&&((AudioContext->SoundTrackBloc->ListCount()<AudioContext->SoundTrackBloc->NbrPacketForFPS)||
                                 (!((LastAudioReadedPosition>=Position+AudioContext->FPSDuration*AudioContext->NbrDuration)))));
}

//============================

void cVideoFile::DecodeAudioFrame(sAudioContext *AudioContext,qreal *FramePts,AVFrame *Frame,int64_t Position) {
    int64_t  SizeDecoded=0;
    uint8_t *Data      =NULL;
    if ((AudioContext->NeedResampling)&&(RSC!=NULL)) {
        Data=Resample(Frame,&SizeDecoded,AudioContext->DstSampleSize);
    } else {
        Data=Frame->data[0];
        SizeDecoded=Frame->nb_samples*av_get_bytes_per_sample((AVSampleFormat)AudioContext->AudioStream->codecpar->format)*AudioContext->AudioStream->codecpar->ch_layout.nb_channels;
    }
    AudioContext->ContinueAudio=(Data!=NULL);
    if (AudioContext->ContinueAudio) {
        // Adjust FrameDuration with real Nbr Sample
        double FrameDuration=double(SizeDecoded)/(AudioContext->SoundTrackBloc->SamplingRate*AudioContext->DstSampleSize);
        // Adjust pts and inc FramePts int the case there is multiple blocks
        qreal pts=(*FramePts)/av_q2d(AudioContext->AudioStream->time_base);
        if (pts<0) pts=qreal(Position+AudioContext->FPSDuration);
        (*FramePts)+=FrameDuration;
        AudioContext->AudioFramePosition=(*FramePts);
        // Adjust volume if master volume <>1
        double Volume=AudioContext->Volume;
        if (Volume==-1) Volume=GetSoundLevel()!=-1?double(ApplicationConfig->DefaultSoundLevel)/double(GetSoundLevel()*100):1;
        if (Volume!=1) {
            int16_t *Buf1=(int16_t*)Data;
            int32_t mix;
            for (int j=0;j<SizeDecoded/4;j++) {
                // Left channel : Adjust if necessary (16 bits)
                mix=int32_t(double(*(Buf1))*Volume); if (mix>32767)  mix=32767; else if (mix<-32768) mix=-32768;  *(Buf1++)=int16_t(mix);
                // Right channel : Adjust if necessary (16 bits)
                mix=int32_t(double(*(Buf1))*Volume); if (mix>32767)  mix=32767; else if (mix<-32768) mix=-32768;  *(Buf1++)=int16_t(mix);
            }
        }
        // Append decoded data to SoundTrackBloc
        if ((AudioContext->DontUseEndPos)||(AudioContext->AudioFramePosition<*AudioContext->dEndFile)) {
            AudioContext->SoundTrackBloc->AppendData(AudioContext->AudioFramePosition*AV_TIME_BASE,(int16_t*)Data,SizeDecoded);
            AudioContext->AudioLenDecoded   +=SizeDecoded;
            AudioContext->AudioFramePosition =AudioContext->AudioFramePosition+FrameDuration;
        }
    }
    LastAudioReadedPosition =int64_t(AudioContext->AudioFramePosition*AV_TIME_BASE);    // Keep NextPacketPosition for determine next time if we need to seek
    if (Data!=Frame->data[0]) av_free(Data);
    av_frame_free(&Frame);
}

//====================================================================================================================

QImage *cVideoFile::ConvertYUVToRGB(bool PreviewMode,AVFrame *Frame) {
    int W=Frame->width*AspectRatio;    W-=(W%4);   // W must be a multiple of 4 ????
    int H=Frame->height;

    LastImage=QImage(W,H,QTPIXFMT);
    // Allocate structure for RGB image
    AVFrame *FrameBufferRGB=av_frame_alloc();

    if (FrameBufferRGB!=NULL) {

        av_image_fill_arrays(
                FrameBufferRGB->data,               // Buffer to prepare
                FrameBufferRGB->linesize,
                LastImage.bits(),                   // Buffer which will contain the image data
                PIXFMT,                             // The format in which the picture data is stored (see 
                W,                                  // The width of the image in pixels
                H,                                  // The height of the image in pixels
                1
        );

        // Get a converter from libswscale
        struct SwsContext *img_convert_ctx=sws_getContext(
            Frame->width,                                                     // Src width
            Frame->height,                                                    // Src height
            (AVPixelFormat)Frame->format,                                     // Src Format
            W,                                                                // Destination width
            H,                                                                // Destination height
            PIXFMT,                                                           // Destination Format
            SWS_BICUBIC,NULL,NULL,NULL);                                      // flags,src Filter,dst Filter,param

        if (img_convert_ctx!=NULL) {
            int ret = sws_scale(
                img_convert_ctx,                                           // libswscale converter
                Frame->data,                                               // Source buffer
                Frame->linesize,                                           // Source Stride ?
                0,                                                         // Source SliceY:the position in the source image of the slice to process, that is the number (counted starting from zero) in the image of the first row of the slice
                Frame->height,                                             // Source SliceH:the height of the source slice, that is the number of rows in the slice
                FrameBufferRGB->data,                                      // Destination buffer
                FrameBufferRGB->linesize                                   // Destination Stride
            );

            if (ret>0) {
                // Auto crop image if 1088 format
                if ((ApplicationConfig->Crop1088To1080)&&(LastImage.height()==1088)&&(LastImage.width()==1920))  LastImage=LastImage.copy(0,4,1920,1080);
                // Reduce image size for preview mode
                if ((PreviewMode)&&(LastImage.height()>ApplicationConfig->MaxVideoPreviewHeight)) LastImage=LastImage.scaledToHeight(ApplicationConfig->MaxVideoPreviewHeight);
            }
            sws_freeContext(img_convert_ctx);
        }

        // free FrameBufferRGB because we don't need it in the future
        av_frame_free(&FrameBufferRGB);
    }

    //return FinalImage;
    return new QImage(LastImage.copy());
}

//====================================================================================================================
// Load a video frame
// DontUseEndPos default=false
QImage *cVideoFile::ImageAt(bool PreviewMode,int64_t Position,cSoundBlockList *SoundTrackBloc,bool Deinterlace,
                            double Volume,bool DontUseEndPos,int NbrDuration) {

    if (!IsValide) return NULL;
    if (!IsOpen) OpenCodecAndFile();

    if ((PreviewMode)&&(!SoundTrackBloc)) {
        // for speed improvment, try to find image in cache (only for interface)
        cLuLoImageCacheObject *ImageObject=ApplicationConfig->ImagesCache.FindObject(RessourceKey,FileKey,ModifDateTime,ImageOrientation,ApplicationConfig->Smoothing,true);
        if (!ImageObject) return ReadFrame(PreviewMode,Position*1000,DontUseEndPos,Deinterlace,SoundTrackBloc,Volume);

        if ((ImageObject->Position==Position)&&(ImageObject->CachePreviewImage)) return new QImage(ImageObject->CachePreviewImage->copy());

        if (ImageObject->CachePreviewImage) {
            delete ImageObject->CachePreviewImage;
            ImageObject->CachePreviewImage=NULL;
        }
        ImageObject->Position=Position;
        ImageObject->CachePreviewImage=ReadFrame(PreviewMode,Position*1000,DontUseEndPos,Deinterlace,SoundTrackBloc,Volume,NbrDuration);
        if (ImageObject->CachePreviewImage) return new QImage(ImageObject->CachePreviewImage->copy());
            else return NULL;

    } else return ReadFrame(PreviewMode,Position*1000,DontUseEndPos,Deinterlace,SoundTrackBloc,Volume);
}

//====================================================================================================================

int cVideoFile::getThreadFlags(AVCodecID ID) {
    int Ret=0;
    switch (ID) {
        case AV_CODEC_ID_PRORES:
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_DVVIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:   Ret=FF_THREAD_SLICE;                    break;
        case AV_CODEC_ID_H264 :        Ret=FF_THREAD_FRAME|FF_THREAD_SLICE;    break;
        default:                       Ret=FF_THREAD_FRAME;                    break;
    }
    return Ret;
}

//====================================================================================================================

bool cVideoFile::OpenCodecAndFile() {
    // Ensure file was previously checked
    if (!IsValide) return false;
    if (!IsInformationValide) GetFullInformationFromFile();

    // Clean memory if a previous file was loaded
    CloseCodecAndFile();

    //**********************************
    // Open audio stream
    //**********************************
    if (AudioStreamNumber!=-1) {

        // Open the file and get a LibAVFormat context and an associated LibAVCodec decoder
        if (avformat_open_input(&FFMPEGaudioFile,FileName().toLocal8Bit(),NULL,NULL)!=0) return false;
        FFMPEGaudioFile->flags|=AVFMT_FLAG_GENPTS;       // Generate missing pts even if it requires parsing future NbrFrames.
        if (avformat_find_stream_info(FFMPEGaudioFile,NULL)<0) {
            avformat_close_input(&FFMPEGaudioFile);
            return false;
        }

        AVStream *AudioStream=FFMPEGaudioFile->streams[AudioStreamNumber];

        // Setup STREAM options
        AudioStream->discard=AVDISCARD_DEFAULT;

        // Find the decoder for the audio stream and open it
        AudioDecoderCodec=avcodec_find_decoder(AudioStream->codecpar->codec_id);
        AudioDecoderCodecCtx=avcodec_alloc_context3(AudioDecoderCodec);
        avcodec_parameters_to_context(AudioDecoderCodecCtx,AudioStream->codecpar);
        
        // Setup decoder options
        AudioDecoderCodecCtx->debug            =0;                    // Debug level (0=nothing)
        AudioDecoderCodecCtx->workaround_bugs  =1;                    // Work around bugs in encoders which sometimes cannot be detected automatically : 1=autodetection
        AudioDecoderCodecCtx->idct_algo        =FF_IDCT_AUTO;         // IDCT algorithm, 0=auto
        AudioDecoderCodecCtx->skip_frame       =AVDISCARD_DEFAULT;    // ???????
        AudioDecoderCodecCtx->skip_idct        =AVDISCARD_DEFAULT;    // ???????
        AudioDecoderCodecCtx->skip_loop_filter =AVDISCARD_DEFAULT;    // ???????
        AudioDecoderCodecCtx->error_concealment=3;
        AudioDecoderCodecCtx->thread_count     =getCpuCount();
        AudioDecoderCodecCtx->thread_type      =getThreadFlags(AudioStream->codecpar->codec_id);

        if ((AudioDecoderCodec==NULL)||(avcodec_open2(AudioDecoderCodecCtx,AudioDecoderCodec,NULL)<0)) {
            return false;
        }

        IsVorbis=(strcmp(AudioDecoderCodec->name,"vorbis")==0);
    }

    //**********************************
    // Open video stream
    //**********************************
    if ((VideoStreamNumber!=-1)&&(!MusicOnly)) {

        // Open the file and get a LibAVFormat context and an associated LibAVCodec decoder
        if (avformat_open_input(&FFMPEGvideoFile,FileName().toLocal8Bit(),NULL,NULL)!=0) return false;
        FFMPEGvideoFile->flags|=AVFMT_FLAG_GENPTS;       // Generate missing pts even if it requires parsing future NbrFrames.
        if (avformat_find_stream_info(FFMPEGvideoFile,NULL)<0) {
            avformat_close_input(&FFMPEGvideoFile);
            return false;
        }

        AVStream *VideoStream=FFMPEGvideoFile->streams[VideoStreamNumber];

        // Setup STREAM options
        VideoStream->discard=AVDISCARD_DEFAULT;

        // Find the decoder for the video stream and open it
        VideoDecoderCodec=avcodec_find_decoder(VideoStream->codecpar->codec_id);
        VideoDecoderCodecCtx=avcodec_alloc_context3(VideoDecoderCodec);
        avcodec_parameters_to_context(VideoDecoderCodecCtx,VideoStream->codecpar);
        
        // Setup decoder options
        VideoDecoderCodecCtx->debug            =0;                    // Debug level (0=nothing)
        VideoDecoderCodecCtx->workaround_bugs  =1;                    // Work around bugs in encoders which sometimes cannot be detected automatically : 1=autodetection
        VideoDecoderCodecCtx->idct_algo        =FF_IDCT_AUTO;         // IDCT algorithm, 0=auto
        VideoDecoderCodecCtx->skip_frame       =AVDISCARD_DEFAULT;    // ???????
        VideoDecoderCodecCtx->skip_idct        =AVDISCARD_DEFAULT;    // ???????
        VideoDecoderCodecCtx->skip_loop_filter =AVDISCARD_DEFAULT;    // ???????
        VideoDecoderCodecCtx->error_concealment=3;
        VideoDecoderCodecCtx->thread_count     =getCpuCount();
        VideoDecoderCodecCtx->thread_type      =getThreadFlags(VideoStream->codecpar->codec_id);

        // Hack to correct wrong frame rates that seem to be generated by some codecs
        if (VideoDecoderCodecCtx->time_base.num>1000 && VideoDecoderCodecCtx->time_base.den==1)
            VideoDecoderCodecCtx->time_base.den=1000;

        if ((VideoDecoderCodec==NULL)||(avcodec_open2(VideoDecoderCodecCtx,VideoDecoderCodec,NULL)<0)) return false;
        FFMPEGstartTime=FFMPEGvideoFile->start_time;
    }
    IsOpen=true;
    return IsOpen;
}

//*********************************************************************************************************************************************
// Base object for music definition
//*********************************************************************************************************************************************

cMusicObject::cMusicObject(cApplicationConfig *ApplicationConfig):cVideoFile(ApplicationConfig) {
    Volume      =-1;                            // Volume as % from 1% to 150% or -1=auto
    AllowCredit =true;                          // // if true, this music will appear in credit title
    ForceFadIn  =0;
    ForceFadOut =0;
    Reset(OBJECTTYPE_MUSICFILE);
}

//====================================================================================================================
// Overloaded function use to dertermine if format of media file is correct

bool cMusicObject::CheckFormatValide(QWidget *Window) {
    bool IsOk=IsValide;

    // try to open file
    if (!OpenCodecAndFile()) {
        QString ErrorMessage =QApplication::translate("MainWindow","Format not supported","Error message");
        CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
        IsOk=false;
    }

    // check if file have at least one sound track compatible
    if ((IsOk)&&(AudioStreamNumber==-1)) {
        QString ErrorMessage=QApplication::translate("MainWindow","No audio track found","Error message")+"\n";
        CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
        IsOk=false;

    } else {
        if (!((FFMPEGaudioFile->streams[AudioStreamNumber]->codecpar->format!=AV_SAMPLE_FMT_S16)||(FFMPEGaudioFile->streams[AudioStreamNumber]->codecpar->format!=AV_SAMPLE_FMT_U8))) {
            QString ErrorMessage=QApplication::translate("MainWindow","This application support only audio track with unsigned 8 bits or signed 16 bits sample format","Error message")+"\n";
            CustomMessageBox(Window,QMessageBox::Critical,QApplication::translate("MainWindow","Error","Error message"),ShortName()+"\n\n"+ErrorMessage,QMessageBox::Close);
            IsOk=false;
        }
    }
    // close file if it was opened
    CloseCodecAndFile();

    return IsOk;
}

//====================================================================================================================

void cMusicObject::SaveToXML(QDomElement *ParentElement,QString ElementName,QString PathForRelativPath,bool ForceAbsolutPath,cReplaceObjectList *ReplaceList,QList<qlonglong> *,bool) {
    QDomDocument    DomDocument;
    QDomElement     Element=DomDocument.createElement(ElementName);
    QString         TheFileName;

    if (ReplaceList) {
        TheFileName=ReplaceList->GetDestinationFileName(FileName());
    } else if (PathForRelativPath!="") {
        if (ForceAbsolutPath) TheFileName=QDir(QFileInfo(PathForRelativPath).absolutePath()).absoluteFilePath(FileName());
            else TheFileName=QDir(QFileInfo(PathForRelativPath).absolutePath()).relativeFilePath(FileName());
    } else TheFileName=FileName();

    Element.setAttribute("FilePath",     TheFileName);
    Element.setAttribute("iStartPos",    QTime(0,0,0,0).msecsTo(StartPos));
    Element.setAttribute("iEndPos",      QTime(0,0,0,0).msecsTo(EndPos));
    Element.setAttribute("Volume",       QString("%1").arg(Volume,0,'f'));
    Element.setAttribute("AllowCredit",  AllowCredit?"1":"0");
    Element.setAttribute("ForceFadIn",   qlonglong(ForceFadIn));
    Element.setAttribute("ForceFadOut",  qlonglong(ForceFadOut));
    Element.setAttribute("GivenDuration",QTime(0,0,0,0).msecsTo(GetGivenDuration()));
    if (IsComputedAudioDuration) {
        Element.setAttribute("RealAudioDuration",      QTime(0,0,0,0).msecsTo(GetRealAudioDuration()));
        Element.setAttribute("IsComputedAudioDuration",IsComputedAudioDuration?"1":0);
        Element.setAttribute("SoundLevel",             QString("%1").arg(SoundLevel,0,'f'));
    }
    ParentElement->appendChild(Element);
}

//====================================================================================================================

bool cMusicObject::LoadFromXML(QDomElement *ParentElement,QString ElementName,QString PathForRelativPath,QStringList *AliasList,bool *ModifyFlag) {
    if ((ParentElement->elementsByTagName(ElementName).length()>0)&&(ParentElement->elementsByTagName(ElementName).item(0).isElement()==true)) {
        QDomElement Element=ParentElement->elementsByTagName(ElementName).item(0).toElement();

        QString FileName=Element.attribute("FilePath","");
        if ((!QFileInfo(FileName).exists())&&(PathForRelativPath!="")) {
            FileName=QDir::cleanPath(QDir(PathForRelativPath).absoluteFilePath(FileName));
            // Fixes a previous bug in relative path
            if (FileName.startsWith("/..")) {
                if (FileName.contains("/home/")) FileName=FileName.mid(FileName.indexOf("/home/"));
                if (FileName.contains("/mnt/"))  FileName=FileName.mid(FileName.indexOf("/mnt/"));
            }
        }
        if (GetInformationFromFile(FileName,AliasList,ModifyFlag)&&(CheckFormatValide(NULL))) {
            // Old format prior to ffDiaporama 2.2.2014.0308
            if (Element.hasAttribute("StartPos")) StartPos=QTime().fromString(Element.attribute("StartPos"));
            if (Element.hasAttribute("EndPos"))   EndPos  =QTime().fromString(Element.attribute("EndPos"));
            // New format since ffDiaporama 2.2.2014.0308
            if (Element.hasAttribute("iStartPos")) StartPos=QTime(0,0,0,0).addMSecs(Element.attribute("iStartPos").toLongLong());
            if (Element.hasAttribute("iEndPos"))   EndPos  =QTime(0,0,0,0).addMSecs(Element.attribute("iEndPos").toLongLong());

            if (Element.hasAttribute("Volume"))                  Volume=GetDoubleValue(Element,"Volume");
            if (Element.hasAttribute("GivenDuration"))           SetGivenDuration(QTime(0,0,0,0).addMSecs(Element.attribute("GivenDuration").toLongLong()));
            if (Element.hasAttribute("IsComputedDuration"))      IsComputedAudioDuration=Element.attribute("IsComputedDuration")=="1";
            if (Element.hasAttribute("IsComputedAudioDuration")) IsComputedAudioDuration=Element.attribute("IsComputedAudioDuration")=="1";
            if (Element.hasAttribute("RealDuration"))            SetRealAudioDuration(QTime(0,0,0,0).addMSecs(Element.attribute("RealDuration").toLongLong()));
            if (Element.hasAttribute("RealAudioDuration"))       SetRealAudioDuration(QTime(0,0,0,0).addMSecs(Element.attribute("RealAudioDuration").toLongLong()));
            if (Element.hasAttribute("SoundLevel"))              SoundLevel =GetDoubleValue(Element,"SoundLevel");
            if (Element.hasAttribute("AllowCredit"))             AllowCredit=Element.attribute("AllowCredit")=="1";
            if (Element.hasAttribute("ForceFadIn"))              ForceFadIn =Element.attribute("ForceFadIn").toLongLong();
            if (Element.hasAttribute("ForceFadOut"))             ForceFadOut=Element.attribute("ForceFadOut").toLongLong();
            return true;
        } else return false;
    } else return false;
}

//====================================================================================================================

QTime cMusicObject::GetDuration() {
    return EndPos.addMSecs(-QTime(0,0,0,0).msecsTo(StartPos));
}

//====================================================================================================================

qreal cMusicObject::GetFading(int64_t Position,bool SlideHaveFadIn,bool SlideHaveFadOut) {
    int64_t RealFadIN =ForceFadIn;
    int64_t RealFadOUT=ForceFadOut;
    int64_t Duration  =QTime(0,0,0,0).msecsTo(GetDuration());
    qreal   RealVolume=Volume;
    // If fade duration longer than duration, then reduce them
    if (Duration<(RealFadIN+RealFadOUT)) {
        qreal Ratio=qreal(RealFadIN+RealFadOUT)/qreal(Duration);
        RealFadIN =int64_t(qreal(RealFadIN)/Ratio);
        RealFadOUT=int64_t(qreal(RealFadOUT)/Ratio);
    }
    if ((!SlideHaveFadIn)&&(Position<RealFadIN)) {
        qreal PCTDone=ComputePCT(SPEEDWAVE_SINQUARTER,double(Position)/double(RealFadIN));
        RealVolume=RealVolume*PCTDone;
    }
    if ((!SlideHaveFadOut)&&(Position>(Duration-RealFadOUT))) {
        qreal PCTDone=ComputePCT(SPEEDWAVE_SINQUARTER,double(Position-(Duration-RealFadOUT))/double(RealFadOUT));
        RealVolume=RealVolume*(1-PCTDone);
    }
    if (RealVolume==-1) RealVolume=1;
    return RealVolume;
}
