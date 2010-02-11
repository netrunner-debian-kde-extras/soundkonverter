
#include "cdopener.h"
#include "../audiocd/cdmanager.h"
#include "../metadata/tagengine.h"
#include "../config.h"
#include "../options.h"

#include <KLocale>
#include <KPushButton>
#include <KLineEdit>
#include <KComboBox>
#include <KNumInput>
#include <KTextEdit>
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>

#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTreeWidget>
// #include <QList>
#include <QDateTime>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QCheckBox>

// ### soundkonverter 0.4: implement cd info text

CDOpener::CDOpener( Config *_config, CDManager *_cdManager, const QString& _device, QWidget *parent /*Mode default_mode, const QString& default_text,*/, Qt::WFlags f )
    : KDialog( parent, f ),
    cdManager( _cdManager ),
    config( _config )
{
    setButtons( 0 );
    
    page = CdOpenPage;

    device = cdManager->newCDDevice( _device );
    // don't execute the dialog, if no audio cd was found
    noCD = device.isEmpty();
    if( noCD ) return;

    // let the dialog look nice
    setCaption( i18n("Add CD tracks") );
    setWindowIcon( KIcon("media-optical-audio") );
    
    QWidget *widget = new QWidget( this );
    QGridLayout *mainGrid = new QGridLayout( widget );
    QGridLayout *topGrid = new QGridLayout( 0 );
    mainGrid->addLayout( topGrid, 0, 0 );
    setMainWidget( widget );

    lSelector = new QLabel( i18n("1. Select CD tracks"), widget );
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


    // CD Opener Widget
    
    cdOpenerWidget = new QWidget( widget );
    mainGrid->addWidget( cdOpenerWidget, 2, 0 );    

    // the grid for all widgets in the dialog
    QGridLayout *gridLayout = new QGridLayout( cdOpenerWidget );

    // the box for the cover and artist/album grid
    QHBoxLayout *topBoxLayout = new QHBoxLayout( 0 );
    gridLayout->addLayout( topBoxLayout, 0, 0 );

    // the album cover
    QLabel *lAlbumCover = new QLabel( "", cdOpenerWidget );
    topBoxLayout->addWidget( lAlbumCover );
//     lAlbumCover->setPixmap( QPixmap("/usr/share/icons/oxygen/64x64/devices/media-optical-audio.png") );
    lAlbumCover->setPixmap( QPixmap( KStandardDirs::locate("data","soundkonverter/images/nocover.png") ) );
    lAlbumCover->setContentsMargins( 0, 0, 6, 0 );

    // the grid for the artist and album input
    QGridLayout *topGridLayout = new QGridLayout( 0 );
    topBoxLayout->addLayout( topGridLayout );

    // set up the first row at the top
    QLabel *lArtistLabel = new QLabel( i18n("Artist:"), cdOpenerWidget );
    topGridLayout->addWidget( lArtistLabel, 0, 0 );
    cArtist = new KComboBox( true, cdOpenerWidget );
    topGridLayout->addWidget( cArtist, 0, 1 );
    cArtist->setMinimumWidth( 180 );
    cArtist->addItem( i18n("Various Artists") );
    cArtist->setEditText( "" );
    connect( cArtist, SIGNAL(textChanged(const QString&)), this, SLOT(artistChanged(const QString&)) );
    // add a horizontal box layout for the composer
    QHBoxLayout *artistBox = new QHBoxLayout( 0 );
    topGridLayout->addLayout( artistBox, 0, 3 );
    // and fill it up
    QLabel *lComposerLabel = new QLabel( i18n("Composer:"), cdOpenerWidget );
    lComposerLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    topGridLayout->addWidget( lComposerLabel, 0, 2 );
    cComposer = new KComboBox( true, cdOpenerWidget );
    artistBox->addWidget( cComposer );
    cComposer->setMinimumWidth( 180 );
    cComposer->addItem( i18n("Various Composer") );
    cComposer->setEditText( "" );
    //cComposer->setSizePolicy( QSizePolicy::Maximum );
    connect( cComposer, SIGNAL(textChanged(const QString&)), this, SLOT(composerChanged(const QString&)) );
    //artistBox->addStretch();
//     artistBox->addSpacing( 130 );
//     pCDDB = new KPushButton( iconLoader->loadIcon("cdaudio_unmount",KIcon::Small), i18n("Request CDDB"), this, "pCDDB" );
//     topGridLayout->addWidget( pCDDB, 0, 8 );

    // set up the second row at the top
    QLabel *lAlbumLabel = new QLabel( i18n("Album:"), cdOpenerWidget );
    topGridLayout->addWidget( lAlbumLabel, 1, 0 );
    lAlbum = new KLineEdit( cdOpenerWidget );
    topGridLayout->addWidget( lAlbum, 1, 1 );
    // add a horizontal box layout for the disc number
    QHBoxLayout *albumBox = new QHBoxLayout( 0 );
    topGridLayout->addLayout( albumBox, 1, 3 );
    // and fill it up
    QLabel *lDiscLabel = new QLabel( i18n("Disc No.:"), cdOpenerWidget );
    lDiscLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    topGridLayout->addWidget( lDiscLabel, 1, 2 );
    iDisc = new KIntSpinBox( 1, 99, 1, 1, cdOpenerWidget );
    albumBox->addWidget( iDisc );
    albumBox->addStretch();

    // set up the third row at the top
    QLabel *lYearLabel = new QLabel( i18n("Year:"), cdOpenerWidget );
    topGridLayout->addWidget( lYearLabel, 2, 0 );
    // add a horizontal box layout for the year and genre
    QHBoxLayout *yearBox = new QHBoxLayout( 0 );
    topGridLayout->addLayout( yearBox, 2, 1 );
    // and fill it up
    iYear = new KIntSpinBox( 0, 99999, 1, QDate::currentDate().year(), cdOpenerWidget );
    yearBox->addWidget( iYear );
    QLabel *lGenreLabel = new QLabel( i18n("Genre:"), cdOpenerWidget );
    lGenreLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    yearBox->addWidget( lGenreLabel );
    cGenre = new KComboBox( true, cdOpenerWidget );
    cGenre->addItems( config->tagEngine()->genreList );
    cGenre->setEditText( "" );
    KCompletion *cGenreCompletion = cGenre->completionObject();
    cGenreCompletion->insertItems( config->tagEngine()->genreList );
    cGenreCompletion->setIgnoreCase( true );
    yearBox->addWidget( cGenre );

    topGridLayout->setColumnStretch( 1, 1 );
    topGridLayout->setColumnStretch( 3, 1 );
    

    // generate the list view for the tracks
    trackList = new QTreeWidget( cdOpenerWidget );
    gridLayout->addWidget( trackList, 1, 0 );
    // and fill in the headers
    trackList->setColumnCount( 5 );
    QStringList labels;
    labels.append( i18n("Rip") );
    labels.append( i18n("Track") );
    labels.append( i18n("Artist") );
    labels.append( i18n("Composer") );
    labels.append( i18n("Title") );
    labels.append( i18n("Length") );
    trackList->setHeaderLabels( labels );
    trackList->setSelectionBehavior( QAbstractItemView::SelectRows );
    trackList->setSelectionMode( QAbstractItemView::ExtendedSelection );
    trackList->setSortingEnabled( false );
    trackList->setRootIsDecorated( false );
    connect( trackList, SIGNAL(itemSelectionChanged()), this, SLOT(trackChanged()) );
    gridLayout->setRowStretch( 1, 1 );

    
    // create the box at the bottom for editing the tags
    tagGroupBox = new QGroupBox( i18n("No track selected"), cdOpenerWidget );
    gridLayout->addWidget( tagGroupBox, 2, 0 );
    QGridLayout *tagGridLayout = new QGridLayout( tagGroupBox );

    // add the up and down buttons
    pTrackUp = new KPushButton( "", tagGroupBox );
    pTrackUp->setIcon( KIcon("arrow-up") );
    pTrackUp->setFixedSize( pTrackUp->sizeHint().height(), pTrackUp->sizeHint().height() );
    pTrackUp->setAutoRepeat( true );
    connect( pTrackUp, SIGNAL(clicked()), this, SLOT(trackUpPressed()) );
    tagGridLayout->addWidget( pTrackUp, 0, 0 );
    pTrackDown = new KPushButton( "", tagGroupBox );
    pTrackDown->setIcon( KIcon("arrow-down") );
    pTrackDown->setFixedSize( pTrackDown->sizeHint().height(), pTrackDown->sizeHint().height() );
    pTrackDown->setAutoRepeat( true );
    connect( pTrackDown, SIGNAL(clicked()), this, SLOT(trackDownPressed()) );
    tagGridLayout->addWidget( pTrackDown, 1, 0 );

    // add the inputs
    // add a horizontal box layout for the title
    QHBoxLayout *trackTitleBox = new QHBoxLayout( 0 );
    tagGridLayout->addLayout( trackTitleBox, 0, 2 );
    // and fill it up
    QLabel *lTrackTitleLabel = new QLabel( i18n("Title:"), tagGroupBox );
    tagGridLayout->addWidget( lTrackTitleLabel, 0, 1 );
    lTrackTitle = new KLineEdit( tagGroupBox );
    trackTitleBox->addWidget( lTrackTitle );
    connect( lTrackTitle, SIGNAL(textChanged(const QString&)), this, SLOT(trackTitleChanged(const QString&)) );
    pTrackTitleEdit = new KPushButton( "", tagGroupBox );
    pTrackTitleEdit->setIcon( KIcon("document-edit") );
    pTrackTitleEdit->setFixedSize( lTrackTitle->sizeHint().height(), lTrackTitle->sizeHint().height() );
    pTrackTitleEdit->hide();
    trackTitleBox->addWidget( pTrackTitleEdit );
    connect( pTrackTitleEdit, SIGNAL(clicked()), this, SLOT(editTrackTitleClicked()) );
    // add a horizontal box layout for the composer
    QHBoxLayout *trackArtistBox = new QHBoxLayout( 0 );
    tagGridLayout->addLayout( trackArtistBox, 1, 2 );
    // and fill it up
    QLabel *lTrackArtistLabel = new QLabel( i18n("Artist:"), tagGroupBox );
    tagGridLayout->addWidget( lTrackArtistLabel, 1, 1 );
    lTrackArtist = new KLineEdit( tagGroupBox );
    trackArtistBox->addWidget( lTrackArtist );
    connect( lTrackArtist, SIGNAL(textChanged(const QString&)), this, SLOT(trackArtistChanged(const QString&)) );
    pTrackArtistEdit = new KPushButton( "", tagGroupBox );
    pTrackArtistEdit->setIcon( KIcon("document-edit") );
    pTrackArtistEdit->setFixedSize( lTrackArtist->sizeHint().height(), lTrackArtist->sizeHint().height() );
    pTrackArtistEdit->hide();
    trackArtistBox->addWidget( pTrackArtistEdit );
    connect( pTrackArtistEdit, SIGNAL(clicked()), this, SLOT(editTrackArtistClicked()) );
    QLabel *lTrackComposerLabel = new QLabel( i18n("Composer:"), tagGroupBox );
    trackArtistBox->addWidget( lTrackComposerLabel );
    lTrackComposer = new KLineEdit( tagGroupBox );
    trackArtistBox->addWidget( lTrackComposer );
    connect( lTrackComposer, SIGNAL(textChanged(const QString&)), this, SLOT(trackComposerChanged(const QString&)) );
    pTrackComposerEdit = new KPushButton( "", tagGroupBox );
    pTrackComposerEdit->setIcon( KIcon("document-edit") );
    pTrackComposerEdit->setFixedSize( lTrackComposer->sizeHint().height(), lTrackComposer->sizeHint().height() );
    pTrackComposerEdit->hide();
    trackArtistBox->addWidget( pTrackComposerEdit );
    connect( pTrackComposerEdit, SIGNAL(clicked()), this, SLOT(editTrackComposerClicked()) );
    // add a horizontal box layout for the comment
    QHBoxLayout *trackCommentBox = new QHBoxLayout( 0 );
    tagGridLayout->addLayout( trackCommentBox, 2, 2 );
    // and fill it up
    QLabel *lTrackCommentLabel = new QLabel( i18n("Comment:"), tagGroupBox );
    tagGridLayout->addWidget( lTrackCommentLabel, 2, 1 );
    tTrackComment = new KTextEdit( tagGroupBox );
    trackCommentBox->addWidget( tTrackComment );
    tTrackComment->setFixedHeight( 45 );
    connect( tTrackComment, SIGNAL(textChanged()), this, SLOT(trackCommentChanged()) );
    pTrackCommentEdit = new KPushButton( "", tagGroupBox );
    pTrackCommentEdit->setIcon( KIcon("document-edit") );
    pTrackCommentEdit->setFixedSize( lTrackTitle->sizeHint().height(), lTrackTitle->sizeHint().height() );
    pTrackCommentEdit->hide();
    trackCommentBox->addWidget( pTrackCommentEdit );
    connect( pTrackCommentEdit, SIGNAL(clicked()), this, SLOT(editTrackCommentClicked()) );

    bool various_artists = false;
    bool various_composer = false;
    QString artist = "";
    QString composer = "";
    QString album = "";
    int disc = 0;
    int year = 0;
    QString genre = "";

    QList<TagData*> tags = cdManager->getTrackList( device );
    for( int i=0; i<tags.count(); i++ )
    {
        if( artist == "" ) artist = tags.at(i)->artist;
        else if( artist != tags.at(i)->artist ) various_artists = true;

        if( composer == "" ) composer = tags.at(i)->composer;
        else if( composer != tags.at(i)->composer ) various_composer = true;

        if( album == "" ) album = tags.at(i)->album;
        if( disc == 0 ) disc = tags.at(i)->disc;
        if( year == 0 ) year = tags.at(i)->year;
        if( genre == "" ) genre = tags.at(i)->genre;

        QStringList data;
        data.append( "" );
        data.append( QString().sprintf("%02i",tags.at(i)->track) );
        data.append( tags.at(i)->artist );
        data.append( tags.at(i)->composer );
        data.append( tags.at(i)->title );
        data.append( QString().sprintf("%i:%02i",tags.at(i)->length/60,tags.at(i)->length%60) );
        QTreeWidgetItem *item = new QTreeWidgetItem( trackList, data );
        item->setCheckState( 0, Qt::Checked );
    }
    trackList->resizeColumnToContents( columnByName( i18n("Rip") ) );
    trackList->resizeColumnToContents( columnByName( i18n("Track") ) );

    // fill in the cd information
    if( various_artists ) cArtist->setCurrentItem( 0 );
    else cArtist->setEditText( artist );
    artistChanged( cArtist->currentText() );

    if( various_composer ) cComposer->setCurrentItem( 0 );
    else cComposer->setEditText( composer );
    composerChanged( cComposer->currentText() );

    lAlbum->setText( album );
    if( disc != 0 ) iDisc->setValue( disc );
    if( year != 0 ) iYear->setValue( year );
    cGenre->setEditText( genre );

    trackList->topLevelItem(0)->setSelected( true );
    

    // Conversion Options Widget
    
    options = new Options( config, i18n("Select your desired output options and click on \"Ok\"."), widget );
    mainGrid->addWidget( options, 2, 0 );
    adjustSize();
    int h_margin = ( cdOpenerWidget->sizeHint().width() - options->sizeHint().width() ) / 4;
    int v_margin = ( cdOpenerWidget->sizeHint().height() - options->sizeHint().height() ) / 4;
    options->setContentsMargins( h_margin, v_margin, h_margin, v_margin );
    options->hide();


    // draw a horizontal line
    QFrame *buttonLineFrame = new QFrame( widget );
    buttonLineFrame->setFrameShape( QFrame::HLine );
    buttonLineFrame->setFrameShadow( QFrame::Sunken );
    buttonLineFrame->setFrameShape( QFrame::HLine );
    mainGrid->addWidget( buttonLineFrame, 4, 0 );

    // add a horizontal box layout for the control elements
    QHBoxLayout *controlBox = new QHBoxLayout( 0 );
    mainGrid->addLayout( controlBox, 5, 0 );

    // add the control elements
    pSaveCue = new KPushButton( KIcon("document-save"), i18n("Save cue sheet..."), widget );
    controlBox->addWidget( pSaveCue );
    connect( pSaveCue, SIGNAL(clicked()), this, SLOT(saveCuesheetClicked()) );
    controlBox->addStretch();

    cEntireCd = new QCheckBox( i18n("Rip entire CD to one file"), widget );
    QStringList errorList;
    cEntireCd->setEnabled( config->pluginLoader()->canRipEntireCd(&errorList) );
    if( !cEntireCd->isEnabled() )
    {
        if( !errorList.isEmpty() )
        {  
            errorList.prepend( i18n("Ripping an entire cd to a single file is not supported by the installed backends.\nPossible solutions are listed below.\n") );
        }
        else
        {
            errorList += i18n("Ripping an entire cd to a single file is not supported by the installed backends.\nPlease check your distribution's package manager in order to install an additional ripper plugin wich supports ripping to one file.");
        }
        cEntireCd->setToolTip( errorList.join("\n") );
    }
    controlBox->addWidget( cEntireCd );
    controlBox->addSpacing( 20 );

    pProceed = new KPushButton( KIcon("go-next"), i18n("Proceed"), widget );
    controlBox->addWidget( pProceed );
    connect( pProceed, SIGNAL(clicked()), this, SLOT(proceedClicked()) );
    pAdd = new KPushButton( KIcon("dialog-ok"), i18n("Ok"), widget );
    controlBox->addWidget( pAdd );
    pAdd->hide();
    connect( pAdd, SIGNAL(clicked()), this, SLOT(addClicked()) );
    pCancel = new KPushButton( KIcon("dialog-cancel"), i18n("Cancel"), widget );
    controlBox->addWidget( pCancel );
    connect( pCancel, SIGNAL(clicked()), this, SLOT(reject()) );
}

