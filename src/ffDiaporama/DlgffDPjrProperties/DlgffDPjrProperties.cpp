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

#include "DlgffDPjrProperties.h"
#include "ui_DlgffDPjrProperties.h"

#include "cTextFrame.h"
#include "cTexteFrameComboBox.h"

#include "DlgSlide/DlgImageComposer.h"

//====================================================================================================================

DlgffDPjrProperties::DlgffDPjrProperties(bool IsPrjCreate,cDiaporama *ffdProject,cApplicationConfig *ApplicationConfig,QWidget *parent)
    :QCustomDialog(ApplicationConfig,parent),ui(new Ui::DlgffDPjrProperties) {

    this->IsPrjCreate=IsPrjCreate;
    this->ffdProject =ffdProject;

    ui->setupUi(this);
    CancelBt=ui->CancelBt;
    OkBt    =ui->OkBt;
    HelpBt  =ui->HelpBt;
    HelpFile="0108";

    ui->ThumbCB->Diaporama=ffdProject;
}

//====================================================================================================================
// Initialise dialog

void DlgffDPjrProperties::DoInitDialog() {
    if (IsPrjCreate) {
        setWindowTitle(QApplication::translate("DlgffDPjrProperties","New project"));
        OkBt->setText(QApplication::translate("DlgffDPjrProperties","Create project"));
    } else {
        setWindowTitle(QApplication::translate("DlgffDPjrProperties","Project properties"));
    }
    ui->GeometryCombo->setCurrentIndex(ffdProject->ImageGeometry);
    ui->GeometryCombo->setEnabled(IsPrjCreate);
    ui->TitleED->setText(ffdProject->ProjectInfo->Title);
    ui->DateED->setPlainText(ffdProject->ProjectInfo->LongDate);
    ui->DateEdit->setDate(ffdProject->ProjectInfo->EventDate);
    ui->DateEdit->setDisplayFormat(ApplicationConfig->ShortDateFormat);
    ui->OverrideDateCB->setChecked(ffdProject->ProjectInfo->OverrideDate);
    ui->DateED->setEnabled(ui->OverrideDateCB->isChecked());
    ui->AuthorED->setText(ffdProject->ProjectInfo->Author);
    ui->AlbumED->setText(ffdProject->ProjectInfo->Album);
    ui->CommentED->setPlainText(ffdProject->ProjectInfo->Comment);
    ui->LanguageED->setText(ffdProject->ProjectInfo->DefaultLanguage);
    ui->TransitionSpeedWaveCB->SetCurrentValue(ffdProject->TransitionSpeedWave);
    ui->BlockSpeedWaveCB->SetCurrentValue(ffdProject->BlockAnimSpeedWave);
    ui->ImageSpeedWaveCB->SetCurrentValue(ffdProject->ImageAnimSpeedWave);
    if (ApplicationConfig->ID3V2Comptatibility) {
        ui->TitleED->setMaxLength(30);
        ui->AuthorED->setMaxLength(30);
        ui->AlbumED->setMaxLength(30);
    }
    DoInitTableWidget(ui->ChapterTable,QApplication::translate("DlgffDPjrProperties","#;Slide;Title;Start;End;Duration","Column headers"));
    for (int i=0;(i==0)||(i<ffdProject->ProjectInfo->NbrChapters);i++) {
        int     iChapterNum=ffdProject->ProjectInfo->NbrChapters==0?0:i+1;
        QString ChapterNum=QString("%1").arg(iChapterNum); while (ChapterNum.length()<3) ChapterNum="0"+ChapterNum;
        ChapterNum="Chapter_"+ChapterNum+":";
        QColor Background=((i & 0x01)==0x01)?Qt::white:QColor(0xE0,0xE0,0xE0);
        ui->ChapterTable->insertRow(ui->ChapterTable->rowCount());
        ui->ChapterTable->setItem(ui->ChapterTable->rowCount()-1,0,CreateItem(QString("%1").arg(iChapterNum),Qt::AlignLeft|Qt::AlignVCenter,Background));
        ui->ChapterTable->setItem(ui->ChapterTable->rowCount()-1,1,CreateItem(GetInformationValue(ChapterNum+"InSlide", &ffdProject->ProjectInfo->ChaptersProperties),Qt::AlignLeft|Qt::AlignVCenter,Background));
        ui->ChapterTable->setItem(ui->ChapterTable->rowCount()-1,2,CreateItem(GetInformationValue(ChapterNum+"title",   &ffdProject->ProjectInfo->ChaptersProperties),Qt::AlignLeft|Qt::AlignVCenter,Background));
        ui->ChapterTable->setItem(ui->ChapterTable->rowCount()-1,3,CreateItem(GetInformationValue(ChapterNum+"Start",   &ffdProject->ProjectInfo->ChaptersProperties),Qt::AlignLeft|Qt::AlignVCenter,Background));
        ui->ChapterTable->setItem(ui->ChapterTable->rowCount()-1,4,CreateItem(GetInformationValue(ChapterNum+"End",     &ffdProject->ProjectInfo->ChaptersProperties),Qt::AlignLeft|Qt::AlignVCenter,Background));
        ui->ChapterTable->setItem(ui->ChapterTable->rowCount()-1,5,CreateItem(GetInformationValue(ChapterNum+"Duration",&ffdProject->ProjectInfo->ChaptersProperties),Qt::AlignLeft|Qt::AlignVCenter,Background));
    }
    DoResizeColumnsTableWidget(ui->ChapterTable);

    if (ffdProject->ThumbnailName=="*") ApplicationConfig->ThumbnailModels->AppendCustomModel();
    ui->ThumbCB->ProjectThumbnail=ffdProject->ProjectThumbnail;
    ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
    ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);

    // Define handler
    connect(ui->DateEdit,SIGNAL(dateChanged(const QDate &)),this,SLOT(EventDateChanged(const QDate &)));
    connect(ui->OverrideDateCB,SIGNAL(stateChanged(int)),this,SLOT(OverrideDateChanged(int)));
    connect(ui->AdminEditThumbBT,SIGNAL(clicked()),this,SLOT(AdminEditThumb()));
    connect(ui->EditThumbBT,SIGNAL(clicked()),this,SLOT(EditThumb()));
    connect(ui->ExportThumbBT,SIGNAL(clicked()),this,SLOT(ExportThumb()));
    connect(ui->ThumbCB,SIGNAL(itemSelectionHaveChanged()),this,SLOT(ThumbChanged()));
    connect(ui->TitleED,SIGNAL(textChanged(QString)),this,SLOT(TitleChanged(QString)));
    connect(ui->AlbumED,SIGNAL(textChanged(QString)),this,SLOT(AlbumChanged(QString)));
    connect(ui->AuthorED,SIGNAL(textChanged(QString)),this,SLOT(AuthorChanged(QString)));

    ui->ThumbCB->MakeIcons();    // Refresh icons for curstom combo
    ThumbChanged();
}

