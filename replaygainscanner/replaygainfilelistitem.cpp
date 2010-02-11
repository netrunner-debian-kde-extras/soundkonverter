
#include "replaygainfilelistitem.h"


ReplayGainFileListItem::ReplayGainFileListItem( QTreeWidget *parent )
    : QTreeWidgetItem( parent )
{
    state = Waiting;
    tags = 0;
    samplingRate = 0;
    take = 0;
    processId = -1;
}

ReplayGainFileListItem::ReplayGainFileListItem( QTreeWidgetItem *parent )
    : QTreeWidgetItem( parent )
{
    state = Waiting;
    tags = 0;
    samplingRate = 0;
    take = 0;
    processId = -1;
}

ReplayGainFileListItem::~ReplayGainFileListItem()
{
//     if( tags ) delete tags;
}
