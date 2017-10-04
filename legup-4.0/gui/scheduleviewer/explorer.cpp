/*
 * explorer.cpp
 *
 *  Created on: Nov 7, 2012
 *      Author: johnqin
 */

#include "explorer.h"

Explorer::Explorer() {
	// TODO Auto-generated constructor stub

}

Explorer::~Explorer() {
	// TODO Auto-generated destructor stub
}

void
Explorer::currentChanged ( const QModelIndex & current, const QModelIndex & previous )
{
    (void)previous;
	emit currentSelected(current);
}


