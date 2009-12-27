
#include "convert.h"
#include "config.h"
// #include "tagengine.h"
// #include "cdmanager.h"
#include "logger.h"
#include "filelist.h"
// #include "replaygainscanner.h"
#include "core/conversionoptions.h"
#include "core/codecplugin.h"
#include "outputdirectory.h"
#include "global.h"

#include <math.h>

#include <klocale.h>
#include <kglobal.h>
//#include <kdebug.h>
#include <ktemporaryfile.h>
#include <kio/job.h>
//#include <kprocess.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qtimer.h>

// Create a named pipe called pipe.wav
// #: mkfifo pipe.wav

//  Now execute the following command. It will hang, as it is waiting for the data to flow through the pipe.
// #: lame --preset standard pipe.wav output.file

//  In a different terminal, execute the following command, to get the data flowing.
// #: mplayer -ao pcm:file=pipe.wav input.file

ConvertItem::ConvertItem()
{
    // create a new item with the file list item pointer set to zero
    ConvertItem( (FileListItem*)0 );
}

ConvertItem::ConvertItem( FileListItem *item )
{
    fileListItem = item;
    getTime = convertTime = decodeTime = encodeTime = replaygainTime = bpmTime = 0.0f;
    int i=0;
    do {
        fifo.setUrl(QString("/tmp/sk_temp_%1").arg(i)); // TODO
        i++;
    } while( QFile::exists(fifo.toLocalFile()) );
    encodePlugin = 0;
    convertID = -1;
    replaygainID = -1;
    take = 0;
    killed = false;
    process = 0;
    kioJob = 0;
}

ConvertItem::~ConvertItem()
{}

void ConvertItem::updateTimes()
{
    getTime = ( mode & ConvertItem::get ) ? 0.8f : 0.0f;            // TODO file size? connection speed?
    convertTime = ( mode & ConvertItem::convert ) ? 1.4f : 0.0f;    // NOTE either convert OR decode & encode is used!
    if( fileListItem->track == -1 )
    {
        decodeTime = ( mode & ConvertItem::decode ) ? 0.4f : 0.0f;
        encodeTime = ( mode & ConvertItem::encode ) ? 1.0f : 0.0f;
    }
    else
    {
        decodeTime = ( mode & ConvertItem::decode ) ? 1.0f : 0.0f;  // TODO drive speed?
        encodeTime = ( mode & ConvertItem::encode ) ? 0.4f : 0.0f;
    }
    replaygainTime = ( mode & ConvertItem::replaygain ) ? 0.2f : 0.0f;
    bpmTime = ( mode & ConvertItem::bpm ) ? 0.2f : 0.0f;

    float sum = getTime + convertTime + decodeTime + encodeTime + replaygainTime + bpmTime;

    getTime *= fileListItem->time/sum;
    convertTime *= fileListItem->time/sum;
    decodeTime *= fileListItem->time/sum;
    encodeTime *= fileListItem->time/sum;
    replaygainTime *= fileListItem->time/sum;
    bpmTime *= fileListItem->time/sum;
    
    finishedTime = 0.0f;
    switch( state )
    {
        case ConvertItem::convert:
            if( mode & ConvertItem::get ) finishedTime += getTime;
            break;
        case ConvertItem::decode:
            if( mode & ConvertItem::get ) finishedTime += getTime;
            break;
        case ConvertItem::encode:
            if( mode & ConvertItem::get ) finishedTime += getTime;
            if( mode & ConvertItem::decode ) finishedTime += decodeTime;
            break;
        case ConvertItem::replaygain:
            if( mode & ConvertItem::get ) finishedTime += getTime;
            if( mode & ConvertItem::convert ) finishedTime += convertTime;
            if( mode & ConvertItem::decode ) finishedTime += decodeTime;
            if( mode & ConvertItem::encode ) finishedTime += encodeTime;
            break;
        case ConvertItem::bpm:
            if( mode & ConvertItem::get ) finishedTime += getTime;
            if( mode & ConvertItem::convert ) finishedTime += convertTime;
            if( mode & ConvertItem::decode ) finishedTime += decodeTime;
            if( mode & ConvertItem::encode ) finishedTime += encodeTime;
            if( mode & ConvertItem::replaygain ) finishedTime += replaygainTime;
            break;
    }
}

// =============
// class Convert
// =============

