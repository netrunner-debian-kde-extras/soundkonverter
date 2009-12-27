
#include "optionslayer.h"
#include "options.h"
#include "config.h"

#include "optionseditor.h"

#include <QLayout>
#include <QFrame>

#include <KLocale>

#include <KPushButton>
#include <KIcon>


OptionsLayer::OptionsLayer( Config *config, QWidget *parent )
    : QWidget( parent )
{
    QGridLayout *gridLayout = new QGridLayout( this );
    gridLayout->setContentsMargins( 20, 20, 20, 20 );
//     gridLayout->setContentsMargins( 0, 0, 0, 0 );

    frame = new QFrame( this );
    gridLayout->addWidget( frame, 0, 0 );
    frame->setFrameShape( QFrame::StyledPanel );
    frame->setFrameShadow( QFrame::Raised );
    frame->setAutoFillBackground( true );
    QPalette palette = frame->palette();
    QBrush brush = palette.window();
    QBrush oldBrush = brush;
//     brush.setColor( QColor(230,236,238) ); // 223,230,231
//     brush.setColor( QColor(223,230,231) );
    palette.setBrush( QPalette::Window, brush );
    frame->setPalette( palette );


    QVBoxLayout *frameLayout = new QVBoxLayout( frame );
    frameLayout->setContentsMargins( 3, 3, 3, 3 );
//     frameLayout->setSpacing( 0 );

    options = new Options( config, i18n("Select your desired output options and click on \"Ok\"."), this );
    frameLayout->addWidget( options );


    QHBoxLayout *buttonBox = new QHBoxLayout();
    buttonBox->setContentsMargins( 0, 0, 6, 6 );
    frameLayout->addLayout( buttonBox );
    buttonBox->addStretch();
    pOk = new KPushButton( KIcon("dialog-ok"), i18n("Ok"), this );
    buttonBox->addWidget( pOk );
    connect( pOk, SIGNAL(clicked()), this, SLOT(ok()) );
    pCancel = new KPushButton( KIcon("dialog-cancel"), i18n("Cancel"), this );
    buttonBox->addWidget( pCancel );
    connect( pCancel, SIGNAL(clicked()), this, SLOT(abort()) );

    palette = options->palette();
    brush = palette.window();
//     brush.setColor( QColor(231,223,230) );  // FIXME color can't be set
    palette.setBrush( QPalette::Window, brush );
//     palette.setBrush( QPalette::Window, oldBrush );
    options->setPalette( palette );

    setAutoFillBackground( true );

    connect( &fadeTimer, SIGNAL(timeout()), this, SLOT(fadeAnim()) );
    fadeAlpha = 0.0f;

//     QPalette newPalette = palette();
//     newPalette.setBrush( QPalette::Window, brushSetAlpha( newPalette.window(), 192 ) );
//     newPalette.setBrush( QPalette::Shadow, brushSetAlpha( newPalette.shadow(), 192 ) );
//     newPalette.setBrush( QPalette::Base, brushSetAlpha( newPalette.base(), 192 ) );
//     setPalette( newPalette );
}

OptionsLayer::~OptionsLayer()
{}

void OptionsLayer::fadeIn()
{
    fadeTimer.start( 50 );
    fadeMode = 1;
    QPalette newPalette = palette();
    newPalette.setBrush( QPalette::Window, brushSetAlpha( newPalette.window(), 0 ) );
    setPalette( newPalette );
    newPalette = frame->palette();
    newPalette.setBrush( QPalette::Window, brushSetAlpha( newPalette.window(), 0 ) );
    frame->setPalette( newPalette );
    frame->hide();
    show();
}

void OptionsLayer::fadeOut()
{
    fadeTimer.start( 50 );
    fadeMode = 2;
    frame->hide();
}

void OptionsLayer::fadeAnim()
{
    if( fadeMode == 1 ) fadeAlpha += 255.0f/50.0f*8.0f;
    else if( fadeMode == 2 ) fadeAlpha -= 255.0f/50.0f*8.0f;

    if( fadeAlpha <= 0.0f ) { fadeAlpha = 0.0f; fadeMode = 0; hide(); }
    else if( fadeAlpha >= 255.0f ) { fadeAlpha = 255.0f; fadeMode = 0; frame->show(); }
    else { fadeTimer.start( 50 ); }

    QPalette newPalette = palette();
    newPalette.setBrush( QPalette::Window, brushSetAlpha( newPalette.window(), 192.0f/255.0f*fadeAlpha ) );
    setPalette( newPalette );

    newPalette = frame->palette();
    newPalette.setBrush( QPalette::Window, brushSetAlpha( newPalette.window(), 230.0f/255.0f*fadeAlpha ) );
    frame->setPalette( newPalette );

/*    newPalette = tab->palette();
    newPalette.setBrush( QPalette::Base, brushSetAlpha( newPalette.base(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Window, brushSetAlpha( newPalette.window(), fadeAlpha ) );
    newPalette.setBrush( QPalette::WindowText, brushSetAlpha( newPalette.windowText(), fadeAlpha ) );
    newPalette.setBrush( QPalette::AlternateBase, brushSetAlpha( newPalette.alternateBase(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Text, brushSetAlpha( newPalette.text(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Button, brushSetAlpha( newPalette.button(), fadeAlpha ) );
    newPalette.setBrush( QPalette::ButtonText, brushSetAlpha( newPalette.buttonText(), fadeAlpha ) );
    newPalette.setBrush( QPalette::BrightText, brushSetAlpha( newPalette.brightText(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Light, brushSetAlpha( newPalette.light(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Midlight, brushSetAlpha( newPalette.midlight(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Dark, brushSetAlpha( newPalette.dark(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Mid, brushSetAlpha( newPalette.mid(), fadeAlpha ) );
    newPalette.setBrush( QPalette::Shadow, brushSetAlpha( newPalette.shadow(), fadeAlpha ) );
    tab->setPalette( newPalette );*/
}

void OptionsLayer::setUrls( const KUrl::List& _urls )
{
    urls = _urls;
}

void OptionsLayer::abort()
{
    fadeOut();
}

void OptionsLayer::ok()
{
//     emit conversionOptionsSelected( options->currentConversionOptions() );
    fadeOut();
    emit done( urls, options->currentConversionOptions() );
}

ConversionOptions *OptionsLayer::currentConversionOptions()
{
    return options->currentConversionOptions();
}
