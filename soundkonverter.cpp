/*
 * soundkonverter.cpp
 *
 * Copyright (C) 2007 Daniel Faust <hessijames@gmail.com>
 */
#include "soundkonverter.h"
#include "soundkonverterview.h"
#include "config.h"
#include "configdialog.h"
#include "logger.h"
#include "logviewer.h"
#include "audiocd/cdmanager.h"
#include "replaygainscanner.h"

// #include "configgeneralpage.h"
// #include "configadvancedpage.h"
// #include "configbackendspage.h"
// #include <KConfigDialog>

// #include <QDropEvent>
// #include <QPainter>

// #include <kconfigdialog.h>

#include <KAction>
#include <KActionCollection>
#include <KStandardAction>

#include <KLocale>

#include <QToolBar>
#include <KToolBar>
#include <KIcon>


soundKonverter::soundKonverter()
    : KXmlGuiWindow()
{
    logViewer = 0;
    replayGainScanner = 0;

    // accept dnd
    setAcceptDrops(true);

    logger = new Logger( this );
    logger->log( 1000, "This is soundKonverter 1.0.0" );

    config = new Config( logger, this );
    config->load();

    cdManager = new CDManager( this );

    m_view = new soundKonverterView( logger, config, cdManager, this );
    connect( m_view, SIGNAL(progressChanged(const QString&)), this, SLOT(progressChanged(QString)) );

    // tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget( m_view );

    // then, setup our actions
    setupActions();

    // add a status bar
//     statusBar()->show();

//     addToolBar("std");
//     toolBar("std")->addAction( KIcon("flag-green"), i18n("Test") );
//     toolBar("std")->show();

    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

//    cdManager->newCDDevice();
}

soundKonverter::~soundKonverter()
{
    if( logViewer ) delete logViewer;
    if( replayGainScanner ) delete replayGainScanner;
}

void soundKonverter::setupActions()
{
    KStandardAction::quit( this, SLOT(close()), actionCollection() );
    KStandardAction::preferences( this, SLOT(showConfigDialog()), actionCollection() );

    KAction *logviewer = actionCollection()->addAction("logviewer");
    logviewer->setText(i18n("View logs ..."));
    logviewer->setIcon(KIcon("view-list-text"));
//     newAct->setShortcut(Qt::Key_F6);
    connect( logviewer, SIGNAL(triggered()), this, SLOT(showLogViewer()) );

    KAction *replaygainscanner = actionCollection()->addAction("replaygainscanner");
    replaygainscanner->setText(i18n("Replay Gain Scanner ..."));
    replaygainscanner->setIcon(KIcon("soundkonverter-replaygain"));
//     newAct->setShortcut(Qt::Key_F6);
    connect( replaygainscanner, SIGNAL(triggered()), this, SLOT(showReplayGainScanner()) );

    KAction *add_files = actionCollection()->addAction("add_files");
    add_files->setText(i18n("Add files ..."));
    add_files->setIcon(KIcon("audio-x-generic"));
    connect( add_files, SIGNAL(triggered()), m_view, SLOT(showFileDialog()) );
    
    KAction *add_folder = actionCollection()->addAction("add_folder");
    add_folder->setText(i18n("Add folder ..."));
    add_folder->setIcon(KIcon("folder"));
    connect( add_folder, SIGNAL(triggered()), m_view, SLOT(showDirDialog()) );
    
    KAction *add_audiocd = actionCollection()->addAction("add_audiocd");
    add_audiocd->setText(i18n("Add CD tracks ..."));
    add_audiocd->setIcon(KIcon("media-optical-audio"));
    connect( add_audiocd, SIGNAL(triggered()), m_view, SLOT(showCdDialog()) );
    
    KAction *add_url = actionCollection()->addAction("add_url");
    add_url->setText(i18n("Add URL ..."));
    add_url->setIcon(KIcon("network-workgroup"));
    connect( add_url, SIGNAL(triggered()), m_view, SLOT(showUrlDialog()) );
    
    KAction *add_playlist = actionCollection()->addAction("add_playlist");
    add_playlist->setText(i18n("Add playlist ..."));
    add_playlist->setIcon(KIcon("view-media-playlist"));
    connect( add_playlist, SIGNAL(triggered()), m_view, SLOT(showPlaylistDialog()) );
    
    KAction *load = actionCollection()->addAction("load");
    load->setText(i18n("Load file list"));
    load->setIcon(KIcon("document-open"));
    connect( load, SIGNAL(triggered()), m_view, SLOT(loadFileList()) );
    
    KAction *save = actionCollection()->addAction("save");
    save->setText(i18n("Save file list"));
    save->setIcon(KIcon("document-save"));
    connect( save, SIGNAL(triggered()), m_view, SLOT(saveFileList()) );
    
//     QToolBar *toolBar = new QToolBar( "tools", this );
//     toolBar->addAction( "test" );
//     toolBar->show();
//     moveDockWindow( toolBar, Qt::DockTop );
    
    // custom menu and menu item - the slot is in the class soundKonverterView
/*    KAction *custom = new KAction(KIcon("colorize"), i18n("Swi&tch Colors"), this);
    actionCollection()->addAction( QLatin1String("switch_action"), custom );
    connect(custom, SIGNAL(triggered(bool)), m_view, SLOT(switchColors()));*/
}

