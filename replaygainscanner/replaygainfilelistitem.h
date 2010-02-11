
#ifndef REPLAYGAINFILELISTITEM_H
#define REPLAYGAINFILELISTITEM_H

#include <QTreeWidget>
#include <KUrl>

#include "metadata/tagengine.h"


class ReplayGainFileListItem : public QTreeWidgetItem
{
public:
    ReplayGainFileListItem( QTreeWidget *parent );
    
    ReplayGainFileListItem( QTreeWidgetItem *parent );
    
    ~ReplayGainFileListItem();
    
    enum Type {
        Track,
        Album
    } type;
    
    enum State {
        Waiting,
        Processing,
        Processed,
        Failed
    } state;
    
    KUrl url;
    QString albumName;
    QString codecName;
    int samplingRate;
    TagData *tags;
    
    int time;
    
    int processId;
    int take;
};

#endif // REPLAYGAINFILELISTITEM_H
