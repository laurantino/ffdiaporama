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

#include "DlgChapter.h"
#include "ui_DlgChapter.h"

DlgChapter::DlgChapter(cDiaporamaObject *DiaporamaObject,cApplicationConfig *ApplicationConfig,QWidget *parent):
    QCustomDialog(ApplicationConfig,parent),ui(new Ui::DlgChapter) {

    ui->setupUi(this);
    OkBt        =ui->OKBT;
    CancelBt    =ui->CancelBt;
    HelpBt      =ui->HelpBt;
    HelpFile    ="0105";
    CurrentSlide=DiaporamaObject;
}

//====================================================================================================================

DlgChapter::~DlgChapter() {
    delete ui;
}

//====================================================================================================================
// Initialise dialog

void DlgChapter::DoInitDialog() {
    ui->ChapterEventDateED->setDisplayFormat(ApplicationConfig->ShortDateFormat);
    ui->ChapterNameED->setText(CurrentSlide->ChapterName);
    ui->ChapterEventDateED->setDate(CurrentSlide->OverrideProjectEventDate?CurrentSlide->ChapterEventDate:CurrentSlide->Parent->ProjectInfo->EventDate);
    ui->ChapterDateED->setPlainText(CurrentSlide->OverrideProjectEventDate?CurrentSlide->OverrideChapterLongDate?CurrentSlide->ChapterLongDate:FormatLongDate(CurrentSlide->ChapterEventDate):CurrentSlide->Parent->ProjectInfo->LongDate);

    RefreshControl();

    connect(ui->StartNewChapterCB,        SIGNAL(stateChanged(int)),this,SLOT(StartNewChapterCBChanged(int)));
    connect(ui->OverrideProjectDateCB,    SIGNAL(stateChanged(int)),this,SLOT(OverrideProjectDateChanged(int)));
    connect(ui->OverrideDateCB,           SIGNAL(stateChanged(int)),this,SLOT(OverrideDateCBChanged(int)));
    connect(ui->ChapterEventDateED,       SIGNAL(dateChanged(const QDate &)),this,SLOT(ChapterEventDateChanged(const QDate &)));
}

//====================================================================================================================
// RefreshControl

void DlgChapter::RefreshControl() {
    ui->StartNewChapterCB->setChecked(CurrentSlide->StartNewChapter);
    ui->OverrideProjectDateCB->setChecked(CurrentSlide->OverrideProjectEventDate);
    ui->OverrideDateCB->setChecked(CurrentSlide->OverrideChapterLongDate);

    ui->ChapterNameLabel->setEnabled(CurrentSlide->StartNewChapter);
    ui->ChapterNameED->setEnabled(CurrentSlide->StartNewChapter);

    ui->OverrideProjectDateCB->setEnabled(CurrentSlide->StartNewChapter);
    ui->ChapterEventDateLabel->setEnabled(CurrentSlide->StartNewChapter && CurrentSlide->OverrideProjectEventDate);
    ui->ChapterEventDateED->setEnabled(CurrentSlide->StartNewChapter && CurrentSlide->OverrideProjectEventDate);

    ui->OverrideDateCB->setEnabled(CurrentSlide->StartNewChapter && CurrentSlide->OverrideProjectEventDate);
    ui->OverrideDateLabel->setEnabled(CurrentSlide->StartNewChapter && CurrentSlide->OverrideProjectEventDate);
    ui->ChapterDateED->setEnabled(CurrentSlide->StartNewChapter && CurrentSlide->OverrideProjectEventDate && CurrentSlide->OverrideChapterLongDate);
}

//====================================================================================================================
// Initiale Undo

void DlgChapter::PrepareGlobalUndo() {
    // Save object before modification for cancel button
    Undo=new QDomDocument(APPLICATION_NAME);
    QDomElement root=Undo->createElement("UNDO-DLG");                                               // Create xml document and root
    CurrentSlide->SaveToXML(root,"UNDO-DLG-OBJECT",CurrentSlide->Parent->ProjectFileName,true,NULL,NULL,false);     // Save object
    Undo->appendChild(root);                                                                        // Add object to xml document
}

//====================================================================================================================
// Apply Undo : call when user click on Cancel button

void DlgChapter::DoGlobalUndo() {
    QDomElement root=Undo->documentElement();
    if (root.tagName()=="UNDO-DLG") CurrentSlide->LoadFromXML(root,"UNDO-DLG-OBJECT","",NULL,NULL,false);
}

//====================================================================================================================

bool DlgChapter::DoAccept() {
    CurrentSlide->StartNewChapter           =ui->StartNewChapterCB->isChecked();

    CurrentSlide->OverrideProjectEventDate  =CurrentSlide->StartNewChapter?ui->OverrideProjectDateCB->isChecked():false;
    CurrentSlide->OverrideChapterLongDate   =CurrentSlide->StartNewChapter?ui->OverrideDateCB->isChecked():false;

    CurrentSlide->ChapterName               =ui->ChapterNameED->text();
    CurrentSlide->ChapterEventDate          =CurrentSlide->OverrideProjectEventDate?ui->ChapterEventDateED->date():CurrentSlide->Parent->ProjectInfo->EventDate;
    CurrentSlide->ChapterLongDate           =CurrentSlide->OverrideProjectEventDate?CurrentSlide->OverrideChapterLongDate?ui->ChapterDateED->toPlainText():FormatLongDate(CurrentSlide->ChapterEventDate):CurrentSlide->Parent->ProjectInfo->LongDate;

    return true;
}

//====================================================================================================================

void DlgChapter::StartNewChapterCBChanged(int) {
    CurrentSlide->StartNewChapter=ui->StartNewChapterCB->isChecked();
    RefreshControl();
}

//====================================================================================================================

void DlgChapter::OverrideProjectDateChanged(int) {
    CurrentSlide->OverrideProjectEventDate=ui->OverrideProjectDateCB->isChecked();
    if (!CurrentSlide->OverrideProjectEventDate) {
        ui->ChapterEventDateED->setDate(CurrentSlide->Parent->ProjectInfo->EventDate);
        CurrentSlide->ChapterLongDate=FormatLongDate(CurrentSlide->Parent->ProjectInfo->EventDate);
        ui->ChapterDateED->setPlainText(CurrentSlide->ChapterLongDate);
    }
    RefreshControl();
}

//====================================================================================================================

void DlgChapter::OverrideDateCBChanged(int) {
    CurrentSlide->OverrideChapterLongDate=ui->OverrideDateCB->isChecked();
    if (!CurrentSlide->OverrideChapterLongDate) {
        CurrentSlide->ChapterLongDate=FormatLongDate(CurrentSlide->ChapterEventDate);
        ui->ChapterDateED->setPlainText(CurrentSlide->ChapterLongDate);
    }
    RefreshControl();
}

//====================================================================================================================

void DlgChapter::ChapterEventDateChanged(const QDate &NewDate) {
    CurrentSlide->ChapterEventDate=NewDate;
    if (!CurrentSlide->OverrideChapterLongDate) {
        CurrentSlide->ChapterLongDate=CurrentSlide->OverrideProjectEventDate?FormatLongDate(CurrentSlide->ChapterEventDate):CurrentSlide->Parent->ProjectInfo->LongDate;
        ui->ChapterDateED->setPlainText(CurrentSlide->ChapterLongDate);
    }
}

//====================================================================================================================
#define FAVACTIONTYPE_ACTIONTYPE    0xF000
#define FAVACTIONTYPE_EDIT          0x1000
#define FAVACTIONTYPE_SELECT        0x8000