Convert::Convert( Config *_config, FileList *_fileList, Logger *_logger )
    : config( _config ),
    fileList( _fileList ),
    logger( _logger )
{
    connect( fileList, SIGNAL(convertItem(FileListItem*)), this, SLOT(add(FileListItem*)) );
    connect( fileList, SIGNAL(killItem(FileListItem*)), this, SLOT(kill(FileListItem*)) );
    connect( this, SIGNAL(finished(FileListItem*,int)), fileList, SLOT(itemFinished(FileListItem*,int)) );
    connect( this, SIGNAL(rippingFinished(const QString&)), fileList, SLOT(rippingFinished(const QString&)) );
    connect( this, SIGNAL(finishedProcess(int,int)), logger, SLOT(processCompleted(int,int)) );

    connect( &updateTimer, SIGNAL(timeout()), this, SLOT(updateProgress()) );

    QList<CodecPlugin*> codecPlugins = config->pluginLoader()->getAllCodecPlugins();
    for( int i=0; i<codecPlugins.size(); i++ )
    {
        connect( codecPlugins.at(i), SIGNAL(jobFinished(int,int)), this, SLOT(pluginProcessFinished(int,int)) );
        connect( codecPlugins.at(i), SIGNAL(log(int,const QString&)), this, SLOT(pluginLog(int,const QString&)) );
    }
    QList<ReplayGainPlugin*> replaygainPlugins = config->pluginLoader()->getAllReplayGainPlugins();
    for( int i=0; i<replaygainPlugins.size(); i++ )
    {
        connect( replaygainPlugins.at(i), SIGNAL(jobFinished(int,int)), this, SLOT(pluginProcessFinished(int,int)) );
        connect( replaygainPlugins.at(i), SIGNAL(log(int,const QString&)), this, SLOT(pluginLog(int,const QString&)) );
    }
    QList<RipperPlugin*> ripperPlugins = config->pluginLoader()->getAllRipperPlugins();
    for( int i=0; i<ripperPlugins.size(); i++ )
    {
        connect( ripperPlugins.at(i), SIGNAL(jobFinished(int,int)), this, SLOT(pluginProcessFinished(int,int)) );
        connect( ripperPlugins.at(i), SIGNAL(log(int,const QString&)), this, SLOT(pluginLog(int,const QString&)) );
    }
}

Convert::~Convert()
{}

void Convert::cleanUp()
{
    // TODO clean up
}

void Convert::get( ConvertItem *item )
{
    logger->log( item->logID, i18n("Getting file") );
    item->state = ConvertItem::get;

/*    item->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Getting file")+"... 00 %" );

    KUrl source( item->fileListItem->options.filePathName );
    KUrl destination( item->tempInFile->name() );

    if( source.isLocalFile() && destination.isLocalFile() ) {
        item->convertProcess->clearProgram();

        *(item->convertProcess) << "cp";
        *(item->convertProcess) << source.path();
        *(item->convertProcess) << destination.path();

        logger->log( item->logID, "cp \"" + source.path() + "\" \"" + destination.path() + "\"" );

/*        item->convertProcess->setPriority( config->data.general.priority );
        item->convertProcess->start( KProcess::NotifyOnExit, KProcess::AllOutput );*
    }
    else {
        item->moveJob = new KIO::FileCopyJob( source, destination, -1, false, true, false, false );
        connect( item->moveJob, SIGNAL(percent(KIO::Job*,unsigned long)),
                this, SLOT(moveProgress(KIO::Job*,unsigned long))
                );
        connect( item->moveJob, SIGNAL(result(KIO::Job*)),
                this, SLOT(moveFinished(KIO::Job*))
                );
    }*/
}

