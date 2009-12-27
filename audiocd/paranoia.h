/***************************************************************************
                          paranoia.h  -  description
                             -------------------
    copyright            : (C) 2002-2006 by Christophe Thommeret
    email                : hftom@free.fr
    modified             : 2006 Daniel Faust <hessijames@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PARANOIA_H
#define PARANOIA_H

#include <qstringlist.h>

extern "C"
{
#include <cdda_interface.h>
#include <cdda_paranoia.h>
}



class Paranoia
{
public:
	Paranoia();
	bool init( QString dev );
	~Paranoia();
	long getTracks();
	long trackTime( int t );
	int trackFirstSector( int t );
	int discFirstSector();
	int discLastSector();
    int discClose();

private:
	bool findCdrom();
	bool procCdrom( QString name );
	bool initTrack( int t );
	void setMode( int mode );
	bool isAudio( int t );
	QString trackSize( int t );
	long trackSectorSize( int t );

	long nTracks;
	cdrom_drive *d;
	cdrom_paranoia *p;
	long currentSector, endOfTrack;
	int paraMode;
	int progress;
};
#endif
