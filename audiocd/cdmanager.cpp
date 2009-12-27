
#include "cdmanager.h"
#include "paranoia.h"
// #include "cddb.h"
// #include "conversionoptions.h"

// #include <qstringlist.h>
// #include <qvaluelist.h>

#include <klocale.h>
#include <kmessagebox.h>
// #include <kinputdialog.h>
// #include <dcopref.h>
#include <solid/device.h>
#include <solid/block.h>
#include <solid/opticaldrive.h>
#include <solid/opticaldisc.h>


// ### soundkonverter 0.4 implement reading of milliseconds/frames

// TODO implement reading of cd data

CDDevice::CDDevice( const QString& _device )
{
    QStringList devices;
    QList<int> cddbData;

    tags.clear();

    if( !_device.isEmpty() )
    {
        devices.append( _device );
    }
    else
    {
    /*    QList<Solid::Device> drives = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive, QString());
        for( int i=0; i<drives.count(); i++ )
        {
            Solid::StorageDrive *storageDrive = drives.value(i).as<Solid::StorageDrive>();
            Solid::Block *block = drives.value(i).as<Solid::Block>();
            if( storageDrive->driveType() == Solid::StorageDrive::CdromDrive )
            {
                KMessageBox::information( 0, QString("found: %1").arg(block->device()), "cool" );
            }
        }*/
        // finds all optical discs (not limited to audio cds)
        QList<Solid::Device> solid_devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume, QString());
        for( int i=0; i<solid_devices.count(); i++ )
        {
            if( solid_devices.value(i).is<Solid::OpticalDisc>() && solid_devices.value(i).is<Solid::Block>() )
            {
                Solid::OpticalDisc *solid_disc = solid_devices.value(i).as<Solid::OpticalDisc>();
                Solid::Block *solid_block = solid_devices.value(i).as<Solid::Block>();
    //             KMessageBox::information( 0, QString("found: %1").arg(solid_block->device()), "cool" );
                devices.append( solid_block->device() );
            }
        }
    }

    paranoia = new Paranoia();
    for( int i=0; i<devices.count(); i++ )
    {
        if( paranoia->init(devices.at(i)) )
        {
            device = devices.at(i);
            trackCount = paranoia->getTracks();
            timeCount = paranoia->trackTime( -1 );
            for( int j=0; j<paranoia->getTracks(); j++ )
            {
                cddbData.append( paranoia->trackFirstSector(j+1) + 150 );
            }
            cddbData.append( paranoia->discFirstSector() );
            cddbData.append( paranoia->discLastSector() );
/*        CDDB* cddb = new CDDB();
            cddb->save_cddb( true );
            if( cddb->queryCD(qvl) ) {
            for( i = 0; i < paranoia->getTracks(); i++ ) {
            tags += new TagData( cddb->artist(i), "", cddb->title(), cddb->track(i), cddb->genre(), "",
            i+1, cddb->disc(), cddb->year(), paranoia->trackTime(i) );
        }
        }
            else {
            cddb->set_server( "freedb.freedb.org", 8880 );
            if( cddb->queryCD(qvl) ) {
            for( i = 0; i < paranoia->getTracks(); i++ ) {
            tags += new TagData( cddb->artist(i), "", cddb->title(), cddb->track(i), cddb->genre(), "",
            i+1, cddb->disc(), cddb->year(), paranoia->trackTime(i) );
        }
        }
            else {*/
            for( i = 0; i < paranoia->getTracks(); i++ )
            {
                tags += new TagData( i18n("Unknown"), "", i18n("Unknown"), i18n("Unknown"), "", "", i+1, 1, 0, paranoia->trackTime(i) );
            }
        /*}
        }
            delete cddb;*/
        
            paranoia->discClose();
            
            delete paranoia;

            return;
        }
    }
/*
    KMessageBox::information( 0, i18n("No audio CD found"), i18n("No audio CD found") );
    device = "";
    delete paranoia;
    paranoia = 0;
    */

    device = "";
//     trackCount = 1;
//     timeCount = 210;
//     tags += new TagData( i18n("Unknown"), "", i18n("Unknown"), i18n("Unknown"), "", "", 1, 1, 0, 210 );
}

CDDevice::~CDDevice()
{
//     if( paranoia ) delete paranoia;
}



CDManager::CDManager( QObject *parent )
    : QObject( parent )
{}

CDManager::~CDManager()
{
    while( cdDevices.count() > 0 )
    {
        delete cdDevices.at(0);
        cdDevices.removeAt(0);
    }
}

QString CDManager::newCDDevice( const QString& device )
{
    CDDevice *cdDevice = new CDDevice( device );
    if( cdDevice->device == "" )
    {
        delete cdDevice;
        return "";
    }

    for( int i=0; i<cdDevices.count(); i++ )
    {
        if( cdDevices.at(i)->device == cdDevice->device )
        {
            delete cdDevices.at(i);
            cdDevices.removeAt(i);
            break;
        }
    }

    cdDevices += cdDevice;
    return cdDevice->device;
}

QList<TagData*> CDManager::getTrackList( const QString& device )
{
    for( int i=0; i<cdDevices.count(); i++ ) {
        if( cdDevices.at(i)->device == device ) return cdDevices.at(i)->tags;
    }

    QList<TagData*> list;
    return list;
}

TagData* CDManager::getTags( const QString& device, int track )
{
    for( int i=0; i<cdDevices.count(); i++ )
    {
        if( cdDevices.at(i)->device == device )
        {
            if( track > 0 )
            {
                return cdDevices.at(i)->tags.at(track-1);
            }
            else
            {
                return cdDevices.at(i)->discTags;
            }
        }
    }

    return 0;
}

int CDManager::getTrackCount( const QString& device )
{
    for( int i=0; i<cdDevices.count(); i++ )
    {
        if( cdDevices.at(i)->device == device ) return cdDevices.at(i)->trackCount;
    }

    return 0;
}

int CDManager::getTimeCount( const QString& device )
{
    for( int i=0; i<cdDevices.count(); i++ )
    {
        if( cdDevices.at(i)->device == device ) return cdDevices.at(i)->timeCount;
    }

    return 0;
}

void CDManager::setDiscTags( const QString& device, TagData *tags )
{
    for( int i=0; i<cdDevices.count(); i++ )
    {
        if( cdDevices.at(i)->device == device ) cdDevices.at(i)->discTags = tags;
    }
}

