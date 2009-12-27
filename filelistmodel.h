//
// C++ Interface: filelistmodel
//
// Description: 
//
//
// Author: Daniel Faust <hessijames@gmail.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include <QStandardItemModel>

/**
	@author Daniel Faust <hessijames@gmail.com>
*/
class FileListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    FileListModel();

    ~FileListModel();

};

#endif