void Convert::convert( ConvertItem *item )
{
    if( !item ) return;
    ConversionOptions *conversionOptions = config->conversionOptionsManager()->getConversionOptions( item->fileListItem->conversionOptionsId );
    if( !conversionOptions ) return;

    if( item->take > item->conversionPipes.count() - 1 )
    {
        logger->log( item->logID, "\t" + i18n("No more backends left to try :(") );
        remove( item, -1 );
        return;
    }

    if( QFile::exists(item->fifo.toLocalFile()) )
    {
        QFile::remove(item->fifo.toLocalFile());
    }

    if( item->conversionPipes.at(item->take).trunks.count() == 1 ) // conversion can be done by one plugin alone
    {
        logger->log( item->logID, i18n("Converting") );
        item->state = ConvertItem::convert;
        item->convertPlugin = item->conversionPipes.at(item->take).trunks.at(0).plugin;
        if( item->convertPlugin->type() == "codec" )
        {
            bool replaygain = ( item->conversionPipes.at(item->take).trunks.at(0).data.hasInternalReplayGain && item->mode & ConvertItem::replaygain );
            item->convertID = qobject_cast<CodecPlugin*>(item->convertPlugin)->convert( item->inputUrl, item->outputUrl, item->conversionPipes.at(item->take).trunks.at(0).codecFrom, item->conversionPipes.at(item->take).trunks.at(0).codecTo, conversionOptions, item->fileListItem->tags, replaygain );
        }
        else if( item->convertPlugin->type() == "ripper" )
        {
            item->fileListItem->ripping = true;
            item->convertID = qobject_cast<RipperPlugin*>(item->convertPlugin)->rip( item->fileListItem->device, item->fileListItem->track, item->fileListItem->tracks, item->inputUrl );
        }
        if( !updateTimer.isActive() ) updateTimer.start( config->data.general.updateDelay );
        return;
    }
    else if( item->conversionPipes.at(item->take).trunks.count() == 2 ) // conversion needs two plugins
    {
        BackendPlugin *plugin1;
        CodecPlugin *plugin2;
        plugin1 = item->conversionPipes.at(item->take).trunks.at(0).plugin;
        plugin2 = qobject_cast<CodecPlugin*>(item->conversionPipes.at(item->take).trunks.at(1).plugin);
        
        QStringList command1, command2;
        if( plugin1->type() == "codec" )
        {
            command1 = qobject_cast<CodecPlugin*>(plugin1)->convertCommand( item->inputUrl, KUrl("-"), item->conversionPipes.at(item->take).trunks.at(0).codecFrom, item->conversionPipes.at(item->take).trunks.at(0).codecTo, conversionOptions, item->fileListItem->tags );
        }
        else if( plugin1->type() == "ripper" )
        {
            item->fileListItem->ripping = true;
            command1 = qobject_cast<RipperPlugin*>(plugin1)->ripCommand( item->fileListItem->device, item->fileListItem->track, item->fileListItem->tracks, KUrl("-") );
        }
        bool replaygain = ( item->conversionPipes.at(item->take).trunks.at(1).data.hasInternalReplayGain && item->mode & ConvertItem::replaygain );
        command2 = plugin2->convertCommand( KUrl("-"), item->outputUrl, item->conversionPipes.at(item->take).trunks.at(1).codecFrom, item->conversionPipes.at(item->take).trunks.at(1).codecTo, conversionOptions, item->fileListItem->tags, replaygain );
        if( !command1.isEmpty() && !command2.isEmpty() )
        {
            // both plugins support pipes
            logger->log( item->logID, i18n("Converting") );
            item->state = ConvertItem::convert;
            logger->log( item->logID, "\t" + command1.join(" ") + " | " + command2.join(" ") );
            item->process->clearProgram();
            item->process->setShellCommand( command1.join(" ") + " | " + command2.join(" ") );
            item->process->start();
        }
        else
        {
            // at least on plugins doesn't support pipes
            // decode with plugin1
            logger->log( item->logID, i18n("Decoding") );
            item->state = ConvertItem::decode;
            item->mode = ConvertItem::Mode( item->mode ^ ConvertItem::convert );
            item->mode = ConvertItem::Mode( item->mode | ConvertItem::decode | ConvertItem::encode );
            item->updateTimes();
            item->convertPlugin = plugin1;
            item->encodePlugin = plugin2;
            item->fifo.setUrl( item->fifo.url() + "." + config->pluginLoader()->codecExtensions(item->conversionPipes.at(item->take).trunks.at(0).codecTo).first() );
            if( plugin1->type() == "codec" )
            {
                item->convertID = qobject_cast<CodecPlugin*>(plugin1)->convert( item->inputUrl, item->fifo, item->conversionPipes.at(item->take).trunks.at(0).codecFrom, item->conversionPipes.at(item->take).trunks.at(0).codecTo, conversionOptions, item->fileListItem->tags );
            }
            else if( plugin1->type() == "ripper" )
            {
                item->fileListItem->ripping = true;
                item->convertID = qobject_cast<RipperPlugin*>(plugin1)->rip( item->fileListItem->device, item->fileListItem->track, item->fileListItem->tracks, item->fifo );
            }
        }
        if( !updateTimer.isActive() ) updateTimer.start( config->data.general.updateDelay );
        return;
    }
}

