/*
 * File:   DialogSelectSignals.cpp
 * Author: nazanin
 * DialogSelectSignals form shows the list of signals to the user. Signals can be selected
 * based on the Functions and IRs.. Extra signals can be loaded as well.
 *
 * Created on October 29, 2013, 12:17 PM
 */

#include "DialogSelectSignals.h"
#include "Globals.h"

DialogSelectSignals::DialogSelectSignals() {
    widget.setupUi(this);
    DB_ID_ROLE = 1000;
    
    filterNames.push_back("top:top_inst|memory_controller_address_a");
    filterNames.push_back("top:top_inst|memory_controller_address_b");
    filterNames.push_back("top:top_inst|memory_controller_in_a");
    filterNames.push_back("top:top_inst|memory_controller_in_b");
    filterNames.push_back("top:top_inst|memory_controller_enable_a");
    filterNames.push_back("top:top_inst|memory_controller_enable_b");
    filterNames.push_back("top:top_inst|memory_controller_out_a");
    filterNames.push_back("top:top_inst|memory_controller_out_b");
    filterNames.push_back("top:top_inst|memory_controller_size_a");
    filterNames.push_back("top:top_inst|memory_controller_size_b");
    filterNames.push_back("top:top_inst|memory_controller_write_enable_a");
    filterNames.push_back("top:top_inst|memory_controller_write_enable_b");
    
    InitializeData();
    returnStatus = 0;//by default cancel..    
}

DialogSelectSignals::~DialogSelectSignals() {
    
}

void DialogSelectSignals::InitializeData() {    
    fillFunctionsList();
    //fillExtraSignalsList();
    fillByDefaultAddedSignals();
}

void DialogSelectSignals::fillExtraSignalsList() {
    this->widget.tableWidgetExtra->clear();
    this->widget.tableWidgetExtra->setColumnCount(2);
    this->widget.tableWidgetExtra->setColumnWidth(0, this->widget.tableWidgetExtra->width());
    this->widget.tableWidgetExtra->setColumnWidth(1, 100);
    this->widget.tableWidgetExtra->setRowCount(nodeNames.size());
    this->widget.tableWidgetExtra->setHorizontalHeaderItem(0, new QTableWidgetItem("Signal"));
    this->widget.tableWidgetExtra->setHorizontalHeaderItem(1, new QTableWidgetItem("Width"));
        
    std::map<std::string, int>::iterator sit;
    int idx = 0;
    for (sit = nodeNames.begin(); sit != nodeNames.end(); ++sit) {        
        
        QString str((*sit).first.c_str());
        QTableWidgetItem* item = new QTableWidgetItem(str);
        item->setData(DB_ID_ROLE, -1);
        this->widget.tableWidgetExtra->setItem(idx, 0, item);
        
        QString widthStr(IntToString((*sit).second).c_str());
        QTableWidgetItem *widthItem = new QTableWidgetItem(widthStr);
        widthItem->setData(DB_ID_ROLE, -1);
        this->widget.tableWidgetExtra->setItem(idx, 1, widthItem);
        
        if (selectedExtraSignals.find((*sit).first) != selectedExtraSignals.end()) {
            item->setBackgroundColor(QColor::fromRgb(0, 255, 0));
            widthItem->setBackgroundColor(QColor::fromRgb(0, 255, 0));
        }
        
        idx++;
    }
}

void DialogSelectSignals::fillFunctionsList() {
    
    this->widget.tableWidgetFunctions->clear();
    this->widget.tableWidgetFunctions->setColumnCount(1);    
    this->widget.tableWidgetFunctions->setColumnWidth(0, this->widget.tableWidgetFunctions->width());
    this->widget.tableWidgetFunctions->setRowCount(functions.size()-1);
    this->widget.tableWidgetFunctions->setHorizontalHeaderItem(0, new QTableWidgetItem("Function Name"));
    
    std::vector<Function*>::iterator fit;
    int idx = 0;
    for (fit = functions.begin(); fit != functions.end(); ++fit) {
        if ((*fit)->name.compare("GLOBAL") == 0)
            continue;
        QString str((*fit)->name.c_str());
        QTableWidgetItem* item = new QTableWidgetItem(str);
        item->setData(DB_ID_ROLE, (*fit)->id);
        this->widget.tableWidgetFunctions->setItem(idx, 0, item);
        idx++;
    }
}

