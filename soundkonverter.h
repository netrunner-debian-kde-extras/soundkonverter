/*
 * soundkonverter.h
 *
 * Copyright (C) 2007 Daniel Faust <hessijames@gmail.com>
 */
#ifndef SOUNDKONVERTER_H
#define SOUNDKONVERTER_H


#include <KXmlGuiWindow>

class soundKonverterView;
class KToggleAction;
class KUrl;
class Config;
class Logger;
class LogViewer;
class CDManager;
class ReplayGainScanner;

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
    /**
     * Default Constructor
     */
    soundKonverter();

    /**
     * Default Destructor
     */
    virtual ~soundKonverter();

private slots:
    void showConfigDialog();
    void showLogViewer();
    void showReplayGainScanner();
    void progressChanged(const QString& progress);

private:
    Config *config;
    Logger *logger;
    CDManager *cdManager;
    ReplayGainScanner *replayGainScanner;

    soundKonverterView *m_view;
    LogViewer *logViewer;

//     KToggleAction *m_toolbarAction;

    void setupActions();
};

#endif // _SOUNDKONVERTER_H_
