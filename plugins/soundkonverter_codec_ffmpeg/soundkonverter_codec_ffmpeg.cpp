
#include "ffmpegcodecglobal.h"

#include "soundkonverter_codec_ffmpeg.h"
#include "../../core/conversionoptions.h"
#include "ffmpegcodecwidget.h"


soundkonverter_codec_ffmpeg::soundkonverter_codec_ffmpeg( QObject *parent, const QStringList& args  )
    : CodecPlugin( parent )
{
    binaries["ffmpeg"] = "";
    
    codecMap["wav"] = "pcm_s16le";
    codecMap["ogg vorbis"] = "vorbis";
    codecMap["mp3"] = "libmp3lame";
    codecMap["flac"] = "flac";
    codecMap["wma"] = "wmav2";
}

soundkonverter_codec_ffmpeg::~soundkonverter_codec_ffmpeg()
{}

QString soundkonverter_codec_ffmpeg::name()
{
    return global_plugin_name;
}

QList<ConversionPipeTrunk> soundkonverter_codec_ffmpeg::codecTable()
{
    QList<ConversionPipeTrunk> table;
    
    fromCodecs += "wav";
    fromCodecs += "ogg vorbis";
    fromCodecs += "mp3";
    fromCodecs += "flac";
    fromCodecs += "wma";

    toCodecs += "wav";
    toCodecs += "ogg vorbis";
    toCodecs += "mp3";
    toCodecs += "flac";
    toCodecs += "wma";
    
    for( int i=0; i<fromCodecs.count(); i++ )
    {
        for( int j=0; j<toCodecs.count(); j++ )
        {
            if( fromCodecs.at(i) == "wav" && toCodecs.at(j) == "wav" ) continue;
          
            ConversionPipeTrunk newTrunk;
            newTrunk.codecFrom = fromCodecs.at(i);
            newTrunk.codecTo = toCodecs.at(j);
            newTrunk.rating = 90;
            newTrunk.enabled = ( binaries["ffmpeg"] != "" );
            newTrunk.problemInfo = i18n("You need to install 'ffmpeg'.\nSince ffmpeg inludes many patented codecs, it may not be included in the default installation of your distribution.\nMany distributions offer ffmpeg in an additional software repository.");
            newTrunk.data.hasInternalReplayGain = false;
            table.append( newTrunk );
        }
    }

    QSet<QString> codecs;
    codecs += QSet<QString>::fromList(fromCodecs);
    codecs += QSet<QString>::fromList(toCodecs);
    allCodecs = codecs.toList();
    
/*
    newTrunk.codecFrom = "wav";
    newTrunk.codecTo = "ogg vorbis";
    newTrunk.rating = 90;
    newTrunk.enabled = ( binaries["ffmpeg"] != "" );
    newTrunk.problemInfo = i18n("In order to encode ogg vorbis files, you need to install 'ffmpeg'.\nSince ffmpeg inludes many patented codecs, it may not be included in the default installation of your distribution.\nMany distributions offer ffmpeg in an additional software repository.");
    newTrunk.data.hasInternalReplayGain = false;
    table.append( newTrunk );

    newTrunk.codecFrom = "ogg vorbis";
    newTrunk.codecTo = "wav";
    newTrunk.rating = 90;
    newTrunk.enabled = ( binaries["ffmpeg"] != "" );
    newTrunk.problemInfo = i18n("In order to decode ogg vorbis files, you need to install 'ffmpeg'.\nSince ffmpeg inludes many patented codecs, it may not be included in the default installation of your distribution.\nMany distributions offer ffmpeg in an additional software repository.");
    newTrunk.data.hasInternalReplayGain = false;
    table.append( newTrunk );

    newTrunk.codecFrom = "wav";
    newTrunk.codecTo = "mp3";
    newTrunk.rating = 90;
    newTrunk.enabled = ( binaries["ffmpeg"] != "" );
    newTrunk.problemInfo = i18n("In order to encode mp3 files, you need to install 'ffmpeg'.\nSince ffmpeg inludes many patented codecs, it may not be included in the default installation of your distribution.\nMany distributions offer ffmpeg in an additional software repository.");
    newTrunk.data.hasInternalReplayGain = false;
    table.append( newTrunk );

    newTrunk.codecFrom = "mp3";
    newTrunk.codecTo = "wav";
    newTrunk.rating = 90;
    newTrunk.enabled = ( binaries["ffmpeg"] != "" );
    newTrunk.problemInfo = i18n("In order to decode mp3 files, you need to install 'ffmpeg'.\nSince ffmpeg inludes many patented codecs, it may not be included in the default installation of your distribution.\nMany distributions offer ffmpeg in an additional software repository.");
    newTrunk.data.hasInternalReplayGain = false;
    table.append( newTrunk );
*/
    return table;
}