void DialogSelectSignals::fillIRList(std::vector<IRInstruction*>& toBeShownIRs) {
    this->widget.tableWidgetIRs->clear();
    this->widget.tableWidgetIRs->setColumnCount(1);
    this->widget.tableWidgetIRs->setColumnWidth(0, this->widget.tableWidgetIRs->width());    
    this->widget.tableWidgetIRs->setRowCount(toBeShownIRs.size());
    this->widget.tableWidgetIRs->setHorizontalHeaderItem(0, new QTableWidgetItem("Instruction"));
    
    std::vector<IRInstruction*>::iterator iit;
    int idx = 0;
    for (iit = toBeShownIRs.begin(); iit != toBeShownIRs.end(); ++iit) {
        QString str((*iit)->dump.c_str());
        QTableWidgetItem *item = new QTableWidgetItem(str);
        item->setData(DB_ID_ROLE, (*iit)->id);
        this->widget.tableWidgetIRs->setItem(idx, 0, item);
        idx++;
    }
}

void DialogSelectSignals::fillSignalList(std::vector<HWSignal*>& toBeShownSignals) {
    this->widget.tableWidgetSignals->clear();
    this->widget.tableWidgetSignals->setColumnCount(1);
    this->widget.tableWidgetSignals->setColumnWidth(0, this->widget.tableWidgetSignals->width());        
    
    std::vector<HWSignal*> filteredList;
    
    std::vector<HWSignal*>::iterator sit;    
    for (sit = toBeShownSignals.begin(); sit != toBeShownSignals.end(); ++sit) {
        HWSignal* sig = (*sit);
        if (sig->name.find("memory_controller_") != std::string::npos ||
                sig->name.find("LEGUP_F_") != std::string::npos ||
                sig->states.size() == 0)
            continue;
        filteredList.push_back(sig);        
    }
    
    this->widget.tableWidgetSignals->setRowCount(filteredList.size());
    this->widget.tableWidgetSignals->setHorizontalHeaderItem(0, new QTableWidgetItem("Signals"));
        
    int idx = 0;
    for (sit = filteredList.begin(); sit != filteredList.end(); ++sit) {        
        QString str((*sit)->name.c_str());
        QTableWidgetItem *item = new QTableWidgetItem(str);
        item->setData(DB_ID_ROLE, (*sit)->id);
        if (std::find(selectedSignalIds.begin(), selectedSignalIds.end(), (*sit)->id) != selectedSignalIds.end())
            item->setBackgroundColor(QColor::fromRgb(0, 255, 0));
        this->widget.tableWidgetSignals->setItem(idx, 0, item);
        idx++;
    }
}

