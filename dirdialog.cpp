
#include "dirdialog.h"
#include "config.h"
#include "options.h"

#include <QLayout>
#include <QLabel>
#include <QDir>
#include <QCheckBox>
#include <KLocale>
#include <KPushButton>
#include <KFileDialog>
#include <KIcon>
#include <KListWidget>
#include <KUrlRequester>


DirDialog::DirDialog( Config *config, Mode _mode, QWidget *parent, Qt::WFlags f )
    : KDialog( parent, f ),
    mode( _mode )
{
    setCaption( i18n("Add folder") );
    setWindowIcon( KIcon("folder") );
    
    if( mode == Convert )
    {
        setButtons( KDialog::User1 | KDialog::Cancel );
    }
    else if( mode == ReplayGain )
    {
        setButtons( KDialog::Ok | KDialog::Cancel );
    }
    
    setButtonText( KDialog::User1, i18n("Proceed") );
    setButtonIcon( KDialog::User1, KIcon("go-next") );

    connect( this, SIGNAL(user1Clicked()), this, SLOT(proceedClicked()) );
    connect( this, SIGNAL(okClicked()), this, SLOT(addClicked()) );

    page = DirOpenPage;

    QWidget *widget = new QWidget();
    QGridLayout *mainGrid = new QGridLayout( widget );
    QGridLayout *topGrid = new QGridLayout( 0 );
    mainGrid->addLayout( topGrid, 0, 0 );
    setMainWidget( widget );

    lSelector = new QLabel( i18n("1. Select directory"), widget );
    QFont font;
    font.setBold( true );
    lSelector->setFont( font );
    topGrid->addWidget( lSelector, 0, 0 );
    lOptions = new QLabel( i18n("2. Set conversion options"), widget );
    topGrid->addWidget( lOptions, 0, 1 );

    // draw a horizontal line
    QFrame *lineFrame = new QFrame( widget );
    lineFrame->setFrameShape( QFrame::HLine );
    lineFrame->setFrameShadow( QFrame::Sunken );
    mainGrid->addWidget( lineFrame, 1, 0 );

    if( mode == ReplayGain )
    {
        lSelector->hide();
        lOptions->hide();
        lineFrame->hide();
    }
    
    // Dir Opener Widget
    
    dirOpenerWidget = new QWidget( widget );
    mainGrid->addWidget( dirOpenerWidget, 2, 0 );    
    
    QVBoxLayout *box = new QVBoxLayout( dirOpenerWidget );

    QHBoxLayout *directoryBox = new QHBoxLayout();
    box->addLayout( directoryBox );

    QLabel *labelFilter = new QLabel( i18n("Directory:"), dirOpenerWidget );
    directoryBox->addWidget( labelFilter );

    uDirectory = new KUrlRequester( QDir::homePath(), dirOpenerWidget );
    uDirectory->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    directoryBox->addWidget( uDirectory );
    
    QLabel *labelDirectory = new QLabel( i18n("Only add selected file formats:"), dirOpenerWidget );
    box->addWidget( labelDirectory );
    
    QHBoxLayout *fileTypesBox = new QHBoxLayout();
    box->addLayout( fileTypesBox );

    QStringList codecList;
    fileTypes = new KListWidget( dirOpenerWidget );
    if( mode == Convert )
    {
        codecList = config->pluginLoader()->formatList( PluginLoader::Decode, PluginLoader::CompressionType(PluginLoader::Lossy|PluginLoader::Lossless|PluginLoader::Hybrid) );
    }
    else if( mode == ReplayGain )
    {
        codecList = config->pluginLoader()->formatList( PluginLoader::ReplayGain, PluginLoader::CompressionType(PluginLoader::Lossy|PluginLoader::Lossless|PluginLoader::Hybrid) );
    }
    for( int i = 0; i < codecList.size(); i++ )
    {
        if( codecList.at(i) == "audio cd" ) continue;
        QListWidgetItem *newItem = new QListWidgetItem( codecList.at(i), fileTypes );
        newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        newItem->setCheckState( Qt::Checked );
    }
    fileTypesBox->addWidget( fileTypes );

    QVBoxLayout *fileTypesButtonsBox = new QVBoxLayout();
    fileTypesBox->addLayout( fileTypesButtonsBox );
    fileTypesButtonsBox->addStretch();

    pSelectAll = new KPushButton( KIcon("edit-select-all"), i18n("Select all"), dirOpenerWidget );
    fileTypesButtonsBox->addWidget( pSelectAll );
    connect( pSelectAll, SIGNAL(clicked()), this, SLOT(selectAllClicked()) );

    pSelectNone = new KPushButton( KIcon("application-x-zerosize"), i18n("Select none"), dirOpenerWidget );
    fileTypesButtonsBox->addWidget( pSelectNone );
    connect( pSelectNone, SIGNAL(clicked()), this, SLOT(selectNoneClicked()) );

    cRecursive = new QCheckBox( i18n("Recursive"), dirOpenerWidget );
    cRecursive->setChecked( true );
    cRecursive->setToolTip( i18n("If checked, files from subdirectories will be added, too.") );
    fileTypesButtonsBox->addWidget( cRecursive );

    fileTypesButtonsBox->addStretch();


    // Conversion Options Widget
    
    options = new Options( config, i18n("Select your desired output options and click on \"Ok\"."), widget );
    mainGrid->addWidget( options, 2, 0 );
    adjustSize();
    options->hide();
    
    
    KUrl url = KFileDialog::getExistingDirectoryUrl( uDirectory->url(), this );
    if( !url.isEmpty() ) uDirectory->setUrl( url );
}

DirDialog::~DirDialog()
{}

void DirDialog::proceedClicked()
{
    if( page == DirOpenPage )
    {
        dirOpenerWidget->hide();
        options->show();
        page = ConversionOptionsPage;
        QFont font;
        font.setBold( false );
        lSelector->setFont( font );
        font.setBold( true );
        lOptions->setFont( font );
        setButtons( KDialog::Ok | KDialog::Cancel );
    }
}

void DirDialog::addClicked()
{
    QStringList selectedCodecs;
    for( int i = 0; i < fileTypes->count(); i++ )
    {
        if( fileTypes->item(i)->checkState() == Qt::Checked ) selectedCodecs += fileTypes->item(i)->text();
    }

    emit accept();
    if( mode == Convert )
    {
        emit done( uDirectory->url(), cRecursive->checkState() == Qt::Checked, selectedCodecs, options->currentConversionOptions() );
    }
    else if( mode == ReplayGain )
    {
        emit done( uDirectory->url(), cRecursive->checkState() == Qt::Checked, selectedCodecs );
    }
}

void DirDialog::selectAllClicked()
{
    for( int i = 0; i < fileTypes->count(); i++ )
    {
        fileTypes->item(i)->setCheckState( Qt::Checked );
    }
}

void DirDialog::selectNoneClicked()
{
    for( int i = 0; i < fileTypes->count(); i++ )
    {
        fileTypes->item(i)->setCheckState( Qt::Unchecked );
    }
}

