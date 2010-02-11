//
// C++ Implementation: opener
//
// Description: 
//
//
// Author: Daniel Faust <hessijames@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fileopener.h"
#include "../options.h"
#include "../config.h"

#include <KLocale>
#include <KPushButton>
#include <QLabel>
#include <QLayout>
#include <KMessageBox>
#include <KFileDialog>
#include <QDir>


FileOpener::FileOpener( Config *_config, QWidget *parent, Qt::WFlags f )
    : KDialog( parent, f ),
    config( _config )
{
    setCaption( i18n("Add Files") );
    setWindowIcon( KIcon("audio-x-generic") );
    setButtons( 0 );
    
    QWidget *widget = new QWidget();
    setMainWidget( widget );

    QGridLayout *mainGrid = new QGridLayout( widget );

    QStringList filterList;
    QStringList allFilter;
    QStringList formats = config->pluginLoader()->formatList( PluginLoader::Decode, PluginLoader::CompressionType(PluginLoader::Lossy|PluginLoader::Lossless|PluginLoader::Hybrid) );
    for( int i=0; i<formats.count(); i++ )
    {
        QString extensionFilter = config->pluginLoader()->codecExtensions(formats.at(i)).join(" *.");
        if( extensionFilter.length() == 0 ) continue;
        extensionFilter = "*." + extensionFilter;
        allFilter += extensionFilter;
        filterList += extensionFilter + "|" + i18n("%1 files",formats.at(i));
    }
    filterList.prepend( allFilter.join(" ") + "|" + i18n("All supported files") );
    filterList += "*.*|" + i18n("All files");
    
    options = new Options( config, i18n("Select your desired output options and click on \"Ok\"."), widget );
    mainGrid->addWidget( options, 1, 0 );
    
    // add a horizontal box layout for the control elements
    QHBoxLayout *controlBox = new QHBoxLayout( 0 );
    mainGrid->addLayout( controlBox, 2, 0 );
    controlBox->addStretch();

    pAdd = new KPushButton( KIcon("dialog-ok"), i18n("Ok"), widget );
    controlBox->addWidget( pAdd );
    connect( pAdd, SIGNAL(clicked()), this, SLOT(okClickedSlot()) );
    pCancel = new KPushButton( KIcon("dialog-cancel"), i18n("Cancel"), widget );
    controlBox->addWidget( pCancel );
    connect( pCancel, SIGNAL(clicked()), this, SLOT(reject()) );
    
    // add the control elements
    formatHelp = new QLabel( i18n("<a href=\"format-help\">Are you missing some file formats?</a>"), widget );
    connect( formatHelp, SIGNAL(linkActivated(const QString&)), this, SLOT(showHelp()) );

    fileDialog = new KFileDialog( KUrl(QDir::homePath()), filterList.join("\n"), this, formatHelp );
    fileDialog->setWindowTitle( i18n("Add Files") );
    fileDialog->setMode( KFile::Files | KFile::ExistingOnly );
    connect( fileDialog, SIGNAL(accepted()), this, SLOT(fileDialogAccepted()) );
    connect( fileDialog, SIGNAL(rejected()), this, SLOT(reject()) );
    fileDialog->show();
}

FileOpener::~FileOpener()
{}