CDOpener::~CDOpener()
{}

int CDOpener::columnByName( const QString& name )
{
    QTreeWidgetItem *header = trackList->headerItem();

    for( int i=0; i<trackList->columnCount(); ++i ) {
        if( header->text(i) == name ) return i;
    }
    return -1;
}

void CDOpener::trackUpPressed()
{
    QTreeWidgetItem *item = trackList->topLevelItem( selectedTracks.first() - 2 );

    if( !item ) return;

    disconnect( trackList, SIGNAL(itemSelectionChanged()), 0, 0 ); // avoid backfireing

    for( int i=0; i<selectedItems.count(); i++ )
    {
        selectedItems.at(i)->setSelected( false );
    }

    item->setSelected( true );
    trackList->scrollToItem( item );

    connect( trackList, SIGNAL(itemSelectionChanged()), this, SLOT(trackChanged()) );

    trackChanged();
}

void CDOpener::trackDownPressed()
{
    QTreeWidgetItem *item = trackList->topLevelItem( selectedTracks.last() );

    if( !item ) return;

    disconnect( trackList, SIGNAL(itemSelectionChanged()), 0, 0 ); // avoid backfireing

    for( int i=0; i<selectedItems.count(); i++ )
    {
        selectedItems.at(i)->setSelected( false );
    }

    item->setSelected( true );
    trackList->scrollToItem( item );

    connect( trackList, SIGNAL(itemSelectionChanged()), this, SLOT(trackChanged()) );

    trackChanged();
}

