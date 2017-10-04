/* 
 * File:   DialogAutomaticSignalSelection.h
 * Author: nazanin
 *
 * Created on December 15, 2013, 9:39 PM
 */

#ifndef _DIALOGAUTOMATICSIGNALSELECTION_H
#define	_DIALOGAUTOMATICSIGNALSELECTION_H

#include "ui_DialogAutomaticSignalSelection.h"
#include "HWSignal.h"
#include "Utility.h"
#include <iostream>

/*
extern std::vector<HWSignal*> Signals;
extern std::string workDir;
extern std::string statesToCyclesFileAddress;
extern std::map<std::string, std::vector<int> > statesToCycles;
*/

class DialogAutomaticSignalSelection : public QDialog {
    Q_OBJECT
public:
    DialogAutomaticSignalSelection();
    virtual ~DialogAutomaticSignalSelection();
private:
    Ui::DialogAutomaticSignalSelection widget;    
    
    int maxCycle;        
};

#endif	/* _DIALOGAUTOMATICSIGNALSELECTION_H */
