/*
 * soundkonverterview.cpp
 *
 * Copyright (C) 2007 Daniel Faust <hessijames@gmail.com>
 */
#include "soundkonverterview.h"
#include "filelist.h"
#include "filelistitem.h"
#include "combobutton.h"
#include "progressindicator.h"
#include "optionslayer.h"
#include "config.h"
#include "opener/fileopener.h"
#include "opener/diropener.h"
#include "opener/cdopener.h"
#include "opener/urlopener.h"
#include "opener/playlistopener.h"
#include "convert.h"
#include "audiocd/cdmanager.h"
#include "options.h"

#include <KLocale>
#include <KPushButton>
#include <KIcon>
#include <KFileDialog>
#include <KMenu>
#include <KAction>
#include <KActionMenu>

#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QTreeView>
#include <QToolButton>
// #include <QMessageBox>
#include <KMessageBox>

soundKonverterView::soundKonverterView( Logger *_logger, Config *_config, CDManager *_cdManager, QWidget *parent )
    : config( _config ),
      logger( _logger ),
      cdManager( _cdManager )
{
//     resize( 600, 400 );
    setAcceptDrops( true );

    // the grid for all widgets in the main window
    QGridLayout* gridLayout = new QGridLayout( this );
    gridLayout->setContentsMargins( 6, 6, 6, 6 );
//     gridLayout->setSpacing( 0 );

    fileList = new FileList( config, cdManager, this );
    gridLayout->addWidget( fileList, 1, 0 );
    gridLayout->setRowStretch( 1, 1 );
    connect( fileList, SIGNAL(fileCountChanged(int)), this, SLOT(fileCountChanged(int)) );
    connect( fileList, SIGNAL(conversionStarted()), this, SLOT(conversionStarted()) );
    connect( fileList, SIGNAL(conversionStopped()), this, SLOT(conversionStopped()) );
    connect( fileList, SIGNAL(queueModeChanged(bool)), this, SLOT(queueModeChanged(bool)) );

    optionsLayer = new OptionsLayer( config, this );
    fileList->setOptionsLayer( optionsLayer );
    optionsLayer->hide();
//     optionsLayer->fadeIn();
    gridLayout->addWidget( optionsLayer, 1, 0 );
    connect( optionsLayer, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );


    // add a horizontal box layout for the add combobutton to the grid
    QHBoxLayout *addBox = new QHBoxLayout( 0 ); // TODO destroy
    addBox->setContentsMargins( 0, 0, 0, 0 );
//     addBox->setSpacing( 0 );
    gridLayout->addLayout( addBox, 3, 0 );

    // create the combobutton for adding files to the file list
    cAdd = new ComboButton( this );
    QFont font = cAdd->font();
    //font.setWeight( QFont::DemiBold );
    font.setPointSize( font.pointSize() + 3 );
    cAdd->setFont( font );
    cAdd->insertItem( KIcon("audio-x-generic"), i18n("Add files...") );
    cAdd->insertItem( KIcon("folder"), i18n("Add folder...") );
    cAdd->insertItem( KIcon("media-optical-audio"), i18n("Add CD tracks...") );
    cAdd->insertItem( KIcon("network-workgroup"), i18n("Add Url...") );
    cAdd->insertItem( KIcon("view-media-playlist"), i18n("Add playlist...") );
    cAdd->increaseHeight( 6 );
    addBox->addWidget( cAdd );
    connect( cAdd, SIGNAL(clicked(int)), this, SLOT(addClicked(int)) );
    cAdd->setFocus();

    addBox->addSpacing( 10 );

    startAction = new KAction( KIcon("system-run"), i18n("Start"), this );
    connect( startAction, SIGNAL(triggered()), fileList, SLOT(startConversion()) );

    pStart = new KPushButton( KIcon("system-run"), i18n("Start"), this );
    pStart->setFixedHeight( pStart->size().height() );
    pStart->setEnabled( false );
    startAction->setEnabled( false );
    addBox->addWidget( pStart );
    connect( pStart, SIGNAL(clicked()), fileList, SLOT(startConversion()) );

    stopActionMenu = new KActionMenu( KIcon("process-stop"), i18n("Stop"), this );
    killAction = new KAction( KIcon("flag-red"), i18n("Stop imediatelly"), this );
    stopActionMenu->addAction( killAction );
    connect( killAction, SIGNAL(triggered()), fileList, SLOT(killConversion()) );
    stopAction = new KAction( KIcon("flag-yellow"), i18n("Stop after current conversions are completed"), this );
    stopActionMenu->addAction( stopAction );
    connect( stopAction, SIGNAL(triggered()), fileList, SLOT(stopConversion()) );
    continueAction = new KAction( KIcon("flag-green"), i18n("Continue after current conversions are completed"), this );
    stopActionMenu->addAction( continueAction );
    connect( continueAction, SIGNAL(triggered()), fileList, SLOT(continueConversion()) );
    queueModeChanged( true );

    pStop = new KPushButton( KIcon("process-stop"), i18n("Stop"), this );
    pStop->setFixedHeight( pStop->size().height() );
    pStop->hide();
    stopActionMenu->setEnabled( false );
    pStop->setMenu( stopActionMenu->menu() );
    addBox->addWidget( pStop );

    addBox->addSpacing( 10 );

    progressIndicator = new ProgressIndicator( /*systemTrayIcon,*/ this );
    addBox->addWidget( progressIndicator );
    connect( progressIndicator, SIGNAL(progressChanged(const QString&)), this, SIGNAL(progressChanged(const QString&)) );
    connect( fileList, SIGNAL(timeChanged(float)), progressIndicator, SLOT(timeChanged(float)) );
    connect( fileList, SIGNAL(finished(float)), progressIndicator, SLOT(finished(float)) );

    Convert *convert = new Convert( config, fileList, logger );
    connect( convert, SIGNAL(updateTime(float)), progressIndicator, SLOT(update(float)) );
    connect( convert, SIGNAL(timeFinished(float)), progressIndicator, SLOT(timeFinished(float)) );

    // DEBUG
//     fileList->addFiles(KUrl("file:///home/daniel/Musik/Backup/1 - 04 - Ratatat - Mirando.mp3"), optionsLayer->currentConversionOptions());
//     fileList->addFiles(KUrl("file:///home/daniel/Musik/Backup/1 - 04 - Ratatat - Mirando.mp3"), optionsLayer->currentConversionOptions());
//     fileList->addFiles(KUrl("file:///home/daniel/Musik/Backup/1 - 04 - Ratatat - Mirando.mp3"), optionsLayer->currentConversionOptions());
//     fileList->addFiles(KUrl("file:///home/daniel/Musik/Backup/1 - 04 - Ratatat - Mirando.mp3"), optionsLayer->currentConversionOptions());
//     fileList->addFiles(KUrl("file:///home/daniel/Musik/Backup/1 - 04 - Ratatat - Mirando.mp3"), optionsLayer->currentConversionOptions());
}