void Convert::encode( ConvertItem *item )
{
    // file has been decoded by plugin1 last time this function was executed
    if( !item ) return;
    if( !item->encodePlugin ) return;
    ConversionOptions *conversionOptions = config->conversionOptionsManager()->getConversionOptions( item->fileListItem->conversionOptionsId );
    if( !conversionOptions ) return;

    logger->log( item->logID, i18n("Encoding") );
    item->state = ConvertItem::encode;
    item->convertPlugin = item->encodePlugin;
    item->encodePlugin = 0;
    bool replaygain = ( item->conversionPipes.at(item->take).trunks.at(1).data.hasInternalReplayGain && item->mode & ConvertItem::replaygain );
    item->convertID = qobject_cast<CodecPlugin*>(item->convertPlugin)->convert( item->fifo, item->outputUrl, item->conversionPipes.at(item->take).trunks.at(1).codecFrom, item->conversionPipes.at(item->take).trunks.at(1).codecTo, conversionOptions, item->fileListItem->tags, replaygain );
}

void Convert::replaygain( ConvertItem *item )
{
    if( !item ) return;

    logger->log( item->logID, i18n("Applying Replay Gain") );

    if( item->take > item->replaygainPipes.count() - 1 )
    {
        logger->log( item->logID, "\t" + i18n("No more backends left to try :(") );
        remove( item, -1 );
        return;
    }

    item->state = ConvertItem::replaygain;
    item->replaygainPlugin = item->replaygainPipes.at(item->take).plugin;
    item->replaygainID = item->replaygainPlugin->apply( item->outputUrl );
    
    if( !updateTimer.isActive() ) updateTimer.start( config->data.general.updateDelay );
}

void Convert::writeTags( ConvertItem *item )
{
    logger->log( item->logID, i18n("Writing tags") );
    item->state = ConvertItem::write_tags;

//     if( item->mode & ConvertItem::encode ) {
//         tagEngine->writeTags( item->tempOutFile->name(), item->fileListItem->tags );
//         item->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Writing tags")+"... 00 %" );
//     }

    executeNextStep( item );
}

void Convert::executeUserScript( ConvertItem *item )
{
    logger->log( item->logID, i18n("Running user script") );
    item->state = ConvertItem::execute_userscript;

//     KUrl source( item->fileListItem->options.filePathName );
//     KUrl destination( item->outputFilePathName );
// 
//     item->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Running user script")+"... 00 %" );
// 
//     item->convertProcess->clearProgram();
// 
//     QString userscript = locate( "data", "soundkonverter/userscript.sh" );
//     if( userscript == "" ) executeNextStep( item );
// 
//     *(item->convertProcess) << userscript;
//     *(item->convertProcess) << source.path();
//     *(item->convertProcess) << destination.path();
// 
//     logger->log( item->logID, userscript + " \"" + source.path() + "\" \"" + destination.path() + "\"" );

// // /*    item->convertProcess->setPriority( config->data.general.priority );
// //     item->convertProcess->start( KProcess::NotifyOnExit, KProcess::AllOutput );*/
}

void Convert::executeNextStep( ConvertItem *item )
{
    logger->log( item->logID, i18n("Executing next step") );

    int take = item->take;
    item->take = 0;
    item->progress = 0.0f;

    switch( item->state )
    {
        case ConvertItem::get:
            if( item->mode & ConvertItem::convert ) convert(item);
            else remove( item, 0 );
            break;
        case ConvertItem::convert:
            if( item->mode & ConvertItem::replaygain ) replaygain(item);
//             else if( item->mode & ConvertItem::bpm ) bpm(item);
            else remove( item, 0 );
            break;
        case ConvertItem::decode:
            if( item->mode & ConvertItem::encode ) { item->take = take; encode(item); }
            else if( item->mode & ConvertItem::replaygain ) replaygain(item);
//             else if( item->mode & ConvertItem::bpm ) bpm(item);
            else remove( item, 0 );
            break;
        case ConvertItem::encode:
            if( item->mode & ConvertItem::replaygain ) replaygain(item);
//             else if( item->mode & ConvertItem::bpm ) bpm(item);
            else remove( item, 0 );
            break;
        case ConvertItem::replaygain:
//             if( item->mode & ConvertItem::bpm ) bpm(item);
            /*else*/ remove( item, 0 );
            break;
        case ConvertItem::bpm:
            remove( item, 0 );
            break;
        default:
            if( item->mode & ConvertItem::get ) get(item);
            else if( item->mode & ConvertItem::convert ) convert(item);
            else if( item->mode & ConvertItem::replaygain ) replaygain(item);
            else remove( item, 0 );
            break;
    }
}

