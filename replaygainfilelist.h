
#ifndef REPLAYGAINFILELIST_H
#define REPLAYGAINFILELIST_H

#include "replaygainfilelistitem.h"
#include "core/replaygainplugin.h"

#include <KUrl>
#include <QTime>

class Config;
class Logger;
class ConversionOptions;
class QProgressBar;


/**
 * @short The file list of the Replay Gain scanner
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 1.0
 */
class ReplayGainFileList : public QTreeWidget
{
    Q_OBJECT
public:
    /** Constructor */
    ReplayGainFileList( Config *_config, Logger *_logger, QWidget *parent=0 );

    /** Destructor */
    virtual ~ReplayGainFileList();
    
    ReplayGainFileListItem *topLevelItem( int index ) const { return static_cast<ReplayGainFileListItem*>( QTreeWidget::topLevelItem(index) ); }
    
    void addFiles( const KUrl::List& fileList, QString codecName = "", ReplayGainFileListItem *after = 0, bool enabled = false );
    
    void calcAllReplayGain( bool force = false );
    void removeAllReplayGain();
    void cancelProcess();
    
private:
    Config *config;
    Logger *logger;

    /** Lists all file in a directory and adds them to the file list, if fast is false. The number of listed files is returned */
    int listDir( const QString& directory, const QStringList& filter, bool recursive, bool fast = false, int count = 0 );
    /** A progressbar, that is shown, when a directory is added recursive */
    QProgressBar *pScanStatus;
    /** Update timer for the scan status */
    QTime tScanStatus;

    void resizeEvent( QResizeEvent *event );

//     bool processing;        // true, if the progress is active (hide some options in the context menu)
    bool queue;
    
    ReplayGainPlugin::ApplyMode mode;
    
    QList<ReplayGainFileListItem*> processedItems;
    
    void processNextFile();
    int processingCount();
    void processItems( const QList<ReplayGainFileListItem*>& itemList );
    void updateItem( ReplayGainFileListItem *item );

public slots:
    void addDir( const KUrl& directory, bool recursive, const QStringList& codecList );
    
private slots:
    void pluginProcessFinished( int id, int exitCode );
    void pluginLog( int id, const QString& message );

signals:
    void processStarted();
    void processStopped();
    void updateProgress( int progress, int totalSteps );
};

#endif // REPLAYGAINFILELIST_H
