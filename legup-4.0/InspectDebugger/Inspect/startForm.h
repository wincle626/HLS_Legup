/* 
 * File:   startForm.h
 * Author: nazanin
 *
 * Created on March 25, 2014, 2:17 PM
 */

#ifndef _STARTFORM_H
#define	_STARTFORM_H

#include "ui_startForm.h"

class startForm : public QDialog {
    Q_OBJECT
public:
    startForm();
    virtual ~startForm();
    
    int mode_number;    
    
    public slots:
        void pushButtonStart_clicked(); 
        void startForm_rejected();
                
private:
    Ui::startForm widget;    
};

#endif	/* _STARTFORM_H */