void Convert::executeSameStep( ConvertItem *item )
{
    item->take++;
    item->progress = 0.0f;
  
    switch( item->state )
    {
        case ConvertItem::get:
            get(item);
            return;
        case ConvertItem::convert:
            convert(item);
            return;
        case ConvertItem::decode:
            convert(item);
            return;
        case ConvertItem::encode: // TODO try next encoder instead of decoding again
            convert(item);
            return;
        case ConvertItem::replaygain:
            replaygain(item);
            return;
//         case ConvertItem::bpm:
//             bpm(item);
//             return;
    }
    
    remove( item, -1 ); // shouldn't be possible
}

void Convert::kioJobProgress( KIO::Job* job, unsigned long percent )
{
    // search the item list for our item
    for( QList<ConvertItem*>::Iterator item = items.begin(); item != items.end(); item++ ) {
        if( (*item)->kioJob == job ) {
            (*item)->progress = (float)percent;
        }
    }
}

void Convert::kioJobFinished( KIO::Job* job )
{
    // search the item list for our item
    for( QList<ConvertItem*>::Iterator item = items.begin(); item != items.end(); item++ ) {
        if( (*item)->kioJob == job ) {
        }
    }
}

void Convert::processOutput()
{
    QString output;
    float progress1, progress2;
    
    for( int i=0; i<items.size(); i++ )
    {
        if( items.at(i)->process == QObject::sender() )
        {
            // TODO more cases
            if( items.at(i)->conversionPipes.at(items.at(i)->take).trunks.count() == 1 )
            {
                BackendPlugin *plugin1;
                plugin1 = items.at(i)->conversionPipes.at(items.at(i)->take).trunks.at(0).plugin;
                output = items.at(i)->process->readAllStandardOutput().data();
                progress1 = plugin1->parseOutput( output );
                if( progress1 > items.at(i)->progress ) items[i]->progress = progress1;
                if( progress1 == -1 && !output.simplified().isEmpty() ) logger->log( items.at(i)->logID, "\t" + output.trimmed() );
            }
            else if( items.at(i)->conversionPipes.at(items.at(i)->take).trunks.count() == 2 )
            {
                // NOTE we can only use the decoder's progress because the encoder encodes a stream (with to the encoder unknown length)
                BackendPlugin *plugin1, *plugin2;
                plugin1 = items.at(i)->conversionPipes.at(items.at(i)->take).trunks.at(0).plugin;
                plugin2 = items.at(i)->conversionPipes.at(items.at(i)->take).trunks.at(1).plugin;
                output = items.at(i)->process->readAllStandardOutput().data();
                progress1 = plugin1->parseOutput( output );
                progress2 = plugin2->parseOutput( output );
                if( progress1 > items.at(i)->progress ) items[i]->progress = progress1;
                if( progress1 == -1 && progress2 == -1 && !output.simplified().isEmpty() ) logger->log( items.at(i)->logID, "\t" + output.trimmed() );
            }

            return;
        }
    }
}

void Convert::processExit( int exitCode, QProcess::ExitStatus exitStatus )
{
    // search the item list for our item
    for( int i=0; i<items.size(); i++ )
    {
        if( items.at(i)->process == QObject::sender() )
        {
            if( items.at(i)->killed )
            {
                // TODO clean up temp files, pipes, etc.
                remove( items.at(i), 1 );
                return;
            }

            if( exitCode == 0 )
            {
                float fileTime;
                switch( items.at(i)->state )
                {
                    case ConvertItem::convert: fileTime = items.at(i)->convertTime; break;
                    case ConvertItem::decode: fileTime = items.at(i)->decodeTime; break;
                    case ConvertItem::encode: fileTime = items.at(i)->encodeTime; break;
                    case ConvertItem::replaygain: fileTime = items.at(i)->replaygainTime; break;
                    case ConvertItem::bpm: fileTime = items.at(i)->bpmTime; break;
                    default: fileTime = 0.0f;
                }
                items.at(i)->finishedTime += fileTime;
                if( items.at(i)->state == ConvertItem::decode && items.at(i)->convertPlugin->type() == "ripper" )
                {
                    items.at(i)->fileListItem->ripping = false;
                    emit rippingFinished( items.at(i)->fileListItem->device );
                }
                if( items.at(i)->conversionPipes.at(items.at(i)->take).trunks.at(0).data.hasInternalReplayGain && items.at(i)->mode & ConvertItem::replaygain )
                {
                    items[i]->mode = ConvertItem::Mode( items[i]->mode ^ ConvertItem::replaygain );
                }
                if( items.at(i)->state == ConvertItem::decode )
                {
                    encode( items.at(i) );
                }
                else
                {
                    executeNextStep( items.at(i) );
                }
            }
            else
            {
                if( QFile::exists(items.at(i)->outputUrl.toLocalFile()) )
                {
                    QFile::remove(items.at(i)->outputUrl.toLocalFile());
                }
                logger->log( items.at(i)->logID, "\t" + i18n("Conversion failed. Exit code: %1").arg(exitCode) );
                executeSameStep( items.at(i) );
            }
        }
    }
}