BackendPlugin::FormatInfo soundkonverter_codec_ffmpeg::formatInfo( const QString& codecName )
{
    BackendPlugin::FormatInfo info;
    info.codecName = codecName;

    if( codecName == "ogg vorbis" )
    {
        info.lossless = false;
        info.description = i18n("Ogg Vorbis is a free and lossy high quality audio codec.\nFor more information see: http://www.xiph.org/vorbis/");
        info.mimeTypes.append( "application/ogg" );
        info.mimeTypes.append( "audio/vorbis" );
        info.mimeTypes.append( "application/x-ogg" );
        info.mimeTypes.append( "audio/ogg" );
        info.mimeTypes.append( "audio/x-vorbis+ogg" );
        info.extensions.append( "ogg" );
    }
    else if( codecName == "mp3" )
    {
        info.lossless = false;
        info.description = i18n("MP3 is a very popular lossy audio codec.");
        info.mimeTypes.append( "audio/x-mp3" );
        info.mimeTypes.append( "audio/mpeg" );
        info.mimeTypes.append( "audio/mp3" );
        info.extensions.append( "mp3" );
    }
    else if( codecName == "flac" )
    {
        info.lossless = true;
        info.description = i18n("Flac is the free lossless audio codec.\nAs it name says, it compresses without any loss.");
        info.mimeTypes.append( "audio/x-flac" );
        info.mimeTypes.append( "audio/x-flac+ogg" );
        info.mimeTypes.append( "audio/x-oggflac" );
        info.extensions.append( "flac" );
        info.extensions.append( "fla" );
        info.extensions.append( "ogg" );
    }
    else if( codecName == "wma" )
    {
        info.lossless = false;
        info.description = i18n("Windows Media Audio is a propritary audio codec from Microsoft.");
        info.mimeTypes.append( "audio/x-ms-wma" );
        info.extensions.append( "wma" );
    }
    else if( codecName == "wav" )
    {
        info.lossless = true;
        info.description = i18n("Wave won't compress the audio stream.");
        info.mimeTypes.append( "audio/x-wav" );
        info.mimeTypes.append( "audio/wav" );
        info.extensions.append( "wav" );
    }

    return info;
}


QString soundkonverter_codec_ffmpeg::getCodecFromFile( const KUrl& filename, const QString& mimeType )
{
    for( int i=0; i<allCodecs.count(); i++ )
    {
        if( formatInfo(allCodecs.at(i)).mimeTypes.indexOf(mimeType) != -1 )
        {
            return allCodecs.at(i);
        }
    }
    
    QString extension = filename.url().right( filename.url().length() - filename.url().lastIndexOf(".") - 1 );

    for( int i=0; i<allCodecs.count(); i++ )
    {
        if( formatInfo(allCodecs.at(i)).extensions.indexOf(extension) != -1 )
        {
            return allCodecs.at(i);
        }
    }
        
/*    
    if( formatInfo("ogg vorbis").mimeTypes.indexOf(mimeType) != -1 )
    {
        return "ogg vorbis";
    }
    else if( formatInfo("mp3").mimeTypes.indexOf(mimeType) != -1 )
    {
        return "mp3";
    }
    else if( formatInfo("wav").mimeTypes.indexOf(mimeType) != -1 )
    {
        return "wav";
    }
    else if( mimeType == "application/octet-stream" )
    {
        extension = filename.url().right( filename.url().length() - filename.url().lastIndexOf(".") - 1 );
        if( formatInfo("ogg vorbis").extensions.indexOf(extension) != -1 ) return "ogg vorbis";
        else if( formatInfo("mp3").extensions.indexOf(extension) != -1 ) return "mp3";
        else if( formatInfo("flac").extensions.indexOf(extension) != -1 ) return "flac";
        else if( formatInfo("wma").extensions.indexOf(extension) != -1 ) return "wma";
        else if( formatInfo("wav").extensions.indexOf(extension) != -1 ) return "wav";
//         if( filename.url().endsWith(".ogg") ) return "ogg vorbis";
//         if( filename.url().endsWith(".mp3") ) return "mp3";
//         if( filename.url().endsWith(".wav") ) return "wav";
    }
*/
    return "";
}

bool soundkonverter_codec_ffmpeg::isConfigSupported( ActionType action )
{
    return false;
}

bool soundkonverter_codec_ffmpeg::showConfigDialog( ActionType action, const QString& format, QWidget *parent )
{}

bool soundkonverter_codec_ffmpeg::hasInfo()
{
    return false;
}

