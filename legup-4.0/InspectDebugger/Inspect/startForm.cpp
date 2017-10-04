/*
 * File:   startForm.cpp
 * Author: nazanin
 *
 * Created on March 25, 2014, 2:17 PM
 */

#include "startForm.h"

startForm::startForm() {
    widget.setupUi(this);
    this->mode_number = 1;
    this->widget.radioButtonSimulationRun->setChecked(true);
}

startForm::~startForm() {
}

void startForm::startForm_rejected(){
    this->mode_number = -1;
}

void startForm::pushButtonStart_clicked() {
    if (this->widget.radioButtonSimulationRun->isChecked())
        this->mode_number = 1;
    else if (this->widget.radioButtonOnChipRun->isChecked())
        this->mode_number = 2;
    else if (this->widget.radioButtonGDB->isChecked())
        this->mode_number = 3;
    else if (this->widget.radioButtonGDBSync->isChecked())
        this->mode_number = 4;
    else if (this->widget.radioButtonBugDetection->isChecked())
        this->mode_number = 5;
    else if (this->widget.radioButtonOnChipBugDetection->isChecked())
        this->mode_number = 6;
        
    this->accept();
    this->close();
}