void Convert::pluginProcessFinished( int id, int exitCode )
{
    for( int i=0; i<items.size(); i++ )
    {
        if( ( items.at(i)->convertPlugin && items.at(i)->convertPlugin == QObject::sender() && items.at(i)->convertID == id ) ||
              items.at(i)->replaygainPlugin && items.at(i)->replaygainPlugin == QObject::sender() && items.at(i)->replaygainID == id )
        {
            if( items.at(i)->killed )
            {
                // TODO clean up temp files, pipes, etc.
                remove( items.at(i), 1 );
                return;
            }

            if( exitCode == 0 )
            {
                float fileTime;
                switch( items.at(i)->state )
                {
                    case ConvertItem::convert: fileTime = items.at(i)->convertTime; break;
                    case ConvertItem::decode: fileTime = items.at(i)->decodeTime; break;
                    case ConvertItem::encode: fileTime = items.at(i)->encodeTime; break;
                    case ConvertItem::replaygain: fileTime = items.at(i)->replaygainTime; break;
                    case ConvertItem::bpm: fileTime = items.at(i)->bpmTime; break;
                    default: fileTime = 0.0f;
                }
                items.at(i)->finishedTime += fileTime;
                items.at(i)->convertID = -1;
                items.at(i)->replaygainID = -1;
                if( items.at(i)->state == ConvertItem::decode && items.at(i)->convertPlugin->type() == "ripper" )
                {
                    items.at(i)->fileListItem->ripping = false;
                    emit rippingFinished( items.at(i)->fileListItem->device );
                }
                if( items.at(i)->conversionPipes.at(items.at(i)->take).trunks.at(0).data.hasInternalReplayGain && items.at(i)->mode & ConvertItem::replaygain )
                {
                    items[i]->mode = ConvertItem::Mode( items[i]->mode ^ ConvertItem::replaygain );
                }
                executeNextStep( items.at(i) );
            }
            else
            {
                if( QFile::exists(items.at(i)->outputUrl.toLocalFile()) )
                {
                    QFile::remove(items.at(i)->outputUrl.toLocalFile());
                }
                logger->log( items.at(i)->logID, "\t" + i18n("Conversion failed. Exit code: %1").arg(exitCode) );
                executeSameStep( items.at(i) );
            }
        }
    }
}

void Convert::pluginLog( int id, const QString& message )
{
    if( message.trimmed().isEmpty() ) return;
  
    for( int i=0; i<items.size(); i++ )
    {
        if( ( items.at(i)->convertPlugin && items.at(i)->convertPlugin == QObject::sender() && items.at(i)->convertID == id ) ||
              items.at(i)->replaygainPlugin && items.at(i)->replaygainPlugin == QObject::sender() && items.at(i)->replaygainID == id )
        {
            logger->log( items.at(i)->logID, "\t" + message.trimmed().replace("\n","\n\t") );
            return;
        }
    }
    
    logger->log( 1000, qobject_cast<BackendPlugin*>(QObject::sender())->name() + ": " + message.trimmed().replace("\n","\n\t") );
}

