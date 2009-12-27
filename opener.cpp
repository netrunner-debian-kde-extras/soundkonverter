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
#include "opener.h"
#include "options.h"
#include "config.h"

#include <KLocale>
#include <KPushButton>
#include <QLabel>
#include <QLayout>
#include <KMessageBox>
#include <QTextStream>

#include <kfilewidget.h>
#include <KUrlRequester>
#include <QDir>


// TODO enable proceed button only if at least one file got selected

// TODO soundkonverter 1.1: xspf support

Opener::Opener( Config *_config, OpenMode _openMode, QWidget *parent, Qt::WFlags f )
    : KDialog( parent, f ),
    config( _config ),
    openMode( _openMode )
{
    if( openMode == Files ) setCaption( i18n("Add Files") );
    else if( openMode == Url ) setCaption( i18n("Add Url") );
    else if( openMode == Playlist ) setCaption( i18n("Add Playlist") );
    setWindowIcon( KIcon("audio-x-generic") );
    setButtons( 0 );
    
    page = FileOpenPage;

    QWidget *widget = new QWidget();
    setMainWidget( widget );

    QGridLayout *mainGrid = new QGridLayout( widget );
    QGridLayout *topGrid = new QGridLayout( widget );
    mainGrid->addLayout( topGrid, 0, 0 );

    if( openMode == Files ) lSelector = new QLabel( i18n("1. Select files"), widget );
    else if( openMode == Url ) lSelector = new QLabel( i18n("1. Enter url"), widget );
    else if( openMode == Playlist ) lSelector = new QLabel( i18n("1. Select playlist"), widget );
    QFont font;
    font.setBold( true );
    lSelector->setFont( font );
    topGrid->addWidget( lSelector, 0, 0 );
    lOptions = new QLabel( i18n("2. Set conversion options"), widget );
    topGrid->addWidget( lOptions, 0, 1 );

    // draw a horizontal line
    QFrame *lineFrame = new QFrame( widget );
    lineFrame->setFrameShape( QFrame::HLine );
    lineFrame->setFrameShadow( QFrame::Sunken );
    mainGrid->addWidget( lineFrame, 1, 0 );

    fileWidget = 0;
    urlRequester = 0;
    
    if( openMode == Files )
    {
        fileWidget = new KFileWidget( KUrl(QDir::homePath()), widget );
        fileWidget->setMode( KFile::Files | KFile::ExistingOnly );
        fileWidget->setOperationMode( KAbstractFileWidget::Opening );
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
        fileWidget->setFilter( filterList.join("\n") );
        mainGrid->addWidget( fileWidget, 2, 0 );
    }
    else if( openMode == Url )
    {
        QVBoxLayout *urlBox = new QVBoxLayout( 0 );
        mainGrid->addLayout( urlBox, 2, 0 );
        urlBox->addSpacing( 60 );
        urlRequester = new KUrlRequester( widget );
        urlBox->addWidget( urlRequester );
        urlBox->addStretch();
    }
    else if( openMode == Playlist )
    {
        fileWidget = new KFileWidget( KUrl(QDir::homePath()), widget );
        fileWidget->setMode( KFile::File | KFile::ExistingOnly );
        fileWidget->setOperationMode( KAbstractFileWidget::Opening );
        fileWidget->setFilter( "*.m3u" );
        mainGrid->addWidget( fileWidget, 2, 0 );
    }

    options = new Options( config, i18n("Select your desired output options and click on \"Ok\"."), widget );
    mainGrid->addWidget( options, 2, 0 );
    adjustSize();
    options->hide();

    
    // add a horizontal box layout for the control elements
    QHBoxLayout *controlBox = new QHBoxLayout( 0 );
    mainGrid->addLayout( controlBox, 5, 0 );

    // add the control elements
    if( openMode == Files )
    {
        formatHelp = new QLabel( i18n("<a href=\"format-help\">Are you missing some file formats?</a>"), widget );
        controlBox->addWidget( formatHelp );
        connect( formatHelp, SIGNAL(linkActivated(const QString&)), this, SLOT(showHelp()) );
    }
    else
    {
        formatHelp = 0;
    }
    controlBox->addStretch();

    pProceed = new KPushButton( KIcon("go-next"), i18n("Proceed"), widget );
    controlBox->addWidget( pProceed );
    connect( pProceed, SIGNAL(clicked()), this, SLOT(proceedClickedSlot()) );
    pAdd = new KPushButton( KIcon("dialog-ok"), i18n("Ok"), widget );
    controlBox->addWidget( pAdd );
    pAdd->hide();
    connect( pAdd, SIGNAL(clicked()), this, SLOT(okClickedSlot()) );
    pCancel = new KPushButton( KIcon("dialog-cancel"), i18n("Cancel"), widget );
    controlBox->addWidget( pCancel );
    connect( pCancel, SIGNAL(clicked()), this, SLOT(reject()) );
    
    QLabel *bugText = new QLabel( "Unfortunately KFileWidget seems to contain a bug. To select a file, don't just select it, but click on it, deselecting won't work.\nI'll redesign this dialog for the final version but until then you can vote at: https://bugs.kde.org/show_bug.cgi?id=215321", this );
    mainGrid->addWidget( bugText, 6, 0 );
}


