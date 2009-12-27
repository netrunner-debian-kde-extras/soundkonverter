

#ifndef CDOPENER_H
#define CDOPENER_H

#include <kdialog.h>

class CDManager;
class TagEngine;
class Config;
class Options;
class ConversionOptions;
class QTreeWidget;
class KPushButton;
class KLineEdit;
class KComboBox;
class KIntSpinBox;
class KTextEdit;
class QGroupBox;
class QTreeWidgetItem;
class QLabel;
class QCheckBox;


/**
 * @short Shows a dialog for selecting files from a CD
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 1.0
 */
class CDOpener : public KDialog
{
     Q_OBJECT
public:
    enum DialogPage {
        CdOpenPage,
        ConversionOptionsPage
    };

//     enum Mode {
//         all_tracks,
//         selected_tracks,
//         full_cd
//     };

    /** Constructor */
    CDOpener( Config *_config, CDManager *_cdManager, const QString& _device, QWidget *parent = 0 /*Mode default_mode = all_tracks, const QString& default_text = "",*/, Qt::WFlags f=0 );

    /** Destructor */
    virtual ~CDOpener();

    /** true if no CD was found (don't execute the dialog) */
    bool noCD;

private:
    /** the widget for selecting and editing the cd tracks */
    QWidget *cdOpenerWidget;
    /** the conversion options editor widget */
    Options *options;
    /** the current page */
    DialogPage page;

    QLabel *lSelector;
    QLabel *lOptions;

    /** A list of all tracks on the CD */
    QTreeWidget *trackList;

    /** A combobox for entering the artist or selecting VA of the whole CD */
    KComboBox *cArtist;
    /** A combobox for entering the composer or selecting VC of the whole CD */
    KComboBox *cComposer;
    /** A lineedit for entering the album name */
    KLineEdit *lAlbum;
    /** A spinbox for entering or selecting the disc number */
    KIntSpinBox *iDisc;
    /** A spinbox for entering or selecting the year of the album */
    KIntSpinBox *iYear;
    /** A combobox for entering or selecting the genre of the album */
    KComboBox *cGenre;

    /** Request CDDB information */
//     KPushButton *pCDDB;

    /** The groupbox shows the selected track numbers */
    QGroupBox *tagGroupBox;

    /** Set the focus of the tag editor to the track over it */
    KPushButton *pTrackUp;
    /** Set the focus of the tag editor to the track under it */
    KPushButton *pTrackDown;

    /** A lineedit for entering the title of track */
    KLineEdit *lTrackTitle;
    KPushButton *pTrackTitleEdit;
    /** A lineedit for entering the artist of a track */
    KLineEdit *lTrackArtist;
    KPushButton *pTrackArtistEdit;
    /** A lineedit for entering the composer of a track */
    KLineEdit *lTrackComposer;
    KPushButton *pTrackComposerEdit;
    /** A textedit for entering a comment for a track */
    KTextEdit *tTrackComment;
    KPushButton *pTrackCommentEdit;

    /** Save the tag information to a cue file */
    KPushButton *pSaveCue;
    /** Rip enitre CD as one track */
    QCheckBox *cEntireCd;
    /** Add selected tracks to the file list and quit the dialog */
    KPushButton *pAdd;
    /** proceed to select conversion options */
    KPushButton *pProceed;
    /** Quit the dialog */
    KPushButton *pCancel;

    CDManager *cdManager;
    Config *config;

    QString device;

    QList<int> selectedTracks;
    QList<QTreeWidgetItem*> selectedItems;

    int columnByName( const QString& name ); // should be obsolete

private slots:
    void trackChanged();
    void trackUpPressed();
    void trackDownPressed();
    void artistChanged( const QString& text );
    void composerChanged( const QString& text );
    void trackTitleChanged( const QString& text );
    void trackArtistChanged( const QString& text );
    void trackComposerChanged( const QString& text );
    void trackCommentChanged();
    void editTrackTitleClicked();
    void editTrackArtistClicked();
    void editTrackComposerClicked();
    void editTrackCommentClicked();

    void proceedClicked();
    void addClicked();
//     void addAsOneTrackClicked();
    void saveCuesheetClicked();

signals:
    void addTracks( const QString& device, QList<int> trackList, ConversionOptions *conversionOptions );
    void addDisc( const QString& device, ConversionOptions *conversionOptions );
    //void openCuesheetEditor( const QString& content );
};

#endif // CDOPENER_H
