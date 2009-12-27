//
// C++ Interface: opener
//
// Description: 
//
//
// Author: Daniel Faust <hessijames@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OPENER_H
#define OPENER_H

#include <KDialog>

#include <KUrl>

class Config;
class Options;
class QLabel;
class ConversionOptions;
class KDialog;
class KFileWidget;
class KPushButton;
class KUrlRequester;

/**
	@author Daniel Faust <hessijames@gmail.com>
*/
class Opener : public KDialog
{
    Q_OBJECT
public:
    enum DialogPage {
        FileOpenPage,
        ConversionOptionsPage
    };
    enum OpenMode {
        Files,
        Url,
        Playlist
    };

    Opener( Config *_config, OpenMode _openMode, QWidget *parent=0, Qt::WFlags f=0 );
    ~Opener();

    DialogPage currentPage() { return page; }

private:
    Config *config;
    OpenMode openMode;
  
    KFileWidget *fileWidget;
    KUrlRequester *urlRequester;
    Options *options;
    DialogPage page;
    QLabel *lSelector;
    QLabel *lOptions;
    KUrl::List urls;
    KPushButton *pProceed;
    KPushButton *pAdd;
    KPushButton *pCancel;
    QLabel *formatHelp;

private slots:
    void proceedClickedSlot();
    void okClickedSlot();
    void showHelp();

signals:
    void done( const KUrl::List& files, ConversionOptions *conversionOptions );


};

#endif