void CDOpener::trackChanged()
{
    // NOTE if no track is selected soundkonverter could use the current item as default item (like qlistview does)

    // rebuild the list of the selected tracks
    selectedTracks.clear();
    selectedItems.clear();
    QTreeWidgetItem *item;
    for( int i=0; i<trackList->topLevelItemCount(); i++ )
    {
        item = trackList->topLevelItem( i );
        if( item->isSelected() )
        {
            selectedTracks.append( i+1 );
            selectedItems.append( item );
        }
    }

    // insert the new values
    if( selectedTracks.count() < 1 )
    {
        pTrackUp->setEnabled( false );
        pTrackDown->setEnabled( false );

        lTrackTitle->setEnabled( false );
        lTrackTitle->setText( "" );
        pTrackTitleEdit->hide();
        lTrackArtist->setEnabled( false );
        lTrackArtist->setText( "" );
        pTrackArtistEdit->hide();
        lTrackComposer->setEnabled( false );
        lTrackComposer->setText( "" );
        pTrackComposerEdit->hide();
        tTrackComment->setEnabled( false );
        tTrackComment->setReadOnly( true );
        tTrackComment->setText( "" );
        pTrackCommentEdit->hide();
        
        pTrackUp->setEnabled( false );
        pTrackDown->setEnabled( false );

        return;
    }
    else if( selectedTracks.count() > 1 )
    {
        if( selectedTracks.first() > 1 ) pTrackUp->setEnabled( true );
        else pTrackUp->setEnabled( false );

        if( selectedTracks.last() < trackList->topLevelItemCount() ) pTrackDown->setEnabled( true );
        else pTrackDown->setEnabled( false );

        QString trackListString = "";
        if( selectedTracks.count() == trackList->topLevelItemCount() )
        {
            trackListString = i18n("All tracks");
        }
        else
        {
            trackListString = i18n("Tracks") + QString().sprintf( " %02i", selectedTracks.at(0) );
            for( int i = 1; i < selectedTracks.count(); i++ )
            {
                trackListString += QString().sprintf( ", %02i", selectedTracks.at(i) );
            }
        }
        tagGroupBox->setTitle( trackListString );

        QString title = cdManager->getTags( device, selectedTracks.at(0) )->title;
        bool equalTitles = true;
        QString artist = cdManager->getTags( device, selectedTracks.at(0) )->artist;
        bool equalArtists = true;
        QString composer = cdManager->getTags( device, selectedTracks.at(0) )->composer;
        bool equalComposers = true;
        QString comment = cdManager->getTags( device, selectedTracks.at(0) )->comment;
        bool equalComments = true;
        for( int i=0; i<selectedTracks.count(); i++ )
        {
            TagData *tags = cdManager->getTags( device, selectedTracks.at(i) );
            Q_CHECK_PTR( tags );

            if( title != tags->title ) equalTitles = false;
            if( artist != tags->artist ) equalArtists = false;
            if( composer != tags->composer ) equalComposers = false;
            if( comment != tags->comment ) equalComments = false;
        }

        if( equalTitles ) {
            lTrackTitle->setEnabled( true );
            lTrackTitle->setText( title );
            pTrackTitleEdit->hide();
        } else {
            lTrackTitle->setEnabled( false );
            lTrackTitle->setText( "" );
            pTrackTitleEdit->show();
        }

        if( cArtist->currentText() == i18n("Various Artists") && equalArtists ) {
            lTrackArtist->setEnabled( true );
            lTrackArtist->setText( artist );
            pTrackArtistEdit->hide();
        } else if( cArtist->currentText() == i18n("Various Artists") ) {
            lTrackArtist->setEnabled( false );
            lTrackArtist->setText( "" );
            pTrackArtistEdit->show();
        } else {
            lTrackArtist->setEnabled( false );
            lTrackArtist->setText( cArtist->currentText() );
            pTrackArtistEdit->hide();
        }

        if( cComposer->currentText() == i18n("Various Composer") && equalComposers ) {
            lTrackComposer->setEnabled( true );
            lTrackComposer->setText( composer );
            pTrackComposerEdit->hide();
        } else if( cComposer->currentText() == i18n("Various Composer") ) {
            lTrackComposer->setEnabled( false );
            lTrackComposer->setText( "" );
            pTrackComposerEdit->show();
        } else {
            lTrackComposer->setEnabled( false );
            lTrackComposer->setText( cComposer->currentText() );
            pTrackComposerEdit->hide();
        }

        if( equalComments ) {
            tTrackComment->setEnabled( true );
            tTrackComment->setReadOnly( false );
            tTrackComment->setText( comment );
            pTrackCommentEdit->hide();
        } else {
            tTrackComment->setEnabled( false );
            tTrackComment->setReadOnly( true );
            tTrackComment->setText( "" );
            pTrackCommentEdit->show();
        }
    }
    else
    {
        if( selectedTracks.first() > 1 ) pTrackUp->setEnabled( true );
        else pTrackUp->setEnabled( false );

        if( selectedTracks.last() < trackList->topLevelItemCount() ) pTrackDown->setEnabled( true );
        else pTrackDown->setEnabled( false );

        tagGroupBox->setTitle( i18n("Track") + QString().sprintf(" %02i",selectedTracks.at(0)) );

        TagData *tags = cdManager->getTags( device, selectedTracks.at(0) );
        Q_CHECK_PTR( tags );
        lTrackTitle->setEnabled( true );
        lTrackTitle->setText( tags->title );
        pTrackTitleEdit->hide();
        
        if( cArtist->currentText() == i18n("Various Artists") ) {
            lTrackArtist->setEnabled( true );
            lTrackArtist->setText( tags->artist );
            pTrackArtistEdit->hide();
        } else {
            lTrackArtist->setEnabled( false );
            lTrackArtist->setText( cArtist->currentText() );
            pTrackArtistEdit->hide();
        }
        
        if( cComposer->currentText() == i18n("Various Composer") ) {
            lTrackComposer->setEnabled( true );
            lTrackComposer->setText( tags->composer );
            pTrackComposerEdit->hide();
        } else {
            lTrackComposer->setEnabled( false );
            lTrackComposer->setText( cComposer->currentText() );
            pTrackComposerEdit->hide();
        }
        
        tTrackComment->setEnabled( true );
        tTrackComment->setReadOnly( false );
        tTrackComment->setText( tags->comment );
        pTrackCommentEdit->hide();
    }
}