void Convert::add( FileListItem* item )
{
    KUrl fileName;
    if( item->track >= 0 )
    {
        if( item->tags )
        {
            fileName = KUrl( i18n("CD track") + " " + QString().sprintf("%02i",item->tags->track) + ": " + item->tags->artist + " - " + item->tags->title );
        }
        else // shouldn't be possible
        {
            fileName = KUrl( i18n("CD track %1").arg(item->track) );
        }
    }
    else
    {
        fileName = item->url;
    }
    logger->log( 1000, i18n("Adding new item to conversion list: '%1'").arg(fileName.pathOrUrl()) );

    // append the item to the item list and store the iterator
    ConvertItem *newItem = new ConvertItem( item );
    items.append( newItem );

    // register at the logger
    newItem->logID = logger->registerProcess( fileName );
    logger->log( 1000, "\t" + i18n("Got log ID: %1").arg(newItem->logID) );

//     logger->log( newItem->logID, "Mime Type: " + newItem->fileListItem->mimeType );
//     if( newItem->fileListItem->tags ) logger->log( newItem->logID, i18n("Tags successfully read") );
//     else logger->log( newItem->logID, i18n("Reading tags failed") );

    // set some variables to default values
    newItem->mode = (ConvertItem::Mode)0x0000;
    newItem->state = (ConvertItem::Mode)0x0000;

    newItem->inputUrl = item->url;
    newItem->outputUrl = ( !item->outputUrl.url().isEmpty() ) ? item->outputUrl : OutputDirectory::calcPath( item, config );

    // connect moveProcess of our new item with the slots of Convert
    newItem->process = new KProcess();
    newItem->process->setOutputChannelMode( KProcess::MergedChannels );
    connect( newItem->process, SIGNAL(readyRead()), this, SLOT(processOutput()) );
    connect( newItem->process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processExit(int,QProcess::ExitStatus)) );

    ConversionOptions *conversionOptions = config->conversionOptionsManager()->getConversionOptions(item->conversionOptionsId);
    if( !conversionOptions )
    {
        logger->log( 1000, "Convert::add(...) no ConversionOptions found" );
        remove( newItem, -1 );
        return;
    }
    
    if( item->track >= 0 )
    {
        logger->log( newItem->logID, "\tTrack#: " + QString::number(item->track) + ", device: " + item->device );
    }

    newItem->conversionPipes = config->pluginLoader()->getConversionPipes( item->codecName, conversionOptions->codecName, conversionOptions->pluginName );

        // NOTE debug
        logger->log( newItem->logID, "\tBuilding pipes ..." );

        for( int i=0; i<newItem->conversionPipes.size(); i++ )
        {
            QString pipe_str;
          
            for( int j = 0; j < newItem->conversionPipes.at(i).trunks.size(); j++ )
            {
                pipe_str += newItem->conversionPipes.at(i).trunks.at(j).codecFrom + " -> " + newItem->conversionPipes.at(i).trunks.at(j).codecTo + " ( " + newItem->conversionPipes.at(i).trunks.at(j).plugin->name() + " ) , ";
            }

            logger->log( newItem->logID, "\t\t" + pipe_str );
        }

        logger->log( newItem->logID, "\t... pipes built" );
        // debug end

    newItem->mode = ConvertItem::Mode( newItem->mode | ConvertItem::convert );

    if( conversionOptions->replaygain )
    {
        newItem->replaygainPipes = config->pluginLoader()->getReplayGainPipes( conversionOptions->codecName );
        newItem->mode = ConvertItem::Mode( newItem->mode | ConvertItem::replaygain );
    }

    if( !newItem->inputUrl.isLocalFile() && item->track == -1 ) newItem->mode = ConvertItem::Mode( newItem->mode | ConvertItem::get );
    
    newItem->updateTimes();
    
    // (visual) feedback
    item->converting = true;
    
    newItem->progressedTime.start();

    // and start
    executeNextStep( newItem );
}