void soundkonverter_codec_ffmpeg::showInfo()
{}

QWidget *soundkonverter_codec_ffmpeg::newCodecWidget()
{
    FFmpegCodecWidget *widget = new FFmpegCodecWidget();
    if( lastUsedConversionOptions )
    {
        widget->setCurrentConversionOptions( lastUsedConversionOptions );
        delete lastUsedConversionOptions;
        lastUsedConversionOptions = 0;
    }
    return qobject_cast<QWidget*>(widget);
}

int soundkonverter_codec_ffmpeg::convert( const KUrl& inputFile, const KUrl& outputFile, const QString& inputCodec, const QString& outputCodec, ConversionOptions *_conversionOptions, TagData *tags, bool replayGain )
{
    QStringList command;
    ConversionOptions *conversionOptions = _conversionOptions;

    if( outputCodec != "wav" )
    {
        command += "ffmpeg";
        command += "-i";
        command += "\"" + inputFile.toLocalFile() + "\"";
        command += "-acodec";
        command += codecMap[conversionOptions->codecName];
        command += "-ab";
        command += QString::number(conversionOptions->bitrate) + "k";
        if( conversionOptions->samplingRate > 0 )
        {
            command += "-ar";
            command += QString::number(conversionOptions->samplingRate);
        }
        if( conversionOptions->channels > 0 )
        {
            command += "-ac 1";
        }
        command += "\"" + outputFile.toLocalFile() + "\"";
    }
    else
    {
        command += "ffmpeg";
        command += "-i";
        command += "\"" + inputFile.toLocalFile() + "\"";
        command += "\"" + outputFile.toLocalFile() + "\"";
    }

    CodecPluginItem *newItem = new CodecPluginItem( this );
    newItem->id = lastId++;
    newItem->process = new KProcess( newItem );
    newItem->process->setOutputChannelMode( KProcess::MergedChannels );
    connect( newItem->process, SIGNAL(readyRead()), this, SLOT(processOutput()) );
    connect( newItem->process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processExit(int,QProcess::ExitStatus)) );

    newItem->process->clearProgram();
    newItem->process->setShellCommand( command.join(" ") );
    newItem->process->start();

    emit log( 1000, command.join(" ") );

    backendItems.append( newItem );
    return newItem->id;
}

QStringList soundkonverter_codec_ffmpeg::convertCommand( const KUrl& inputFile, const KUrl& outputFile, const QString& inputCodec, const QString& outputCodec, ConversionOptions *_conversionOptions, TagData *tags, bool replayGain )
{
    if( !_conversionOptions ) return QStringList();
    
    QStringList command;
    ConversionOptions *conversionOptions = _conversionOptions;

    if( conversionOptions->codecName == "wav" )
    {
        command += "ffmpeg";
        command += "-i";
        command += "\"" + inputFile.toLocalFile() + "\"";
        command += "\"" + outputFile.toLocalFile() + "\"";
    }

    return command;
}

float soundkonverter_codec_ffmpeg::parseOutput( const QString& output, int *length )
{
    // size=    1508kB time=48.25 bitrate= 256.0kbits/s
    
    QString data = output;
    QString time;
    
    QRegExp reg("(\\d{2,}):(\\d{2}):(\\d{2})\\.(\\d{2})");
    if( length && data.contains(reg) )
    {
        *length = reg.cap(1).toInt()*3600 + reg.cap(2).toInt()*60 + reg.cap(3).toInt();
//         emit log( 1000, "got length: " + QString::number(*length) );
    }
    if( data.contains("time") )
    {
        data.remove( 0, data.indexOf("time")+5 );
        time = data.left( data.indexOf(" ") );
        return time.toFloat();
    }
    
    return -1;
}

float soundkonverter_codec_ffmpeg::parseOutput( const QString& output )
{
    return parseOutput( output, 0 );
}

void soundkonverter_codec_ffmpeg::processOutput()
{
    CodecPluginItem *pluginItem;
    float progress;
    for( int i=0; i<backendItems.size(); i++ )
    {
        if( backendItems.at(i)->process == QObject::sender() )
        {
            QString output = backendItems.at(i)->process->readAllStandardOutput().data();
            pluginItem = qobject_cast<CodecPluginItem*>(backendItems.at(i));
            progress = parseOutput( output, &pluginItem->data.length );
            if( progress == -1 && !output.simplified().isEmpty() ) emit log( backendItems.at(i)->id, output );
            progress = progress * 100 / pluginItem->data.length;
            if( progress > backendItems.at(i)->progress ) backendItems.at(i)->progress = progress;
            return;
        }
    }
}

#include "soundkonverter_codec_ffmpeg.moc"