Opener::~Opener()
{}

void Opener::proceedClickedSlot()
{
    QString codecName;
    QStringList errorList;
    QMap< QString, QList<QStringList> > problems;
    QStringList messageList;
    QString fileName;
    QStringList affectedFiles;
    QStringList filesNotFound;
  
    if( page == FileOpenPage )
    {
        if( openMode == Files || openMode == Playlist )
        {
            urls.clear();
            fileWidget->accept();
            if( openMode == Files )
            {
                urls = fileWidget->selectedUrls();
            }
            else
            {
                KUrl playlistUrl = fileWidget->selectedUrl();
                QFile playlistFile( playlistUrl.toLocalFile() );
                if( playlistFile.open(QIODevice::ReadOnly) )
                {
                    QTextStream stream(&playlistFile);
                    QString line;
                    do
                    {
                        line = stream.readLine();
                        if( !line.startsWith("#EXTM3U") && !line.startsWith("#EXTINF") && !line.isEmpty() )
                        {
                            KUrl url(line);
                            if( url.isRelative() ) url = KUrl( playlistUrl.directory() + "/" + line );
                            url.cleanPath();

                            if( !url.isLocalFile() || QFile::exists(url.toLocalFile()) ) urls += url;
                            else filesNotFound += url.pathOrUrl();
                        }
                    } while( !line.isNull() );
                    playlistFile.close();
                }
            }

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
                    messageList += "<b>Possible solutions for " + codecName + "</b>:\n" + problems.value(codecName).at(1).join("\n") + i18n("\nAffected files:\n") + affectedFiles.join("\n");
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

            if( urls.count() <= 0 ) return;

            fileWidget->hide();
            options->show();
            page = ConversionOptionsPage;
            QFont font;
            font.setBold( false );
            lSelector->setFont( font );
            font.setBold( true );
            lOptions->setFont( font );
            if( formatHelp ) formatHelp->hide();
            pProceed->hide();
            pAdd->show();
        }
        else if( openMode == Url )
        {
            if( !urlRequester->url().isValid() )
            {
                KMessageBox::information( this, i18n("The Url you entered is invalid. Please try again.") );
                return;
            }
            
            urls = urlRequester->url();
          
            urlRequester->hide();
            options->show();
            page = ConversionOptionsPage;
            QFont font;
            font.setBold( false );
            lSelector->setFont( font );
            font.setBold( true );
            lOptions->setFont( font );
            pProceed->hide();
            pAdd->show();
        }
    }
}

void Opener::okClickedSlot()
{
    if( page == ConversionOptionsPage )
    {
        emit done( urls, options->currentConversionOptions() );
    }
    accept();
}

void Opener::showHelp()
{
    QStringList messageList;
    QString codecName;
    
    QMap<QString,QStringList> problems = config->pluginLoader()->decodeProblems();
    for( int i=0; i<problems.count(); i++ )
    {
        codecName = problems.keys().at(i);
        if( codecName != "wav" )
        {
            messageList += "<b>Possible solutions for " + codecName + "</b>:\n" + problems.value(codecName).join("\n");
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
