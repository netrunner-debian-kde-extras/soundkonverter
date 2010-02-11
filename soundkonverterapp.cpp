
#include "soundkonverterapp.h"
#include "soundkonverter.h"

// #include <qstringlist.h>
// #include <qfile.h>
// #include <qmovie.h>

#include <kglobal.h>
// #include <kstartupinfo.h>
#include <kcmdlineargs.h>
// #include <dcopclient.h>
// #include <ksystemtray.h>
// #include <kstandarddirs.h>
#include <stdio.h>

#include <kurl.h>


soundKonverterApp::soundKonverterApp()
    : KUniqueApplication()
{
    mainWindow = new soundKonverter();
    mainWindow->show();
}

soundKonverterApp::~soundKonverterApp()
{}

int soundKonverterApp::newInstance()
{
    //KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
/*    
    if( !activeWindow() )
    {
//         sk_window = new soundKonverter();
//         setActiveWindow( sk_window );
    }
    else
    {
//         sk_window = qobject_cast<soundKonverter*>( activeWindow() );
    }
*/

    if( args->count() > 0 )
    {
        QString device = args->getOption( "rip" );
        if( !device.isEmpty() )
        {
            mainWindow->ripCd( device );
        }
        
        mainWindow->setAutoClose( args->isSet( "autoclose" ) );
        
        if( args->isSet( "invisible" ) )
        {
            mainWindow->setAutoClose( true );
            mainWindow->hide();
            mainWindow->showSystemTray();
        }
        else
        {
            mainWindow->setAutoClose( false );
            mainWindow->show();
        }

        QString profile = args->getOption( "profile" );
        QString format = args->getOption( "format" );
        QString directory = args->getOption( "output" );
        
        if( args->isSet( "replaygain" ) )
        {
            KUrl::List urls;
            for( int i=0; i<args->count(); i++ )
            {
                urls.append( args->arg(i) );
            }
            if( !urls.isEmpty() ) mainWindow->addReplayGainFiles( urls );
        }
        else
        {
            KUrl::List urls;
            for( int i=0; i<args->count(); i++ )
            {
                urls.append( args->arg(i) );
            }
            if( !urls.isEmpty() ) mainWindow->addConvertFiles( urls, profile, format, directory );
        }
    }
    args->clear();
    
    mainWindow->activateWindow();

    return 0;

    
    
/*    
    // register ourselves as a dcop client
    if( !dcopClient()->isRegistered() )
        dcopClient()->registerAs( name(), false );

    // see if we are starting with session management
    if( restoringSession() )
    {
        RESTORE( soundKonverter );
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if( !mainWidget() )
        {
            soundKonverter *widget = new soundKonverter();
            setMainWidget(widget);
            //widget->show();
        }
        else
            KStartupInfo::setNewStartupId( mainWidget(), kapp->startupId());

        soundKonverter *widget = ::qt_cast<soundKonverter*>( mainWidget() );

        widget->increaseInstances();

        QCString notify = args->getOption( "command" );
        if( notify ) {
            widget->setNotify( notify );
        }

        QCString profile = args->getOption( "profile" );
        if( profile ) {
            widget->profile = profile;
        }

        QCString format = args->getOption( "format" );
        if( format ) {
            widget->format = format;
        }

        QCString directory = args->getOption( "output" );
        if( directory ) {
            widget->directory = directory;
        }

        QCString device = args->getOption( "rip" );
        if( device ) {
            if( !args->isSet( "invisible" ) ) {
                widget->visible = true;
                widget->show();
                widget->systemTray->hide();
                widget->systemTray->setPixmap( 0 );
            }
            widget->device = device;
            widget->showCdDialog( false );
        }

        widget->autoclose = args->isSet( "autoclose" );

        if( args->isSet( "invisible" ) ) {
            widget->visible = false;
            widget->autoclose = true;
            widget->hide();
            widget->systemTray->show();
            KStandardDirs* stdDirs = new KStandardDirs();
            widget->systemTray->setMovie( QMovie(stdDirs->findResource("data","soundkonverter/pics/systray.mng")) );
            delete stdDirs;
        }
        else {
            widget->visible = true;
            widget->show();
            widget->systemTray->hide();
            widget->systemTray->setPixmap( 0 );
        }

        // add the files to the file lists depending on the used switch
        if( args->isSet( "replaygain" ) ) {
            QStringList replayGainFiles;
            for( int i = 0; i < args->count(); i++ ) {
//                 replayGainFiles.append(KURL::encode_string(args->arg(i)));
                replayGainFiles.append(KURL::encode_string(QString::fromUtf8(args->arg(i))));
            }
            if(!replayGainFiles.isEmpty())
                widget->openArgReplayGainFiles(replayGainFiles);
        }
//         else if( args->isSet( "repair" ) ) {
//             QStringList repairFiles;
//             for( int i = 0; i < args->count(); i++ ) {
//                 repairFiles.append(QFile::decodeName(args->arg(i)));
//             }
//             if(!repairFiles.isEmpty())
//                 widget->openArgRepairFiles(repairFiles);
//         }
        else {
            QStringList files;
            for( int i = 0; i < args->count(); i++ )
            {
                files.append(KURL::encode_string(QString::fromUtf8(args->arg(i))));
            }
            if(!files.isEmpty())
                widget->openArgFiles(files);
        }

        args->clear();
    }
    return 0;
    */
}

