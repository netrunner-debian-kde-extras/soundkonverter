
#include "options.h"
#include "optionssimple.h"
#include "optionsdetailed.h"
#include "outputdirectory.h"
#include "pluginloader.h"
#include "config.h"

#include <QLayout>
#include <QToolTip>
#include <QFile>

#include <KLocale>
#include <KTabWidget>
#include <KPushButton>
// #include <kiconloader.h>
#include <KStandardDirs>

// FIXME prevent converting wav files to wav

Options::Options( Config *_config, const QString& text, QWidget *parent )
    : QWidget( parent ),
    config( _config )
{
//     connect( config, SIGNAL(configChanged()),
//                this, SLOT(configChanged())
//              );

    QGridLayout *gridLayout = new QGridLayout( this );
    gridLayout->setContentsMargins( 0, 0, 0, 0 );

    tab = new KTabWidget( this );
    gridLayout->addWidget( tab, 0, 0 );
    connect( tab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );

//     optionsDetailed = new OptionsDetailed( config, this );
    optionsSimple = new OptionsSimple( config, /*optionsDetailed,*/ text, this );
    connect( optionsSimple, SIGNAL(optionsChanged()), this, SLOT(simpleOptionsChanged()) );
    connect( optionsSimple->outputDirectory, SIGNAL(modeChanged(int)), this, SLOT(simpleOutputDirectoryModeChanged(int)) );
    connect( optionsSimple->outputDirectory, SIGNAL(directoryChanged(const QString&)), this, SLOT(simpleOutputDirectoryChanged(const QString&)) );



    optionsDetailed = new OptionsDetailed( config, this );
//     connect( optionsDetailed, SIGNAL(optionsChanged()), this, SLOT(detailedOptionsChanged()) );
    connect( optionsDetailed->outputDirectory, SIGNAL(modeChanged(int)), this, SLOT(detailedOutputDirectoryModeChanged(int)) );
    connect( optionsDetailed->outputDirectory, SIGNAL(directoryChanged(const QString&)), this, SLOT(detailedOutputDirectoryChanged(const QString&)) );
    connect( optionsDetailed, SIGNAL(currentDataRateChanged(int)), optionsSimple, SLOT(currentDataRateChanged(int)) );
    optionsDetailed->somethingChanged();

    connect( optionsDetailed, SIGNAL(customProfilesEdited()), optionsSimple, SLOT(updateProfiles()) );
    connect( optionsSimple, SIGNAL(customProfilesEdited()), optionsDetailed, SLOT(updateProfiles()) );

    QString format;
    if( config->data.general.defaultFormat == i18n("Last used") )
    {
        format = config->data.general.lastFormat;
    }
    else
    {
        format = config->data.general.defaultFormat;
    }
    optionsDetailed->setCurrentFormat( format );

    QString profile;
    if( config->data.general.defaultProfile == i18n("Last used") )
    {
        profile = config->data.general.lastProfile;
    }
    else
    {
        profile = config->data.general.defaultProfile;
    }
    if( config->customProfiles().indexOf(profile) != -1 )
    {
        optionsDetailed->loadCustomProfile( profile );
    }
    else
    {
        optionsDetailed->setCurrentProfile( profile );
    }

    tab->addTab( optionsSimple, i18n("Simple") );
    tab->addTab( optionsDetailed, i18n("Detailed") );

//     KTabWidget *tab2 = new KTabWidget( this );
//     gridLayout->addWidget( tab2, 0, 1 );
//     tab2->addTab( optionsDetailed, i18n("Detailed") );
}

Options::~Options()
{}

ConversionOptions *Options::currentConversionOptions()
{
    return optionsDetailed->currentConversionOptions();
}

bool Options::setCurrentConversionOptions( ConversionOptions *options )
{
    bool success =  optionsDetailed->setCurrentConversionOptions( options );
    tabChanged( 0 ); // NOTE not very clean, but optionsSimple needs to get updated
//     optionsSimple->refill();
    return success;
}

