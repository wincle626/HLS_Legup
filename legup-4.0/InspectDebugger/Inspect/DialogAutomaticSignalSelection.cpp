/*
 * File:   DialogAutomaticSignalSelection.cpp
 * Author: nazanin
 * Automatic Signal Selection Dialog opens a form showing the list of signals to the user
 * based on the cycles..
 *
 * Created on December 15, 2013, 9:39 PM
 */

#include <fstream>

#include "DialogAutomaticSignalSelection.h"
#include "Utility.h"
#include "HWSignal.h"
#include "Globals.h"

DialogAutomaticSignalSelection::DialogAutomaticSignalSelection() {
    widget.setupUi(this);
    
    maxCycle = loadStatesToCyclesFile();
    
    std::vector<HWSignal*> mergedSignals;
    
    this->widget.tableWidgetSignalsToCycles->setColumnCount(maxCycle);    
    
    /* excluding memory controller signals (they are byDefaultAdded)
     * excluding state signals (LEGUP_F_*)
       excluding all other signals that are not appearing in any state */
    std::vector<HWSignal*> toBeShownSignals;
    
    for (int i = 0 ; i < Signals.size(); i++) {
        HWSignal *sig = Signals[i];
        if (sig->name.find("memory_controller_") != std::string::npos ||
                sig->name.find("LEGUP_F_") != std::string::npos ||
                sig->states.size() == 0)
            continue;
        toBeShownSignals.push_back(sig);
    }
    this->widget.tableWidgetSignalsToCycles->setRowCount(toBeShownSignals.size());
    for (int i = 0 ; i < toBeShownSignals.size(); i++) {
        HWSignal *sig = toBeShownSignals[i];
	int size = sig->states.size();        
        QTableWidgetItem *item = new QTableWidgetItem((sig->name + " (" + IntToString(size) + ")").c_str());
        this->widget.tableWidgetSignalsToCycles->setVerticalHeaderItem(i, item);
    }
    
    for (int i = 0 ; i < maxCycle; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(("Cycle #" + IntToString(i+1)).c_str());
        this->widget.tableWidgetSignalsToCycles->setHorizontalHeaderItem(i, item);
    }
    
    for (int i = 0 ; i < toBeShownSignals.size(); i++) {
        HWSignal *sig = toBeShownSignals[i];
        for (int s = 0 ; s < sig->states.size(); s++) {
            State *st = sig->states[s];
            std::vector<int> cycles = statesToCycles[st->name];
            for (int cycleIdx = 0 ; cycleIdx < cycles.size(); cycleIdx++) {
                int cycle = cycles[cycleIdx];
                QTableWidgetItem* item = new QTableWidgetItem("");
                item->setBackgroundColor(Qt::green);
                this->widget.tableWidgetSignalsToCycles->setItem(i, cycle + 1, item);
            }
        }            
    }    
}

DialogAutomaticSignalSelection::~DialogAutomaticSignalSelection() {
}
