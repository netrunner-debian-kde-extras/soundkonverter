
#ifndef DIRDIALOG_H
#define DIRDIALOG_H

#include <kdialog.h>

class Config;
class Options;
class ConversionOptions;

class QLabel;
class QCheckBox;
class KPushButton;
class KUrlRequester;
class KListWidget;


/**
 * @short The Replay Gain Tool
 * @author Daniel Faust <hessijames@gmail.com>
 * @version 0.3
 */
class DirDialog : public KDialog
{
    Q_OBJECT
public:
    enum DialogPage {
        DirOpenPage,
        ConversionOptionsPage
    };

    enum Mode {
        Convert    = 0x0001,
        ReplayGain = 0x0002
    };

    /** Constructor */
    DirDialog( Config *config, Mode _mode, QWidget *parent=0, Qt::WFlags f=0 );

    /** Destructor */
    virtual ~DirDialog();

private slots:
    void proceedClicked();
    void addClicked();
    void selectAllClicked();
    void selectNoneClicked();

private:
    /** the widget for selecting the directory */
    QWidget *dirOpenerWidget;
    /** the conversion options editor widget */
    Options *options;
    /** the current page */
    DialogPage page;
    /** th dialog mode */
    Mode mode;

    QLabel *lSelector;
    QLabel *lOptions;

    KUrlRequester *uDirectory;
    KListWidget *fileTypes;
    KPushButton *pSelectAll;
    KPushButton *pSelectNone;
    QCheckBox *cRecursive;
    
signals:
    void done( const KUrl& directory, bool recursive, const QStringList& codecList, ConversionOptions *conversionOptions = 0 );
};


#endif
