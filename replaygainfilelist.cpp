
#include "replaygainfilelist.h"

#include "logger.h"
#include "config.h"

#include <QResizeEvent>
#include <QGridLayout>
#include <QProgressBar>
#include <KMessageBox>
#include <QDir>
#include <QFileInfo>


ReplayGainFileList::ReplayGainFileList( Config *_config, Logger *_logger, QWidget *parent )
    : QTreeWidget( parent )
{
    config = _config;
    logger = _logger;

    queue = false;

    setColumnCount( 3 );
    QStringList labels;
    labels.append( i18n("File") );
    labels.append( i18n("Track") );
    labels.append( i18n("Album") );
    setHeaderLabels( labels );
//     header()->setClickEnabled( false );

    setSelectionBehavior( QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSortingEnabled( false );

    setDragDropMode( QAbstractItemView::InternalMove );

    QGridLayout *grid = new QGridLayout( this );
    grid->setRowStretch( 0, 1 );
    grid->setRowStretch( 2, 1 );
    grid->setColumnStretch( 0, 1 );
    grid->setColumnStretch( 2, 1 );
    pScanStatus = new QProgressBar( this );
    pScanStatus->setMinimumHeight( pScanStatus->height() );
    pScanStatus->setFormat( "%v / %m" );
    pScanStatus->hide();
    grid->addWidget( pScanStatus, 1, 1 );
    grid->setColumnStretch( 1, 2 );

    QList<ReplayGainPlugin*> replayGainPlugins = config->pluginLoader()->getAllReplayGainPlugins();
    for( int i = 0; i < replayGainPlugins.size(); i++ )
    {
        connect( replayGainPlugins.at(i), SIGNAL(applyFinished(int,int)), this, SLOT(pluginProcessFinished(int,int)) );
        connect( replayGainPlugins.at(i), SIGNAL(log(int,const QString&)), this, SLOT(pluginLog(int,const QString&)) );
    }
}

ReplayGainFileList::~ReplayGainFileList()
{}

void ReplayGainFileList::resizeEvent( QResizeEvent *event )
{
    if( event->size().width() < 300 ) return;

    setColumnWidth( 0, event->size().width()-160 );
    setColumnWidth( 1, 80 );
    setColumnWidth( 2, 80 );
}

int ReplayGainFileList::listDir( const QString& directory, const QStringList& filter, bool recursive, bool fast, int count )
{
    QString codecName;
  
    QDir dir( directory );
    dir.setFilter( QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::Readable );

    QStringList list = dir.entryList();

    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        if( *it == "." || *it == ".." ) continue;
        
        QFileInfo fileInfo( directory + "/" + *it );
        
        if( fileInfo.isDir() && recursive )
        {
            count = listDir( directory + "/" + *it, filter, recursive, fast, count );
        }
        else if( !fileInfo.isDir() ) // NOTE checking for isFile may not work with all file names
        {
            count++;
            
            if( fast )
            {
                pScanStatus->setMaximum( count );
            }
            else
            {
                codecName = config->pluginLoader()->getCodecFromFile( directory + "/" + *it );
                
                if( filter.count() == 0 || filter.contains(codecName) )
                {
                    addFiles( KUrl(directory + "/" + *it), codecName );
                    if( tScanStatus.elapsed() > config->data.general.updateDelay * 10 )
                    {
                        pScanStatus->setValue( count );
                        tScanStatus.start();
                    }
                }
            }
        }
    }

    return count;
}

void ReplayGainFileList::addFiles( const KUrl::List& fileList, QString codecName, ReplayGainFileListItem *after, bool enabled )
{
    ReplayGainFileListItem *lastListItem;
    if( !after && !enabled ) lastListItem = topLevelItem( topLevelItemCount()-1 );
    else lastListItem = after;
    ReplayGainFileListItem *newItem;
    QString filePathName;
    QString device;
    QStringList unsupportedList;

    for( int i = 0; i < fileList.count(); i++ )
    {
        if( codecName.isEmpty() )
        {
            codecName = config->pluginLoader()->getCodecFromFile( fileList.at(i) );
            
            if( !config->pluginLoader()->canReplayGain(codecName,0) )
            {
                unsupportedList.append( fileList.at(i).pathOrUrl() );
                continue;
            }
        }

        TagData *tags = config->tagEngine()->readTags( fileList.at(i) );

        if( tags && !tags->album.isEmpty() )
        {
            newItem = 0;

            for( int j = 0; j < topLevelItemCount(); j++ )
            {
                if( topLevelItem(j)->type == ReplayGainFileListItem::Album && topLevelItem(j)->codecName == codecName && topLevelItem(j)->text(0) == tags->album )
                {
                    newItem = new ReplayGainFileListItem( topLevelItem(j) );
                    newItem->type = ReplayGainFileListItem::Track;
                    newItem->codecName = codecName;
                    newItem->url = fileList.at(i);
                    newItem->tags = tags;
                    newItem->time = tags->length;
                    break;
                }
            }

            if( !newItem )
            {
                    newItem = new ReplayGainFileListItem( this );
                    newItem->type = ReplayGainFileListItem::Album;
                    newItem->codecName = codecName;
                    newItem->setText( 0, tags->album );
                    newItem->setExpanded( true );
                    newItem = new ReplayGainFileListItem( newItem );
                    newItem->type = ReplayGainFileListItem::Track;
                    newItem->codecName = codecName;
                    newItem->url = fileList.at(i);
                    newItem->tags = tags;
                    newItem->time = tags->length;
            }
        }
        else
        {
            newItem = new ReplayGainFileListItem( this );
            newItem->type = ReplayGainFileListItem::Track;
            newItem->codecName = codecName;
            newItem->url = fileList.at(i);
            newItem->tags = new TagData();
            newItem->time = 200;
        }

        updateItem( newItem );

//         emit timeChanged( newItem->time );
    }

//     emit fileCountChanged( topLevelItemCount() );

    if( unsupportedList.size() > 0 ) KMessageBox::errorList( this, "The following files could not be added:", unsupportedList );
}

