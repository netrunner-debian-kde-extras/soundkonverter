
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
  
    KUrl url;
    QString codecName;
    TagData *tags;
    
    int time;
    
    bool processing;
    
    int processId;
};

#endif // REPLAYGAINFILELISTITEM_H