//====================================================================================================================

DlgffDPjrProperties::~DlgffDPjrProperties() {
    ffdProject->CloseUnusedFFMPEG(ffdProject->CurrentCol);
    delete ui;
}

//====================================================================================================================
// Initiale Undo

void DlgffDPjrProperties::PrepareGlobalUndo() {
    // Save object before modification for cancel button
    Undo=new QDomDocument(APPLICATION_NAME);
    QDomElement root=Undo->createElement("UNDO-DLG"); // Create xml document and root
    ffdProject->ProjectInfo->SaveToXML(&root);
    ffdProject->ProjectThumbnail->SaveToXML(root,"UNDO-DLG-ProjectThumbnail",ffdProject->ProjectFileName,true,NULL,NULL,false);
    Undo->appendChild(root); // Add object to xml document
}

//====================================================================================================================
// Apply Undo : call when user click on Cancel button

void DlgffDPjrProperties::DoGlobalUndo() {
    QDomElement root=Undo->documentElement();
    if (root.tagName()=="UNDO-DLG") {
        ffdProject->ProjectInfo->LoadFromXML(&root);
        ffdProject->ProjectThumbnail->LoadFromXML(root,"UNDO-DLG-ProjectThumbnail","",NULL,NULL,false);
    }
}

//====================================================================================================================
// Call when user click on Ok button