void ReplayGainFileList::addDir( const KUrl& directory, bool recursive, const QStringList& codecList )
{
    pScanStatus->setValue( 0 );
    pScanStatus->setMaximum( 0 );
    pScanStatus->show(); // show the status while scanning the directories
    tScanStatus.start();

    listDir( directory.path(), codecList, recursive, true );
    listDir( directory.path(), codecList, recursive );

    pScanStatus->hide(); // hide the status bar, when the scan is done
}

void ReplayGainFileList::updateItem( ReplayGainFileListItem *item )
{
    if( !item || item->type == ReplayGainFileListItem::Album ) return;

    item->setText( 0, item->url.pathOrUrl() );
    item->setText( 1, QString().sprintf("%+.2f dB",item->tags->track_gain) );
    item->setText( 2, QString().sprintf("%+.2f dB",item->tags->album_gain) );
}

void ReplayGainFileList::processItems( const QList<ReplayGainFileListItem*>& itemList )
{
    if( itemList.count() <= 0 ) return;
    
    ReplayGainPlugin *plugin = config->pluginLoader()->getReplayGainPipes( itemList.at(0)->codecName ).at(0).plugin; // TODO iterate through all plugins and try them unil one works
    
    if( !plugin ) return;
    
    KUrl::List urls;
    for( int i = 0; i < itemList.count(); i++ )
    {
        urls += itemList.at(i)->url;
    }
    
    int id = plugin->apply( urls, mode );

    for( int i = 0; i < itemList.count(); i++ )
    {
        itemList.at(i)->processId = id;
    }
}

void ReplayGainFileList::calcAllReplayGain( bool force )
{
    queue = true;
    mode = ReplayGainPlugin::Add;
    if( force ) mode = (ReplayGainPlugin::ApplyMode)( mode | ReplayGainPlugin::Force );
    emit processStarted();
    processNextFile();
}

void ReplayGainFileList::removeAllReplayGain()
{
    queue = true;
    mode = ReplayGainPlugin::Remove;
    emit processStarted();
    processNextFile();
}

void ReplayGainFileList::cancelProcess()
{
/*    queue = false;
    if( process->isRunning() )
    {
        bool ret = process->kill( SIGKILL );
        if( ret ) {
            logger->log( logID, i18n("Killing process ...") );
        }
        else {
            logger->log( logID, i18n("Killing process failed. Stopping after files are completed ...") );
        }
    }*/
}

void ReplayGainFileList::processNextFile()
{
    if( !queue ) return;
  
    int count = processingCount();
    ReplayGainFileListItem *item, *child;
    QList<ReplayGainFileListItem*> itemList;

    for( int i = 0; i < topLevelItemCount() && count < config->data.general.numFiles; i++ )
    {
        item = topLevelItem( i );
        if( item->processing ) continue;
        if( item->type == ReplayGainFileListItem::Track )
        {
              itemList += item;
        }
        else
        {
            for( int j = 0; j < item->childCount(); j++ )
            {
                child = (ReplayGainFileListItem*)item->child( j );
                if( child->processing ) { itemList.clear(); break; }
                else itemList += child;
            }
        }
        count++;
        processItems( itemList );
    }
    
    if( count <= 0 )
    {
        queue = false;
        emit processStopped();
    }
}

int ReplayGainFileList::processingCount()
{
    ReplayGainFileListItem *item, *child;
    int count = 0;

    for( int i = 0; i < topLevelItemCount(); i++ )
    {
        item = topLevelItem( i );
        if( item->processing ) count++;
        if( item->type == ReplayGainFileListItem::Album )
        {
            for( int j = 0; j < item->childCount(); j++ )
            {
                child = (ReplayGainFileListItem*)item->child( j );
                if( child->processing ) count++;
            }
        }
        count++;
    }
    
    return count;
}

void ReplayGainFileList::pluginProcessFinished( int id, int exitCode )
{
    ReplayGainFileListItem *item, *child;

    for( int i = 0; i < topLevelItemCount(); i++ )
    {
        item = topLevelItem( i );
        if( item->processId == id )
        {
            if( exitCode == 0 )
            {
                // update time / progress
                delete item;
                i--;
            }
            else
            {
                logger->log( 1000, "\t" + i18n("Replay Gain failed. Exit code: %1").arg(exitCode) );
            }
            
        }
        else if( item->type == ReplayGainFileListItem::Album )
        {
            for( int j = 0; j < item->childCount(); j++ )
            {
                child = (ReplayGainFileListItem*)item->child( j );
                if( child->processId == id )
                {
                    if( exitCode == 0 )
                    {
                        // update time / progress
                        delete child;
                        j--;
                    }
                    else
                    {
                        logger->log( 1000, "\t" + i18n("Replay Gain failed. Exit code: %1").arg(exitCode) );
                    }
                }
            }
            if( item->childCount() <= 0 )
            {
                delete item;
                i--;
            }
        }
    }
    processNextFile();
}

void ReplayGainFileList::pluginLog( int id, const QString& message )
{
    logger->log( 1000, "\t" + message.trimmed().replace("\n","\n\t") );
}