void Convert::remove( ConvertItem *item, int state )
{
    // TODO "remove" (re-add) the times to the progress indicator
    //emit uncountTime( item->getTime + item->getCorrectionTime + item->ripTime +
    //                  item->decodeTime + item->encodeTime + item->replaygainTime );

    QString exitMessage;
    if( state == 0 ) exitMessage = i18n("Normal exit");
    else if( state == 1 ) exitMessage = i18n("Aborted by the user");
    else exitMessage = i18n("An error occured");
    logger->log( item->logID, i18n("Removing file from conversion list. Exit code %1 (%2)").arg(state).arg(exitMessage) );
    
    logger->log( item->logID, "\tprogressedTime: " + QString::number(item->progressedTime.elapsed()) );

    emit timeFinished( item->finishedTime );

/*    if( item->fileListItem->notify != "" ) {
        QString command = item->fileListItem->notify;
        command.replace( "%u", item->fileListItem->url );
        command.replace( "%i", item->fileListItem->options.filePathName.replace(" ","%20") );
        command.replace( "%o", item->outputFilePathName.replace(" ","%20") );
        logger->log( item->logID, " "+i18n("Executing command: \"%1\"").arg(command) );
        notify.clearArguments();
        QString paramSplinter;
        // FIXME split correct (strings with spaces are splited by mistake)
        // FIXME only one command can be executed at once!?
        QStringList params = QStringList::split( ' ', item->fileListItem->notify );
        for( QStringList::Iterator it = params.begin(); it != params.end(); ++it )
        {
            paramSplinter = *it;
            paramSplinter.replace( "%u", item->fileListItem->url );
            paramSplinter.replace( "%i", item->fileListItem->options.filePathName );
            paramSplinter.replace( "%o", item->outputFilePathName );
            notify << paramSplinter;
        }
        notify.start( KProcess::DontCare );
    }
*/
    item->fileListItem->converting = false;
    emit finished( item->fileListItem, state ); // send signal to FileList
    emit finishedProcess( item->logID, state ); // send signal to Logger

    if( QFile::exists(item->fifo.toLocalFile()) )
    {
        QFile::remove(item->fifo.toLocalFile());
    }

    item->fileListItem = 0;
    if( item->process != 0 ) delete item->process;
    item->process = 0;
    if( item->kioJob != 0 ) delete item->kioJob;
    item->kioJob = 0;

    items.removeAll( item );

    delete item;
    item = 0;

    if( items.size() == 0 ) updateTimer.stop();
}

void Convert::kill( FileListItem *item )
{
    for( int i=0; i<items.size(); i++ )
    {
        if( items.at(i)->fileListItem == item )
        {
            items.at(i)->killed = true;
            if( items.at(i)->convertID != -1 ) items.at(i)->convertPlugin->kill( items.at(i)->convertID );
            else if( items.at(i)->replaygainID != -1 ) items.at(i)->replaygainPlugin->kill( items.at(i)->convertID );
            else if( items.at(i)->process != 0 ) items.at(i)->process->kill();
        }
    }
}

/*void Convert::kill()
{
    for( int i=0; i<items.size(); i++ )
    {
        if( items.at(i)->convertID != -1 )
        {
            items.at(i)->convertPlugin->stop( items.at(i)->convertID );
        }
        else
        {
            stop( items.at(i)->fileListItem );
        }
    }
}*/

void Convert::updateProgress()
{
    float time = 0;
    float fileTime;
    float fileProgress;
    for( int i=0; i<items.size(); i++ )
    {
        if( items.at(i)->convertID != -1 )
        {
            fileProgress = items.at(i)->convertPlugin->progress( items.at(i)->convertID );
        }
        else if( items.at(i)->replaygainID != -1 )
        {
            fileProgress = items.at(i)->replaygainPlugin->progress( items.at(i)->replaygainID );
        }
        else
        {
            fileProgress = items.at(i)->progress;
        }
        
        switch( items.at(i)->state )
        {
            case ConvertItem::get:
                fileTime = items.at(i)->getTime;
                items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Getting file")+"... "+Global::prettyNumber(fileProgress,"%") );
                break;
            case ConvertItem::convert:
                fileTime = items.at(i)->convertTime;
                items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Converting")+"... "+Global::prettyNumber(fileProgress,"%") );
                break;
            case ConvertItem::decode:
                fileTime = items.at(i)->decodeTime;
                if( items.at(i)->convertPlugin->type() == "convert" ) items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Decoding")+"... "+Global::prettyNumber(fileProgress,"%") );
                else if( items.at(i)->convertPlugin->type() == "ripper" ) items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Ripping")+"... "+Global::prettyNumber(fileProgress,"%") );
                break;
            case ConvertItem::encode:
                fileTime = items.at(i)->encodeTime;
                items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Encoding")+"... "+Global::prettyNumber(fileProgress,"%") );
                break;
            case ConvertItem::replaygain:
                fileTime = items.at(i)->replaygainTime;
                items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Replay Gain")+"... "+Global::prettyNumber(fileProgress,"%") );
                break;
//             case ConvertItem::bpm:
//                 items.at(i)->fileListItem->setText( fileList->columnByName(i18n("State")), i18n("Calculating BPM")+"... "+Global::prettyNumber(fileProgress,"%") );
//                 fileTime = items.at(i)->bpmTime;
//                 break;
            default: fileTime = 0.0f;
        }
//         fileList->repaint( fileList->visualItemRect(items.at(i)->fileListItem) );
        time += items.at(i)->finishedTime + fileProgress * fileTime / 100.0f;
        logger->log( items.at(i)->logID, QString("Progress: %1").arg(fileProgress) );
    }
    emit updateTime( time );
}