void Options::simpleOutputDirectoryModeChanged( int mode )
{
    if(optionsDetailed && optionsDetailed->outputDirectory) optionsDetailed->outputDirectory->setMode( (OutputDirectory::Mode)mode );
}

void Options::simpleOutputDirectoryChanged( const QString& directory )
{
    if(optionsDetailed && optionsDetailed->outputDirectory)  optionsDetailed->outputDirectory->setDirectory( directory );
}

void Options::simpleOptionsChanged()
{
    config->data.general.lastProfile = optionsSimple->currentProfile();
    config->data.general.lastFormat = optionsSimple->currentFormat();

    optionsDetailed->setCurrentFormat( optionsSimple->currentFormat() );
    if( config->customProfiles().contains(optionsSimple->currentProfile()) )
    {
        optionsDetailed->loadCustomProfile( optionsSimple->currentProfile() );
    }
    else
    {
        optionsDetailed->setCurrentProfile( optionsSimple->currentProfile() );
    }
    optionsDetailed->setReplayGainChecked( optionsSimple->isReplayGainChecked() );
//     optionsDetailed->setBpmEnabled( optionsSimple->isBpmChecked() );
    QString toolTip;
    bool replaygainEnabled = optionsDetailed->isReplayGainEnabled( &toolTip );
    optionsSimple->setReplayGainEnabled( replaygainEnabled, toolTip );
}

void Options::detailedOutputDirectoryModeChanged( int mode )
{
//     if(optionsSimple && optionsSimple->outputDirectory) optionsSimple->outputDirectory->setMode( (OutputDirectory::Mode)mode );
}

void Options::detailedOutputDirectoryChanged( const QString& directory )
{
//     if(optionsSimple && optionsSimple->outputDirectory) optionsSimple->outputDirectory->setDirectory( directory );
}

// void Options::detailedOptionsChanged()
// {
// }

void Options::tabChanged( int pageIndex )
{
    if( pageIndex == 0 )
    {
        //pAdvancedOptionsToggle->hide();
        optionsSimple->updateProfiles();
        optionsSimple->setCurrentProfile( optionsDetailed->currentProfile() );
        optionsSimple->setCurrentFormat( optionsDetailed->currentFormat() );
        // HACK signals are firing back
        QString toolTip;
        bool replaygainEnabled = optionsDetailed->isReplayGainEnabled( &toolTip );
        bool replaygainChecked = optionsDetailed->isReplayGainChecked();
//         KMessageBox::information( this, QString::number(replaygain) );
//         bool bpm = optionsDetailed->isBpmEnabled();
        optionsSimple->setReplayGainEnabled( replaygainEnabled, toolTip );
        optionsSimple->setReplayGainChecked( replaygainChecked );
//         optionsSimple->setBpmChecked( bpm );
        optionsSimple->setCurrentPlugin( optionsDetailed->getCurrentPlugin() );
        
        optionsSimple->outputDirectory->setMode( optionsDetailed->outputDirectory->mode() );
        optionsSimple->outputDirectory->setDirectory( optionsDetailed->outputDirectory->directory() );
    }
//     else
//     {
        //pAdvancedOptionsToggle->show();
//     }
    //config->data.general.lastTab = tab->currentPageIndex();
}

// void Options::setProfile( const QString& profile )
// {
//     optionsSimple->setCurrentProfile( profile );
// }
// 
// void Options::setFormat( const QString& format )
// {
//     optionsSimple->setCurrentFormat( format );
// }
// 
// void Options::setOutputDirectory( const QString& directory )
// {
//     optionsSimple->setCurrentOutputDirectory( directory );
// }
// 
// void Options::somethingChanged()
// {
//     emit optionsChanged();
// }
// 
// // TODO right this way? - seems to work
// void Options::configChanged()
// {
//     optionsDetailed->refill();
//     /*if( tab->page(tab->currentPageIndex()) == optionsSimple ) {
//         optionsSimple->refill();
//     }*/
//     optionsSimple->refill();
// }