bool DlgffDPjrProperties::DoAccept() {
    if (IsPrjCreate) ffdProject->ImageGeometry=(ffd_GEOMETRY)ui->GeometryCombo->currentIndex();
    ffdProject->ProjectInfo->Album          =ui->AlbumED->text();
    ffdProject->ProjectInfo->Author         =ui->AuthorED->text();
    ffdProject->ProjectInfo->LongDate       =ui->DateED->toPlainText();
    ffdProject->ProjectInfo->EventDate      =ui->DateEdit->date();
    ffdProject->ProjectInfo->OverrideDate   =ui->OverrideDateCB->isChecked();
    ffdProject->ProjectInfo->Comment        =ui->CommentED->toPlainText();
    ffdProject->ProjectInfo->DefaultLanguage=ui->LanguageED->text();
    ffdProject->TransitionSpeedWave         =ui->TransitionSpeedWaveCB->GetCurrentValue();
    ffdProject->BlockAnimSpeedWave          =ui->BlockSpeedWaveCB->GetCurrentValue();
    ffdProject->ImageAnimSpeedWave          =ui->ImageSpeedWaveCB->GetCurrentValue();
    ffdProject->ThumbnailName               =ui->ThumbCB->GetCurrentModel();

    return true;
}

//====================================================================================================================

void DlgffDPjrProperties::TitleChanged(QString) {
    ffdProject->ProjectInfo->Title=ui->TitleED->text();
    ui->ThumbCB->MakeIcons();    // Refresh icons for curstom combo
}

void DlgffDPjrProperties::AlbumChanged(QString) {
    ffdProject->ProjectInfo->Album=ui->AlbumED->text();
    ui->ThumbCB->MakeIcons();    // Refresh icons for curstom combo
}

void DlgffDPjrProperties::AuthorChanged(QString) {
    ffdProject->ProjectInfo->Author=ui->AuthorED->text();
    ui->ThumbCB->MakeIcons();    // Refresh icons for curstom combo
}

//====================================================================================================================

void DlgffDPjrProperties::EventDateChanged(const QDate &NewDate) {
    ffdProject->ProjectInfo->EventDate=NewDate;
    if (!ffdProject->ProjectInfo->OverrideDate) {
        ffdProject->ProjectInfo->LongDate=FormatLongDate(ffdProject->ProjectInfo->EventDate);
        ui->DateED->setPlainText(ffdProject->ProjectInfo->LongDate);
    }
    ui->ThumbCB->MakeIcons();    // Refresh icons for curstom combo
}

//====================================================================================================================

void DlgffDPjrProperties::OverrideDateChanged(int) {
    ffdProject->ProjectInfo->OverrideDate=ui->OverrideDateCB->isChecked();
    if (!ffdProject->ProjectInfo->OverrideDate) {
        ffdProject->ProjectInfo->LongDate=FormatLongDate(ffdProject->ProjectInfo->EventDate);
        ui->DateED->setPlainText(ffdProject->ProjectInfo->LongDate);
    }
    ui->DateED->setEnabled(ui->OverrideDateCB->isChecked());
    ui->ThumbCB->MakeIcons();    // Refresh icons for curstom combo
}

//====================================================================================================================
// Export Thumb

void DlgffDPjrProperties::ExportThumb() {
    QString OutputFileName=QFileInfo(ffdProject->ProjectFileName).absolutePath();
    if (!OutputFileName.endsWith(QDir::separator())) OutputFileName=OutputFileName+QDir::separator();
    OutputFileName=OutputFileName+"folder.jpg";
    QString Filter="JPG (*.jpg)";
    OutputFileName=QFileDialog::getSaveFileName(this,QApplication::translate("MainWindow","Select destination file"),OutputFileName,"JPG (*.jpg)",&Filter);
    if (OutputFileName!="") {
        int Index=ApplicationConfig->ThumbnailModels->SearchModel(ffdProject->ThumbnailName);
        if (Index>=0) {
            QSize  ForcedThumbnailSize(THUMBWITH,THUMBHEIGHT);
            QImage Image=ApplicationConfig->ThumbnailModels->List[Index]->PrepareImage(0,ffdProject,ffdProject->ProjectThumbnail,&ForcedThumbnailSize);
            if (Image.save(OutputFileName,0,100))
                CustomMessageBox(this,QMessageBox::Information,QApplication::translate("DlgffDPjrProperties","Export thumbnail"),
                                 QApplication::translate("DlgffDPjrProperties","Thumbnail successfully exported","Information message"),QMessageBox::Close);
        }
    }
}

//====================================================================================================================
// Edit a custom thumb model