void DialogSelectSignals::fillSelectedSignalsList() {
    this->widget.tableWidgetSelectedSignals->clear();
    this->widget.tableWidgetSelectedSignals->setColumnCount(2);
    this->widget.tableWidgetSelectedSignals->setColumnWidth(0, 400);
    this->widget.tableWidgetSelectedSignals->setColumnWidth(1, 100);
    this->widget.tableWidgetSelectedSignals->setRowCount(selectedSignalIds.size() + selectedExtraSignals.size());
    this->widget.tableWidgetSelectedSignals->setHorizontalHeaderItem(0, new QTableWidgetItem("Selected Signals"));
    this->widget.tableWidgetSelectedSignals->setHorizontalHeaderItem(1, new QTableWidgetItem("Width"));
    
    for (int i = 0 ; i < selectedSignalIds.size(); i++) {
        HWSignal* sig = IdsToSignals[selectedSignalIds[i]];
        QString str(sig->name.c_str());
        QTableWidgetItem *item = new QTableWidgetItem(str);
        item->setData(DB_ID_ROLE, selectedSignalIds[i]);        
        this->widget.tableWidgetSelectedSignals->setItem(i, 0, item);
        
        QString widthStr(IntToString(sig->width).c_str());
        QTableWidgetItem *widthItem = new QTableWidgetItem(widthStr);
        widthItem->setData(DB_ID_ROLE, selectedSignalIds[i]);
        this->widget.tableWidgetSelectedSignals->setItem(i, 1, widthItem);
    }    
    std::map<std::string, int>::iterator it;
    int i = 0;
    for (it = selectedExtraSignals.begin(); it != selectedExtraSignals.end(); ++it) {
        QString str((*it).first.c_str());
        QTableWidgetItem *item = new QTableWidgetItem(str);
        item->setData(DB_ID_ROLE, -1);
        this->widget.tableWidgetSelectedSignals->setItem(i + selectedSignalIds.size(), 0, item);
        
        QString widthStr(IntToString((*it).second).c_str());
        QTableWidgetItem *widthItem = new QTableWidgetItem(widthStr);
        widthItem->setData(DB_ID_ROLE, -1);
        this->widget.tableWidgetSelectedSignals->setItem(i + selectedSignalIds.size(), 1, widthItem);
        i++;
    }    
}

void DialogSelectSignals::tableWidgetFunctions_ItemClicked(QTableWidgetItem* item) {
    this->selectedFunctionId = item->data(DB_ID_ROLE).toInt();
    std::vector<IRInstruction*> toBeShownInstructions;
    std::vector<IRInstruction*>::iterator iit;
    for (iit = IRInstructions.begin(); iit != IRInstructions.end(); ++iit) {
        if ((*iit)->function_id != selectedFunctionId)
            continue;
        toBeShownInstructions.push_back((*iit));
    }
    fillIRList(toBeShownInstructions);
    std::vector<HWSignal*> toBeShownSignals;
    for (iit = toBeShownInstructions.begin(); iit != toBeShownInstructions.end(); ++iit)
    {
        
        std::vector<HWSignal*>::iterator sit;
        for(sit = (*iit)->signalList.begin(); sit != (*iit)->signalList.end(); ++sit)
        {
            if(std::find(toBeShownSignals.begin(), toBeShownSignals.end(), (*sit))== toBeShownSignals.end())
                toBeShownSignals.push_back((*sit));
        }
        
        //QString inst(IntToString((*iit)).c_str());
        //QTableWidgetItem item = new QTableWidgetItem(inst);
        //tableWidgetIRs_ItemClicked(item);
    }
    fillSignalList(toBeShownSignals);
    //tableWidgetIRs_ItemClicked(item);
}

void DialogSelectSignals::tableWidgetIRs_ItemClicked(QTableWidgetItem* item) {
    this->selectedInstructionId = item->data(DB_ID_ROLE).toInt();
    IRInstruction *instr = IRIdsToInstructions[selectedInstructionId];
    fillSignalList(instr->signalList);
}

void DialogSelectSignals::tableWidgetSignals_ItemDoubleClicked(QTableWidgetItem* item) {
    int signalId = item->data(DB_ID_ROLE).toInt();
    HWSignal* sig = IdsToSignals[signalId];
    if (std::find(selectedSignalIds.begin(), selectedSignalIds.end(), signalId) == selectedSignalIds.end()) {
        selectedSignalIds.push_back(signalId);
        fillSelectedSignalsList();
        item->setBackgroundColor(QColor::fromRgb(0, 255, 0));
    }
    else
    {
        QMessageBox mbox;
        mbox.setWindowModality(Qt::NonModal);
        mbox.setInformativeText("Signal is already added!");
        mbox.exec();
    }
}

