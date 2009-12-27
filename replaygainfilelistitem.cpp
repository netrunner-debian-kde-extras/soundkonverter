
#include "replaygainfilelistitem.h"


ReplayGainFileListItem::ReplayGainFileListItem( QTreeWidget *parent )
    : QTreeWidgetItem( parent )
{
    processing = false;
    tags = 0;
}

ReplayGainFileListItem::ReplayGainFileListItem( QTreeWidgetItem* parent )
    : QTreeWidgetItem( parent )
{
    processing = false;
    tags = 0;
}

ReplayGainFileListItem::~ReplayGainFileListItem()
{
//     if( tags ) delete tags;
}