void DlgffDPjrProperties::AdminEditThumb() {
    int ThumbnailIndex=ApplicationConfig->ThumbnailModels->SearchModel(ffdProject->ThumbnailName);
    if ((ThumbnailIndex<0)||(ThumbnailIndex>=ApplicationConfig->ThumbnailModels->List.count())) return;
    if (!ApplicationConfig->ThumbnailModels->List[ThumbnailIndex]->IsCustom) {
        QString OldName=ffdProject->ThumbnailName;
        QString NewName=ffdProject->ProjectThumbnail->SaveAsNewCustomModelFile(ffd_MODELTYPE_THUMBNAIL);
        ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
        ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
        DlgImageComposer Dlg(ffdProject,ApplicationConfig,this);
        Dlg.InitDialog();
        if (Dlg.exec()==0) {
            ffdProject->ProjectThumbnail->SaveModelFile(ffd_MODELTYPE_THUMBNAIL,NewName);
            ApplicationConfig->ThumbnailModels->FillModelType(ffd_MODELTYPE_THUMBNAIL);
            ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
            ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
        } else {
            QFile(NewName).remove();
            (*ApplicationConfig->ThumbnailModels->NextNumber)--;
            ffdProject->ThumbnailName=OldName;
            ApplicationConfig->ThumbnailModels->FillModelType(ffd_MODELTYPE_THUMBNAIL);
            ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
            ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
        }
    } else {
        DlgImageComposer Dlg(ffdProject,ApplicationConfig,this);
        Dlg.InitDialog();
        if (Dlg.exec()==0) {
            ffdProject->ProjectThumbnail->SaveModelFile(ffd_MODELTYPE_THUMBNAIL,ApplicationConfig->ThumbnailModels->List[ApplicationConfig->ThumbnailModels->SearchModel(ffdProject->ThumbnailName)]->FileName);
            ApplicationConfig->ThumbnailModels->FillModelType(ffd_MODELTYPE_THUMBNAIL);
            ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
            ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
        }
    }
    ThumbChanged();
}

//====================================================================================================================
// Edit customized thumb (thumb integrated in the project)

void DlgffDPjrProperties::EditThumb() {
    if (ffdProject->ThumbnailName!="*") {
        ffdProject->ThumbnailName="*";
        ApplicationConfig->ThumbnailModels->AppendCustomModel();
        ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
        ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
    }
    DlgImageComposer Dlg(ffdProject,ApplicationConfig,this);
    Dlg.InitDialog();
    if (Dlg.exec()==0) {
        ApplicationConfig->ThumbnailModels->AppendCustomModel();
        ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
        ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
    }
    ThumbChanged();
}

//====================================================================================================================

void DlgffDPjrProperties::ThumbChanged() {
    ffdProject->ThumbnailName=ui->ThumbCB->GetCurrentModel();
    int ThumbnailIndex=ApplicationConfig->ThumbnailModels->SearchModel(ffdProject->ThumbnailName);
    if ((ThumbnailIndex<0)||(ThumbnailIndex>=ApplicationConfig->ThumbnailModels->List.count())) return;

    if (ffdProject->ThumbnailName!="*") {
        ApplicationConfig->ThumbnailModels->RemoveCustomModel();
        if ((ThumbnailIndex>=0)&&(ThumbnailIndex<ApplicationConfig->ThumbnailModels->List.count()))
            ffdProject->ProjectThumbnail->LoadModelFromXMLData(ffd_MODELTYPE_THUMBNAIL,ApplicationConfig->ThumbnailModels->List[ThumbnailIndex]->Model,
                                                               &ApplicationConfig->ThumbnailModels->List[ThumbnailIndex]->ResKeyList,true);
        ui->ThumbCB->PrepareTable(true,ApplicationConfig->ThumbnailModels);
        ui->ThumbCB->SetCurrentModel(ffdProject->ThumbnailName);
    }
    if ((ThumbnailIndex<0)||(ThumbnailIndex>=ApplicationConfig->ThumbnailModels->List.count())) return;
    if (ApplicationConfig->ThumbnailModels->List[ThumbnailIndex]->IsCustom) {
        ui->AdminEditThumbBT->setText(QApplication::translate("DlgffDPjrProperties","Edit\ncustom model"));
        ui->AdminEditThumbBT->setEnabled(ApplicationConfig->ThumbnailModels->List[ThumbnailIndex]->Name!="*");
    } else {
        ui->AdminEditThumbBT->setText(QApplication::translate("DlgffDPjrProperties","Create\ncustom model"));
        ui->AdminEditThumbBT->setEnabled(true);
    }
}

//====================================================================================================================
#define FAVACTIONTYPE_ACTIONTYPE    0xF000
#define FAVACTIONTYPE_EDIT          0x1000
#define FAVACTIONTYPE_SELECT        0x8000

//====================================================================================================================