void soundKonverter::showConfigDialog()
{
    ConfigDialog *dialog = new ConfigDialog( config, this/*, ConfigDialog::Page(configStartPage)*/ );

    Q_CHECK_PTR( dialog );
    dialog->resize( size() );
    dialog->exec();

    delete dialog;

//     configStartPage = ConfigDialog::GeneralPage;




/*    KConfigDialog *dialog = new KConfigDialog(this, "settings", new KConfigSekeleton("test",this));
    dialog->setFaceType(KPageDialog::List);
    dialog->addPage(new ConfigGeneralPage( config, this ), i18n("General"), KIcon("configure") );
    dialog->addPage(new ConfigAdvancedPage( config, this ), i18n("Advanced"), KIcon("preferences-desktop-gaming") );
    dialog->addPage(new ConfigBackendsPage( config, this ), i18n("Backends"), KIcon("applications-system") );*/
    
//     configGeneralPage = new ConfigGeneralPage( config, this );
//     generalPage = addPage( (QWidget*)configGeneralPage, i18n("General") );
//     generalPage->setIcon( KIcon("configure") );
// 
//     configAdvancedPage = new ConfigAdvancedPage( config, this );
//     advancedPage = addPage( (QWidget*)configAdvancedPage, i18n("Advanced") );
//     advancedPage->setIcon( KIcon("preferences-desktop-gaming") );
// 
//     configBackendsPage = new ConfigBackendsPage( config, this );
//     backendsPage = addPage( (QWidget*)configBackendsPage, i18n("Backends") );
//     backendsPage->setIcon( KIcon("applications-system") );

/*    connect(dialog, SIGNAL(settingsChanged(const QString&)), mainWidget, SLOT(loadSettings()));
    connect(dialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(loadSettings()));*/
//     dialog->show();
}

void soundKonverter::showLogViewer()
{
    if( !logViewer ) logViewer = new LogViewer( logger, 0 );

    logViewer->show();
    logViewer->raise();
}

void soundKonverter::showReplayGainScanner()
{
    if( !replayGainScanner ) replayGainScanner = new ReplayGainScanner( config, logger, 0 );

    replayGainScanner->show();
    replayGainScanner->raise();
}

void soundKonverter::progressChanged(const QString& progress)
{
    setWindowTitle( progress + " - soundKonverter" );
}

// void soundKonverter::optionsPreferences()
// {
//     // The preference dialog is derived from prefs_base.ui
//     //
//     // compare the names of the widgets in the .ui file
//     // to the names of the variables in the .kcfg file
//     //avoid to have 2 dialogs shown
//     if ( KConfigDialog::showDialog( "settings" ) )  {
//         return;
//     }
//     KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());
//     QWidget *generalSettingsDlg = new QWidget;
//     ui_prefs_base.setupUi(generalSettingsDlg);
//     dialog->addPage(generalSettingsDlg, i18n("General"), "package_setting");
//     connect(dialog, SIGNAL(settingsChanged(QString)), m_view, SLOT(settingsChanged()));
//     dialog->setAttribute( Qt::WA_DeleteOnClose );
//     dialog->show();
// }

#include "soundkonverter.moc"