void CDOpener::artistChanged( const QString& text )
{
    trackList->setColumnHidden( columnByName( i18n("Artist") ), text != i18n("Various Artists") );
    trackChanged();
}

void CDOpener::composerChanged( const QString& text )
{
    trackList->setColumnHidden( columnByName( i18n("Composer") ), text != i18n("Various Composer") );
    trackChanged();
}

void CDOpener::trackTitleChanged( const QString& text )
{
    if( !lTrackTitle->isEnabled() ) return;

    for( QList<QTreeWidgetItem*>::Iterator it = selectedItems.begin(); it != selectedItems.end(); ++it )
    {
        (*it)->setText( columnByName( i18n("Title") ), text );
    }
    for( QList<int>::Iterator it = selectedTracks.begin(); it != selectedTracks.end(); ++it )
    {
        TagData* tags = cdManager->getTags( device, *it );
        tags->title = text;
    }
}

void CDOpener::trackArtistChanged( const QString& text )
{
    if( !lTrackArtist->isEnabled() ) return;

    for( QList<QTreeWidgetItem*>::Iterator it = selectedItems.begin(); it != selectedItems.end(); ++it )
    {
        (*it)->setText( columnByName( i18n("Artist") ), text );
    }
    for( QList<int>::Iterator it = selectedTracks.begin(); it != selectedTracks.end(); ++it )
    {
        TagData* tags = cdManager->getTags( device, *it );
        tags->artist = text;
    }

    //trackList->resizeColumnToContents( columnByName( i18n("Artist") ) );
}

