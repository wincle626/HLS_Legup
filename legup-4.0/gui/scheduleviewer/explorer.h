/*
 * explorer.h
 *
 *  Created on: Nov 7, 2012
 *      Author: johnqin
 */

#ifndef EXPLORER_H_
#define EXPLORER_H_

#include <QtGui>

class Explorer: public QTreeView {
	Q_OBJECT
public:
	Explorer();
	~Explorer();
	QSize minimumSizeHint() const {
		return QSize(50, 100);
	}

	QSize sizeHint() const {
		return QSize(200, 500);
	}
signals:
    void currentSelected(const QModelIndex & current);
private slots:
	void currentChanged ( const QModelIndex & current, const QModelIndex & previous );

};

#endif /* EXPLORER_H_ */