void DialogSelectSignals::tableWidgetSelectedSignals_ItemDoubleClicked(QTableWidgetItem* item) {
    int signalId = item->data(DB_ID_ROLE).toInt();
    if (signalId == -1) {//extra signal
        //selectedExtraSignals.erase(std::find(selectedExtraSignals.begin(), selectedExtraSignals.end(), item->text().toStdString()));
        selectedExtraSignals.erase(selectedExtraSignals.find(item->text().toStdString()));
        fillExtraSignalsList();
    } else {
        selectedSignalIds.erase(std::find(selectedSignalIds.begin(), selectedSignalIds.end(), signalId));        
    }
    fillSelectedSignalsList();
    IRInstruction *instr = IRIdsToInstructions[selectedInstructionId];
    fillSignalList(instr->signalList);
}

void DialogSelectSignals::actionLoadAllOnChipSignals() {
    std::ifstream in;
    in.open((workDir + nodeNamesFilename).c_str());    
    while(in.good()) {        
        std::string line;
        getline(in, line);
        if (line.compare("") == 0)
            continue;        
        std::string name = split(line, ';')[0];
        if (name.compare("") == 0)
            continue;
        std::string pureName = split(name, '[')[0];
        nodeNames[pureName]++;
        /*if (std::find(nodeNames.begin(), nodeNames.end(), pureName) == nodeNames.end())
            nodeNames.push_back(pureName);*/
    }    
    in.close();
    fillExtraSignalsList();
}

void DialogSelectSignals::tableWidgetExtra_ItemDoubleClicked(QTableWidgetItem* item) {
    std::string itemText = item->text().toStdString();
    if (selectedExtraSignals.find(itemText) == selectedExtraSignals.end()) {
        selectedExtraSignals[itemText] = nodeNames[itemText];
        fillSelectedSignalsList();
        item->setBackgroundColor(QColor::fromRgb(0, 255, 0));
    }
    else {
        QMessageBox mbox;
        mbox.setWindowModality(Qt::NonModal);
        mbox.setInformativeText("Signal is already added!");
        mbox.exec();
    }
}

void DialogSelectSignals::fillByDefaultAddedSignals(){
    std::ifstream in;
    in.open((workDir + nodeNamesFilename).c_str());    
    while(in.good()) {        
        std::string line;
        getline(in, line);
        if (line.compare("") == 0)
            continue;        
        std::string name = split(line, ';')[0];
        if (name.compare("") == 0)
            continue;
        std::string pureName = split(name, '[')[0];
        bool shouldFilter = false;
        for (int i = 0 ; i < filterNames.size(); i++) {
            if (pureName.find(filterNames[i]) != std::string::npos) {
                shouldFilter = true;
                break;
            }
        }
        if (shouldFilter)
            byDefaultAddedSignals[pureName]++;
            
    }
    in.close();    
    
    std::map<std::string, int>::iterator it;
    for (it = byDefaultAddedSignals.begin(); it != byDefaultAddedSignals.end(); ++it) {
        if ((*it).second != 1)
            (*it).second--;
    }
    
}

void DialogSelectSignals::pushButtonLoadSignals_clicked() {
    std::ifstream in;
    in.open((workDir + nodeNamesFilename).c_str());    
    while(in.good()) {        
        std::string line;
        getline(in, line);
        if (line.compare("") == 0)
            continue;        
        std::string name = split(line, ';')[0];
        if (name.compare("") == 0)
            continue;
        std::string pureName = split(name, '[')[0];
        bool shouldFilter = false;
        for (int i = 0 ; i < filterNames.size(); i++) {
            if (pureName.find(filterNames[i]) != std::string::npos) {
                shouldFilter = true;
                break;
            }
        }
        if (!shouldFilter)
            nodeNames[pureName]++;
        //else
        //    byDefaultAddedSignals[pureName]++;
        /*if (std::find(nodeNames.begin(), nodeNames.end(), pureName) == nodeNames.end())
            nodeNames.push_back(pureName);*/        
    }
    in.close();    
    
    std::map<std::string, int>::iterator it;
    for (it = nodeNames.begin(); it != nodeNames.end(); ++it) {
        if ((*it).second != 1)
            (*it).second--;            
    }
   // for (it = byDefaultAddedSignals.begin(); it != byDefaultAddedSignals.end(); ++it) {
   //     if ((*it).second != 1)
   //         (*it).second--;
  //  }
    fillExtraSignalsList();
}
