/*
 * soundkonverterview.cpp
 *
 * Copyright (C) 2007 Daniel Faust <hessijames@gmail.com>
 */
#include "soundkonverterview.h"
#include "filelistmodel.h"
#include "filelist.h"
#include "filelistitem.h"
#include "combobutton.h"
#include "progressindicator.h"
#include "optionslayer.h"
#include "config.h"
#include "opener.h"
#include "dirdialog.h"
#include "convert.h"
#include "audiocd/cdopener.h"
#include "audiocd/cdmanager.h"

#include <KLocale>
#include <KPushButton>
#include <KIcon>
#include <KFileDialog>

#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QTreeView>
#include <QMenu>
#include <QAction>
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

    OptionsLayer *optionsLayer = new OptionsLayer( config, this );
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
    cAdd->insertItem( KIcon("audio-x-generic"), i18n("Add files ...") );
    cAdd->insertItem( KIcon("folder"), i18n("Add folder ...") );
    cAdd->insertItem( KIcon("media-optical-audio"), i18n("Add CD tracks ...") );
    cAdd->insertItem( KIcon("network-workgroup"), i18n("Add URL ...") );
    cAdd->insertItem( KIcon("view-media-playlist"), i18n("Add playlist ...") );
    cAdd->increaseHeight( 6 );
    addBox->addWidget( cAdd );
    connect( cAdd, SIGNAL(clicked(int)), this, SLOT(addClicked(int)) );
    cAdd->setFocus();

//     addActionMenu = new QMenu();
//     addFilesAction = addActionMenu->addAction( KIcon("audio-x-generic"), i18n("Add files ...") );
//     addDirectoryAction = addActionMenu->addAction( KIcon("folder"), i18n("Add folder ...") );
//     addAudioCdAction = addActionMenu->addAction( KIcon("media-optical-audio"), i18n("Add CD tracks ...") );
//     addUrlAction = addActionMenu->addAction( KIcon("network-workgroup"), i18n("Add URL ...") );
//     addPlaylistAction = addActionMenu->addAction( KIcon("view-media-playlist"), i18n("Add playlist ...") );
    
//     pAdd = new QToolButton( this );
//     pAdd->setMenu( addActionMenu );
//     pAdd->setPopupMode( QToolButton::MenuButtonPopup );
//     pAdd->setDefaultAction( addFilesAction );
//     addBox->addWidget( pAdd );
    
    addBox->addSpacing( 18 );

    pStart = new KPushButton( KIcon("system-run"), i18n("Start"), this );
    pStart->setFixedHeight( pStart->size().height() );
    pStart->setEnabled( false );
    addBox->addWidget( pStart );
    connect( pStart, SIGNAL(clicked()), fileList, SLOT(startConversion()) );

    stopActionMenu = new QMenu();
    killAction = stopActionMenu->addAction( KIcon("flag-red"), i18n("Stop imediatelly") );
    connect( killAction, SIGNAL(triggered()), fileList, SLOT(killConversion()) );
    stopAction = stopActionMenu->addAction( KIcon("flag-yellow"), i18n("Stop after current conversions are completed") );
    connect( stopAction, SIGNAL(triggered()), fileList, SLOT(stopConversion()) );
    continueAction = stopActionMenu->addAction( KIcon("flag-green"), i18n("Continue after current conversions are completed") );
    connect( continueAction, SIGNAL(triggered()), fileList, SLOT(continueConversion()) );
    queueModeChanged( false );

    pStop = new KPushButton( KIcon("process-stop"), i18n("Stop"), this );
    pStop->setFixedHeight( pStop->size().height() );
    pStop->hide();
    pStop->setMenu( stopActionMenu );
    addBox->addWidget( pStop );

    addBox->addSpacing( 8 );

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
    Opener *dialog = new Opener( config, Opener::Files, this );
    dialog->resize( size().width() - 10, size().height() );

    connect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );

    dialog->exec();

    disconnect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::showDirDialog()
{
    DirDialog *dialog = new DirDialog( config, DirDialog::Convert, this );
    
    connect( dialog, SIGNAL(done(const KUrl&,bool,const QStringList&,ConversionOptions*)), fileList, SLOT(addDir(const KUrl&,bool,const QStringList&,ConversionOptions*)) );

    dialog->exec();
    
    disconnect( dialog, SIGNAL(done(const KUrl&,bool,const QStringList&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::showCdDialog( bool intern )
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
            message = i18n("Ripping audio CDs is currently not supported because there are missing bakends.\nPossible solutions are listed below.");
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

    QString device = "";

    // create a new CDOpener object for letting the user add some tracks from a CD
    CDOpener *dialog = new CDOpener( config, cdManager, device, this );

    device = "";

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
    Opener *dialog = new Opener( config, Opener::Url, this );

    connect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );

    dialog->exec();

    disconnect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::showPlaylistDialog()
{
    Opener *dialog = new Opener( config, Opener::Playlist, this );
    dialog->resize( size().width() - 10, size().height() );

    connect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), fileList, SLOT(addFiles(const KUrl::List&,ConversionOptions*)) );

    dialog->exec();

    disconnect( dialog, SIGNAL(done(const KUrl::List&,ConversionOptions*)), 0, 0 );

    delete dialog;
}

void soundKonverterView::settingsChanged()
{
//     QPalette pal;
//     pal.setColor( QPalette::Window, Settings::col_background());
//     pal.setColor( QPalette::WindowText, Settings::col_foreground());
//     ui_soundkonverterview_base.kcfg_sillyLabel->setPalette( pal );

    // i18n : internationalization
//     ui_soundkonverterview_base.kcfg_sillyLabel->setText( i18n("This project is %1 days old",Settings::val_time()) );
//     emit signalChangeStatusbar( i18n("Settings changed") );
}

void soundKonverterView::fileCountChanged( int count )
{
    pStart->setEnabled( count > 0 );
}

void soundKonverterView::conversionStarted()
{
    pStart->hide();
    pStop->show();
    stopActionMenu->setEnabled( true );
}

void soundKonverterView::conversionStopped()
{
    pStart->show();
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
    fileList->load();
}

void soundKonverterView::saveFileList()
{
    fileList->save();
}

#include "soundkonverterview.moc"