soundKonverterView::~soundKonverterView()
{}

void soundKonverterView::addClicked( int index )
{
    fileList->save();

    if( index == 0 )
    {
        showFileDialog();
    }
    else if( index == 1 )
    {
        showDirDialog();
    }
    else if( index == 2 )
    {
        showCdDialog();
    }
    else if( index == 3 )
    {
        showUrlDialog();
    }
    else
    {
        showPlaylistDialog();
    }

    fileList->save();
}

void soundKonverterView::showFileDialog()
{
    FileOpener *dialog = new FileOpener( config, this );
//     dialog->resize( size().width() - 10, size().height() );

    connect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );

    dialog->exec();

    disconnect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::showDirDialog()
{
    DirOpener *dialog = new DirOpener( config, DirOpener::Convert, this );
    
    connect( dialog, SIGNAL(done(const KUrl&,bool,const QStringList&,ConversionOptions*)), fileList, SLOT(addDir(const KUrl&,bool,const QStringList&,ConversionOptions*)) );

    dialog->exec();
    
    disconnect( dialog, SIGNAL(done(const KUrl&,bool,const QStringList&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::showCdDialog( const QString& device, bool intern )
{
    /*
    ConversionOptions conversionOptions = options->getCurrentOptions();

    if( ( instances <= 1 || config->data.general.askForNewOptions ) && ( profile == "" || format == "" || directory == "" ) && !intern )
    {
        OptionsRequester* dialog = new OptionsRequester( config, "", this );

        connect( dialog, SIGNAL(setCurrentOptions(const ConversionOptions&)),
                 options, SLOT(setCurrentOptions(const ConversionOptions&))
               );
//         connect( dialog, SIGNAL(addFiles(QStringList)),
//                    fileList, SLOT(addFiles(QStringList))
//                  );

        Q_CHECK_PTR( dialog );

        if( profile != "" ) {
            dialog->setProfile( profile );
            profile = "";
        }
        if( format != "" ) {
            dialog->setFormat( format );
            format = "";
        }
        if( directory != "" ) {
            dialog->setOutputDirectory( directory );
            directory = "";
        }

        dialog->exec();

        disconnect( dialog, SIGNAL(setCurrentOptions(const ConversionOptions&)), 0, 0 );
//         disconnect( dialog, SIGNAL(addFiles(QStringList)), 0, 0 );

        delete dialog;
    }
    else
    {
        if( profile != "" ) {
            options->setProfile( profile );
            profile = "";
        }
        if( format != "" ) {
            options->setFormat( format );
            format = "";
        }
        if( directory != "" ) {
            options->setOutputDirectory( directory );
            directory = "";
        }
    }

    kapp->eventLoop()->exitLoop();
*/
    
    QString message;
    QStringList errorList;
    if( !config->pluginLoader()->canDecode("audio cd",&errorList) )
    {
        if( !errorList.isEmpty() )
        {
            message = i18n("Ripping audio CDs is currently not supported because of missing backends.\nPossible solutions are listed below.");
        }
        else
        {
            message = i18n("Ripping audio CDs is not supported by any installed plugin.\nPlease have a look at your distributions package manager in order to get a cd ripper plugin for soundKonverter.");
        }
        QMessageBox *messageBox = new QMessageBox( this );
        messageBox->setIcon( QMessageBox::Information );
        messageBox->setWindowTitle( i18n("Missing backends") );
        messageBox->setText( message + "<br><br>" + errorList.join("\n\n").replace("\n","<br>") );
        messageBox->setTextFormat( Qt::RichText );
        messageBox->exec();
        return;
    }

    // create a new CDOpener object for letting the user add some tracks from a CD
    CDOpener *dialog = new CDOpener( config, cdManager, device, this );

    if( !dialog->noCD )
    {
        connect( dialog, SIGNAL(addTracks(const QString&,QList<int>,ConversionOptions*)), fileList, SLOT(addTracks(const QString&,QList<int>,ConversionOptions*)) );

        dialog->exec();

        disconnect( dialog, SIGNAL(addTracks(const QString&,QList<int>,ConversionOptions*)), 0, 0 );
    }
    else
    {
        KMessageBox::information( this, i18n("No audio CD found.") );
    }
    
    delete dialog;
/*
    kapp->eventLoop()->enterLoop();

    options->setCurrentOptions( conversionOptions );
*/
}

void soundKonverterView::showUrlDialog()
{
    UrlOpener *dialog = new UrlOpener( config, this );

    connect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );

    dialog->exec();

    disconnect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::showPlaylistDialog()
{
    PlaylistOpener *dialog = new PlaylistOpener( config, this );
//     dialog->resize( size().width() - 10, size().height() );

    connect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );

    dialog->exec();

    disconnect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::addConvertFiles( const KUrl::List& urls, QString _profile, QString _format, const QString& directory )
{
    KUrl::List k_urls;
    QString codecName;
    QStringList errorList;
    QMap< QString, QList<QStringList> > problems;
    QStringList messageList;
    QString fileName;
    QStringList affectedFiles;
    
    for( int i=0; i<urls.size(); i++ )
    {
        codecName = config->pluginLoader()->getCodecFromFile( urls.at(i) );

        if( codecName == "inode/directory" || config->pluginLoader()->canDecode(codecName,&errorList) )
        {
            k_urls += urls.at(i);
        }
        else
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
                affectedFiles += i18n("... and %1 more files",problems.value(codecName).at(0).count()-3);
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

    if( k_urls.count() > 0 )
    {
        QString profile;
        QString format;
        QStringList formatList = config->pluginLoader()->formatList( PluginLoader::Encode, PluginLoader::CompressionType(PluginLoader::Lossy|PluginLoader::Lossless|PluginLoader::Hybrid) );
        for( int i=0; i<formatList.count(); i++ )
        {
            if( _format == formatList.at(i) || config->pluginLoader()->codecExtensions(formatList.at(i)).contains(_format) )
            {
                format = formatList.at(i);
                break;
            }
        }
        bool lossy = false;
        if( _profile.toLower() == i18n("Very low").toLower() || _profile.toLower() == "very low" || _profile.toLower() == "very_low" )
        {
            profile = i18n("Very low");
            lossy = true;
        }
        else if( _profile.toLower() == i18n("Low").toLower() || _profile.toLower() == "low" )
        {
            profile = i18n("Low");
            lossy = true;
        }
        else if( _profile.toLower() == i18n("Medium").toLower() || _profile.toLower() == "medium" )
        {
            profile = i18n("Medium");
            lossy = true;
        }
        else if( _profile.toLower() == i18n("High").toLower() || _profile.toLower() == "high" )
        {
            profile = i18n("High");
            lossy = true;
        }
        else if( _profile.toLower() == i18n("Very high").toLower() || _profile.toLower() == "very high" || _profile.toLower() == "very_high" )
        {
            profile = i18n("Very high");
            lossy = true;
        }
        else if( _profile.toLower() == i18n("Lossless").toLower() || _profile.toLower() == "lossless" )
        {
            profile = i18n("Lossless");
            format = config->pluginLoader()->formatList(PluginLoader::Encode,PluginLoader::Lossless).contains(format) ? format : "";
        }
        else if( _profile.toLower() == i18n("Hybrid").toLower() || _profile.toLower() == "hybrid" )
        {
            profile = i18n("Hybrid");
            format = config->pluginLoader()->formatList(PluginLoader::Encode,PluginLoader::Hybrid).contains(format) ? format : "";
        }
        else
        {
            for( int i=0; i<config->data.profiles.count(); i++ )
            {
                if( config->data.profiles.at(i).profileName == _profile )
                {
                    profile = _profile;
                    format = config->data.profiles.at(i).codecName;
                }
            }
        }
        
        if( lossy )
        {
            format = "";
            QStringList formatList = config->pluginLoader()->formatList( PluginLoader::Encode, PluginLoader::Lossy );
            for( int i=0; i<formatList.count(); i++ )
            {
                if( _format == formatList.at(i) || config->pluginLoader()->codecExtensions(formatList.at(i)).contains(_format) )
                {
                    format = formatList.at(i);
                    break;
                }
            }
        }

        if( !profile.isEmpty() && !format.isEmpty() && !directory.isEmpty() )
        {
            Options *options = new Options( config, "", 0 );
            options->hide();
            options->setProfile( profile );
            options->setFormat( format );
            options->setOutputDirectory( directory );
            ConversionOptions *conversionOptions = options->currentConversionOptions();
            delete options;
            fileList->addFiles( k_urls, conversionOptions );
        }
        else
        {
            optionsLayer->addUrls( k_urls );
            if( !profile.isEmpty() ) optionsLayer->setProfile( profile );
            if( !format.isEmpty() ) optionsLayer->setFormat( format );
            if( !directory.isEmpty() ) optionsLayer->setOutputDirectory( directory );
            optionsLayer->fadeIn();
        }
    }
}

void soundKonverterView::fileCountChanged( int count )
{
    pStart->setEnabled( count > 0 );
    startAction->setEnabled( count > 0 );
}

void soundKonverterView::conversionStarted()
{
    pStart->hide();
    startAction->setEnabled( false );
    pStop->show();
    stopActionMenu->setEnabled( true );
}

void soundKonverterView::conversionStopped()
{
    pStart->show();
    startAction->setEnabled( true );
    pStop->hide();
    stopActionMenu->setEnabled( false );

//     if( autoclose ) kapp->quit(); // NOTE close app on conversion stop (may irritate the user when stopping the conversion)
}

void soundKonverterView::queueModeChanged( bool enabled )
{
    stopAction->setVisible( enabled );
    continueAction->setVisible( !enabled );    
}

void soundKonverterView::loadFileList()
{
    fileList->load( true );
}

void soundKonverterView::saveFileList()
{
    fileList->save( true );
}

#include "soundkonverterview.moc"