void CDOpener::trackComposerChanged( const QString& text )
{
    if( !lTrackComposer->isEnabled() ) return;

    for( QList<QTreeWidgetItem*>::Iterator it = selectedItems.begin(); it != selectedItems.end(); ++it )
    {
        (*it)->setText( columnByName( i18n("Composer") ), text );
    }
    for( QList<int>::Iterator it = selectedTracks.begin(); it != selectedTracks.end(); ++it )
    {
        TagData* tags = cdManager->getTags( device, *it );
        tags->composer = text;
    }

    //trackList->resizeColumnToContents( columnByName( i18n("Composer") ) );
}

void CDOpener::trackCommentChanged()
{
    QString text = tTrackComment->toPlainText();

    if( !tTrackComment->isEnabled() ) return;

    for( QList<int>::Iterator it = selectedTracks.begin(); it != selectedTracks.end(); ++it )
    {
        TagData* tags = cdManager->getTags( device, *it );
        tags->comment = text;
    }
}

void CDOpener::editTrackTitleClicked()
{
    lTrackTitle->setEnabled( true );
    lTrackTitle->setFocus();
    pTrackTitleEdit->hide();
    trackTitleChanged( lTrackTitle->text() );
}

void CDOpener::editTrackArtistClicked()
{
    lTrackArtist->setEnabled( true );
    lTrackArtist->setFocus();
    pTrackArtistEdit->hide();
    trackArtistChanged( lTrackArtist->text() );
}

