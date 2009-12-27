//
// C++ Interface: optionslayer
//
// Description: 
//
//
// Author: Daniel Faust <hessijames@gmail.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OPTIONSLAYER_H
#define OPTIONSLAYER_H

#include <QWidget>
#include <QColor>
#include <QTimer>
#include <KUrl>

class Config;
class Options;
class ConversionOptions;

class QFrame;
class KPushButton;

/**
	@author Daniel Faust <hessijames@gmail.com>
*/
class OptionsLayer : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    OptionsLayer( Config *config, QWidget *parent );

    /**
     * Destructor
     */
    virtual ~OptionsLayer();

    void fadeIn();
    void fadeOut(); // should be private
    void setUrls( const KUrl::List& _urls );

    ConversionOptions *currentConversionOptions(); // HACK unneeded

private:
    QFrame *frame;
    Options *options;
    KPushButton *pOk;
    KPushButton *pCancel;

    QTimer fadeTimer;
    float fadeAlpha;
    int fadeMode; // 1 = fade in, 2 = fade out
    
    KUrl::List urls;

    inline QBrush brushSetAlpha( QBrush brush, const int alpha )
    {
        QColor color = brush.color();
        color.setAlpha( alpha );
        brush.setColor( color );
        return brush;
    }

private slots:
    void fadeAnim();
    void abort();
    void ok();
    
signals:
    void done( const KUrl::List& urls, ConversionOptions *options );

};

#endif
