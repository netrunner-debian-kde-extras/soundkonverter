/*
 * soundkonverterview.h
 *
 * Copyright (C) 2007 Daniel Faust <hessijames@gmail.com>
 */
#ifndef _SOUNDKONVERTERVIEW_H_
#define _SOUNDKONVERTERVIEW_H_

#include <QWidget>

class KPushButton;
class QMenu;
class QAction;
class QToolButton;

class ProgressIndicator;
class ComboButton;
class Config;
class Logger;
class CDManager;
class FileList;

// class QPainter;
// class KUrl;

/**
 * This is the main view class for soundKonverter.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 1.0
 */

class soundKonverterView : public QWidget
{
    Q_OBJECT
public:
    /** Default constructor */
    soundKonverterView( Logger *_logger, Config *_config, CDManager *_cdManager, QWidget *parent);

    /** Destructor */
    virtual ~soundKonverterView();

signals:
    /** Use this signal to change the content of the statusbar */
//     void signalChangeStatusbar(const QString& text);

    /** Use this signal to change the content of the caption */
//     void signalChangeCaption( const QString& text );

private slots:
    void addClicked( int index );
    void showFileDialog();
    void showDirDialog();
    void showCdDialog( bool intern = true );
    void showUrlDialog();
    void showPlaylistDialog();
    void settingsChanged();

    // connected to fileList
    /** The count of items in the file list has changed to @p count */
    void fileCountChanged( int count );
    /** The conversion has started */
    void conversionStarted();
    /** The conversion has stopped */
    void conversionStopped();
    /** Conversion will continue/stop after current files have been converted */
    void queueModeChanged( bool enabled );
    
private:
    Config *config;
    Logger *logger;
    CDManager *cdManager;

    FileList *fileList;

    /** The combobutton for adding files */
    ComboButton *cAdd;
    QToolButton *pAdd;
    /** The menu for the add button */
    QMenu *addActionMenu;
    QAction *addFilesAction;
    QAction *addDirectoryAction;
    QAction *addAudioCdAction;
    QAction *addUrlAction;
    QAction *addPlaylistAction;

    /** The button to start the conversion */
    KPushButton *pStart;

    /** The button to stop the conversion */
    KPushButton *pStop;
    /** The menu for the stop button */
    QMenu *stopActionMenu;
    QAction *stopAction;
    QAction *continueAction;
    QAction *killAction;

    /** Displays the current progress */
    ProgressIndicator *progressIndicator;

public slots:
    void loadFileList();
    void saveFileList();

signals:
    void progressChanged( const QString& progress );
};

#endif // _soundKonverterVIEW_H_
