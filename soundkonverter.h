/*
 * soundkonverter.h
 *
 * Copyright (C) 2007 Daniel Faust <hessijames@gmail.com>
 */
#ifndef SOUNDKONVERTER_H
#define SOUNDKONVERTER_H


#include <KXmlGuiWindow>
#include <KUrl>

class soundKonverterView;
class KToggleAction;
class KUrl;
class Config;
class Logger;
class LogViewer;
class CDManager;
class ReplayGainScanner;
class KStatusNotifierItem;

/**
 * This class serves as the main window for soundKonverter.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 1.0
 */
class soundKonverter : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /** Default Constructor */
    soundKonverter();

    /** Default Destructor */
    virtual ~soundKonverter();
    
    void showSystemTray();
    void addConvertFiles( const KUrl::List& urls, const QString& profile, const QString& format, const QString& directory );
    void addReplayGainFiles( const KUrl::List& urls );
    void ripCd( const QString& device );
    void setAutoClose( bool enabled ) { autoclose = enabled; }

private slots:
    void showConfigDialog();
    void showLogViewer();
    void showReplayGainScanner();
    void progressChanged( const QString& progress );

    /** The conversion has started */
    void conversionStarted();
    /** The conversion has stopped */
    void conversionStopped();
    
private:
    Config *config;
    Logger *logger;
    CDManager *cdManager;
    ReplayGainScanner *replayGainScanner;

    soundKonverterView *m_view;
    LogViewer *logViewer;
    
    KStatusNotifierItem *systemTray;
    
    bool autoclose;

//     KToggleAction *m_toolbarAction;

    void setupActions();
};

#endif // _SOUNDKONVERTER_H_