void FileOpener::fileDialogAccepted()
{
    QString codecName;
    QStringList errorList;
    QMap< QString, QList<QStringList> > problems;
    QStringList messageList;
    QString fileName;
    QStringList affectedFiles;
    QStringList filesNotFound;
  
    urls.clear();
    urls = fileDialog->selectedUrls();

    for( int i=0; i<urls.count(); i++ )
    {
        codecName = config->pluginLoader()->getCodecFromFile( urls.at(i) );

        if( !config->pluginLoader()->canDecode(codecName,&errorList) )
        {
            fileName = urls.at(i).pathOrUrl();
            if( codecName.isEmpty() ) codecName = fileName.right(fileName.length()-fileName.lastIndexOf(".")-1);
            if( problems.value(codecName).count() < 2 )
            {
                problems[codecName] += QStringList();
                problems[codecName] += QStringList();
            }
            problems[codecName][0] += fileName;
            if( !errorList.isEmpty() )
            {
                problems[codecName][1] += errorList;
            }
            else
            {
                problems[codecName][1] += i18n("This file type is unknown to soundKonverter.\nMaybe you need to install an additional soundKonverter plugin.\nYou should have a look at your distribution's package manager for this.");
            }
            urls.removeAt(i);
            i--;
        }
    }

    for( int i=0; i<problems.count(); i++ )
    {
        codecName = problems.keys().at(i);
        if( codecName != "wav" )
        {
            problems[codecName][1].removeDuplicates();
            affectedFiles.clear();
            if( problems.value(codecName).at(0).count() <= 3 )
            {
                affectedFiles = problems.value(codecName).at(0);
            }
            else
            {
                affectedFiles += problems.value(codecName).at(0).at(0);
                affectedFiles += problems.value(codecName).at(0).at(1);
                affectedFiles += i18n("... and %1 more files",problems.value(codecName).at(0).count()-2);
            }
            messageList += "<b>Possible solutions for " + codecName + "</b>:\n" + problems.value(codecName).at(1).join("\n<b>or</b>\n") + i18n("\n\nAffected files:\n") + affectedFiles.join("\n");
        }
    }
    
    if( !messageList.isEmpty() )
    {
        messageList.prepend( i18n("Some files can't be decoded.\nPossible solutions are listed below.") );
        QMessageBox *messageBox = new QMessageBox( this );
        messageBox->setIcon( QMessageBox::Information );
        messageBox->setWindowTitle( i18n("Missing backends") );
        messageBox->setText( messageList.join("\n\n").replace("\n","<br>") );
        messageBox->setTextFormat( Qt::RichText );
        messageBox->exec();
    }
    
    if( !filesNotFound.isEmpty() )
    {
        int filesNotFoundCount = filesNotFound.count();
        if( filesNotFoundCount > 5 )
        {
            do {
                filesNotFound.removeLast();
            } while( filesNotFound.count() >= 5 );
            filesNotFound += i18n("... and %1 more files",filesNotFoundCount-4);
        }
        filesNotFound.prepend( i18n("The following files couldn't be found:\n") );
        QMessageBox *messageBox = new QMessageBox( this );
        messageBox->setIcon( QMessageBox::Information );
        messageBox->setWindowTitle( i18n("Files not found") );
        messageBox->setText( filesNotFound.join("\n").replace("\n","<br>") );
        messageBox->setTextFormat( Qt::RichText );
        messageBox->exec();
    }

    if( urls.count() <= 0 ) reject();
}

void FileOpener::okClickedSlot()
{
    emit done( urls, options->currentConversionOptions() );
    accept();
}

void FileOpener::showHelp()
{
    QStringList messageList;
    QString codecName;
    
    QMap<QString,QStringList> problems = config->pluginLoader()->decodeProblems();
    for( int i=0; i<problems.count(); i++ )
    {
        codecName = problems.keys().at(i);
        if( codecName != "wav" )
        {
            messageList += "<b>Possible solutions for " + codecName + "</b>:\n" + problems.value(codecName).join("\n<b>or</b>\n");
        }
    }
    
    if( messageList.isEmpty() )
    {
        messageList += i18n("soundKonverter couldn't find any missing packages.\nMaybe you need to install an additional plugin via the packagemanager of your distribution.");
    }
    else
    {
        messageList.prepend( i18n("Some of the installed plugins aren't working because they are missing additional programs.\nPossible solutions are listed below.") );
    }
    
    QMessageBox *messageBox = new QMessageBox( this );
    messageBox->setIcon( QMessageBox::Information );
    messageBox->setWindowTitle( i18n("Missing backends") );
    messageBox->setText( messageList.join("\n\n").replace("\n","<br>") );
    messageBox->setTextFormat( Qt::RichText );
    messageBox->exec();
}
