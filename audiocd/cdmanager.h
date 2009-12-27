

#ifndef CDMANAGER_H
#define CDMANAGER_H

#include "../metadata/tagengine.h"

#include <qobject.h>

class Paranoia;

/**
 * @short All data needed for a cd device
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 0.3
 */
class CDDevice
{
public:
    /**
     * Constructor
     */
    CDDevice( const QString& _device="" );

    /**
     * Destructor
     */
    virtual ~CDDevice();

    QString device;
    Paranoia* paranoia;
    QList<TagData*> tags;
    TagData* discTags;
    int trackCount;
    int timeCount;
};

/**
 * @short The CD manager
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 0.3
 */
class CDManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    CDManager( QObject *parent=0 );

    /**
     * Destructor
     */
    virtual ~CDManager();

    /**
     * Create a new CDDevice entry in cdDevices. Use @param device or auto search for an audio cd
     * Return the used device (usefull, if auto searching was used)
     */
    QString newCDDevice( const QString& device="" );

    /**
     * Return a list of all tracks on the cd in drive @param device
     */
    QList<TagData*> getTrackList( const QString& device );

    /**
     * Return the tags of the track @param track on the cd in drive @param device
     */
    TagData *getTags( const QString& device, int track );

    /**
     * Set the tags of the cd in drive @param device
     */
    void setDiscTags( const QString& device, TagData *tags );

    /**
     * Return the sum of all tracks of the cd in drive @param device
     */
    int getTrackCount( const QString& device );

    /**
     * Return the complete length of the cd in drive @param device
     */
    int getTimeCount( const QString& device );

private:
    /** a list of all devices */
    QList<CDDevice*> cdDevices;

};

#endif // CDMANAGER_H
