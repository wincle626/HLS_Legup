/*
 * scheduleitem.cpp
 *
 *  Created on: Jan 10, 2013
 *      Author: johnqin
 */

#include <scheduleitem.h>
using namespace std;

ScheduleItem::ScheduleItem(const QString & text):
QStandardItem(text)
{

}
/*
QVariant
Item::data( int role = Qt::UserRole + 1 ) const
{
	//return QVariant();
	 QList<QVariant> textList;
	 textList << "list item";
	 QString text("test");
	 return QVariant(text);
}
*/
int
ScheduleItem::type () const
{
	return 1001;
}





