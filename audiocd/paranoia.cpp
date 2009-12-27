/***************************************************************************
                          paranoia.cpp  -  description
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

#include "paranoia.h"

#include <unistd.h>
#include <math.h>

#include <QFile>
#include <QTextStream>
// #include <qslider.h>
// #include <qlcdnumber.h>
// #include <qdir.h>
// #include <qlineedit.h>
// #include <qbuttongroup.h>
// #include <qtoolbutton.h>
// #include <qcheckbox.h>

// #include <qcombobox.h>
// 
// #include <kmessagebox.h>
#include <klocale.h>
// //#include <kdebug.h>
// #include <ktrader.h>
// #include <kpushbutton.h>
// #include <kiconloader.h>
// #include <kfiledialog.h>
// #include <kparts/componentfactory.h>

#define DEFAULT_DRIVE "/dev/cdrom"




void paranoiaCallback( long, int )
{
}



Paranoia::Paranoia()
{
	d = 0;
	p = 0;
}



bool Paranoia::init( QString dev )
{
	QString s;
        QFile f(dev);

        if ( p!=0 ) paranoia_free( p );
	if ( d!=0 ) cdda_close( d );
	nTracks = 0;

	if ( !f.exists() ) {
		/*if ( !findCdrom() ) {
			d = cdda_find_a_cdrom( CDDA_MESSAGE_PRINTIT, 0 );
			if ( cdda_open( d )!=0 )
				return false;
		}*/
		return false;
	}
       else {
		d = cdda_identify( dev.toAscii(), CDDA_MESSAGE_PRINTIT, 0 );
                if ( d==0 )
                    return false;
                if ( cdda_open( d )!=0 )
			return false;
	}
	p = paranoia_init( d );
	nTracks = cdda_tracks( d );
	return true;
}



bool Paranoia::findCdrom()
{
	QFile *f;
	QString c;
	QString s="";
	int pos, i;
	bool stop=false;
	char dev[4][4]={"","","",""};

	f = new QFile( "/proc/sys/dev/cdrom/info" );
	if ( !f->open(QIODevice::ReadOnly) )
		return false;

	QTextStream t( f );
	while ( !t.atEnd() && !stop ) {
		s = t.readLine();
		if ( s.contains("drive name:") )
			stop = true;
	}
	if ( !stop )
		return false;

	pos = s.indexOf(":");
	c = s.right( s.length()-pos-1 );
	sscanf( c.toLatin1(), "%s %s %s %s", dev[0], dev[1], dev[2], dev[3] );

	for ( i=0; i<4; i++ )
		if ( procCdrom( dev[i] ) )
			return true;

	f->close();
	return false;
}



bool Paranoia::procCdrom( QString name )
{
	int pos;

	if ( name.contains("sr") ) {
                pos = name.indexOf("r");
		name = name.right( name.length()-pos-1 );
		name = "/dev/scd"+name;
		d = cdda_identify( name.toAscii(), CDDA_MESSAGE_PRINTIT, 0 );
		if ( cdda_open( d )==0 )
			return true;
	}
	else if ( name.contains("hd") ) {
		name = "/dev/"+name;
		d = cdda_identify( name.toAscii(), CDDA_MESSAGE_PRINTIT, 0 );
		if ( cdda_open( d )==0 )
			return true;
	}
	return false;
}



void Paranoia::setMode( int mode )
{
	switch ( mode ) {
		case 0 : mode = PARANOIA_MODE_DISABLE;
				break;
		case 1 : mode = PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP;
				break;
		case 2 : mode = PARANOIA_MODE_FULL;
	}
	paranoia_modeset( p, mode );
}


bool Paranoia::initTrack( int t )
{
	currentSector = cdda_track_firstsector( d, t );
	endOfTrack = cdda_track_lastsector( d, t );
	paranoia_seek( p, currentSector, SEEK_SET );
	return true;
}




int Paranoia::trackFirstSector( int t )
{
	return cdda_track_firstsector( d, t );
}



int Paranoia::discFirstSector()
{
	return cdda_disc_firstsector( d );
}



int Paranoia::discLastSector()
{
	return cdda_disc_lastsector( d );
}


int Paranoia::discClose()
{
        int ret = cdda_close( d );
        d = 0;
        return ret;
}


bool Paranoia::isAudio( int t )
{
	if ( cdda_track_audiop( d, t+1 ) ) return true;
	else return false;
}



QString Paranoia::trackSize( int t )
{
	QString s, c;
	long total;

	total = CD_FRAMESIZE_RAW * (cdda_track_lastsector( d, t+1 )-cdda_track_firstsector( d, t+1 ) );
	if ( total>(1048576 ) ) s = c.setNum(total/1048576.0, 'f', 2)+" "+i18n("MB");
	else if ( total>1024 ) s = c.setNum(total/1024.0, 'f', 2)+" "+i18n("KB");
	else s = c.setNum(total*1.0, 'f', 2)+" "+i18n("Bytes");
	return s;
}



long Paranoia::trackSectorSize( int t )
{
	return cdda_track_lastsector( d, t )-cdda_track_firstsector( d, t );
}



long Paranoia::trackTime( int t )
{
	QString c;
	long total, time;
// 	int m, s;

	if ( t<0 ) total = CD_FRAMESIZE_RAW * (cdda_disc_lastsector( d )-cdda_disc_firstsector( d ) );
	else total = CD_FRAMESIZE_RAW * (cdda_track_lastsector( d, t+1 )-cdda_track_firstsector( d, t+1 ) );
	time = (8 * total) / (44100 * 2 * 16);
// 	m = time/60;
// 	s = time%60;
// 	c.sprintf( "%.2i:%.2i", m, s );
	return time;
}



Paranoia::~Paranoia()
{
	if ( p!=0 ) paranoia_free( p );
	if (d!=0 ) cdda_close( d );
}



long Paranoia::getTracks()
{
	return nTracks;
}