void CDOpener::editTrackComposerClicked()
{
    lTrackComposer->setEnabled( true );
    lTrackComposer->setFocus();
    pTrackComposerEdit->hide();
    trackComposerChanged( lTrackComposer->text() );
}

void CDOpener::editTrackCommentClicked()
{
    tTrackComment->setEnabled( true );
    tTrackComment->setReadOnly( false );
    tTrackComment->setFocus();
    pTrackCommentEdit->hide();
    trackCommentChanged();
}

void CDOpener::proceedClicked()
{
    int trackCount = 0;
  
    for( int i=0; i<trackList->topLevelItemCount(); i++ )
    {
        trackCount++;
    }
    
    if( trackCount == 0 )
    {
        KMessageBox::error( this, i18n("You haven't selected a single so we can't proceed.") );
        return;
    }

    cdOpenerWidget->hide();
    pSaveCue->hide();
    cEntireCd->hide();
    options->show();
    page = ConversionOptionsPage;
    QFont font;
    font.setBold( false );
    lSelector->setFont( font );
    font.setBold( true );
    lOptions->setFont( font );
    pProceed->hide();
    pAdd->show();
}

void CDOpener::addClicked()
{
    QList<int> tracks;

    if( cEntireCd->isChecked() )
    {
        cdManager->setDiscTags( device, new TagData( cArtist->currentText(), cComposer->currentText(), lAlbum->text(), lAlbum->text(), cGenre->currentText(), "", 1, iDisc->value(), iYear->value(), cdManager->getTimeCount(device) ) );

        tracks.append( 0 );
    }
    else
    {
        for( int i=0; i<trackList->topLevelItemCount(); i++ )
        {
            if( trackList->topLevelItem(i)->checkState(0) == Qt::Checked )
            {
                TagData *tags = cdManager->getTags( device, i + 1 );
                if( cArtist->currentText() != i18n("Various Artists") ) tags->artist = cArtist->currentText();
                if( cComposer->currentText() != i18n("Various Composer") ) tags->composer = cComposer->currentText();
                tags->album = lAlbum->text();
                tags->disc = iDisc->value();
                tags->year = iYear->value();
                tags->genre = cGenre->currentText();

                tracks.append( i + 1 );
            }
        }

        emit addTracks( device, tracks, options->currentConversionOptions() );
    }
    
    accept();
}

// void CDOpener::addAsOneTrackClicked()
// {
//     // TODO save all options (album artist, disc, genre, etc.)
//     cdManager->setDiscTags( device,
//             new TagData( cArtist->currentText(), cComposer->currentText(),
//                           lAlbum->text(), /*cArtist->currentText() + " - " + */lAlbum->text(),
//                           cGenre->currentText(), "",
//                           1, iDisc->value(), iYear->value(),
//                           cdManager->getTimeCount(device) ) );
// 
//     emit addDisc( device );
//     accept();
// }

void CDOpener::saveCuesheetClicked()
{
    QString filename = KFileDialog::getSaveFileName( QDir::homePath(), "*.cue" );
    if( filename.isEmpty() ) return;

    QFile cueFile( filename );
    if( cueFile.exists() ) {
        int ret = KMessageBox::questionYesNo( this,
                    i18n("A file with this name already exists.\n\nDo you want to overwrite the existing one?"),
                    i18n("File already exists") );
        if( ret == KMessageBox::No ) return;
    }
    if( !cueFile.open( QIODevice::WriteOnly ) ) return;

    QString content;

    content.append( "TITLE \"" + lAlbum->text() + "\"\n" );
    content.append( "PERFORMER \"" + cArtist->currentText() + "\"\n" );
    content.append( "FILE \"\" MP3\n" );

    int INDEX = 0;
    bool addFrames = false;
    QList<TagData*> tags = cdManager->getTrackList( device );
    for( QList<TagData*>::Iterator it = tags.begin(); it != tags.end(); ++it ) {
        content.append( QString().sprintf("  TRACK %02i AUDIO\n",(*it)->track ) );
        content.append( "    TITLE \"" + (*it)->title + "\"\n" );
        content.append( "    PERFORMER \"" + (*it)->artist + "\"\n" );
        if( addFrames ) {
            content.append( QString().sprintf("    INDEX 01 %02i:%02i:37\n",INDEX/60,INDEX%60) );
            INDEX++;
            addFrames = false;
        }
        else {
            content.append( QString().sprintf("    INDEX 01 %02i:%02i:00\n",INDEX/60,INDEX%60) );
            addFrames = true;
        }

        INDEX += (*it)->length;
    }

    QTextStream ts( &cueFile );
    ts << content;

    cueFile.close();
}



