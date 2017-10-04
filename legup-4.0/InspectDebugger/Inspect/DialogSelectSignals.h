/* 
 * File:   DialogSelectSignals.h
 * Author: nazanin
 *
 * Created on October 29, 2013, 12:17 PM
 */

#ifndef _DIALOGSELECTSIGNALS_H
#define	_DIALOGSELECTSIGNALS_H

#include "ui_DialogSelectSignals.h"
#include "QMessageBox"
#include "DataAccess.h"
#include <iostream>
#include <fstream>

/*
extern DataAccess *DA;
extern std::vector<HWSignal*> Signals;
extern std::vector<Variable*> Variables;
extern std::map<int, Variable*> IdsToVariables;
extern std::vector<IRInstruction*> IRInstructions;
extern std::map<int, IRInstruction*> IRIdsToInstructions;
extern std::map<Variable*, VariableUpdateInfo*> variablesToUpdateInfo;
extern std::vector<Function*> functions;
extern std::map<int, Function*> IdsToFunctions;

extern std::string workDir;
extern std::string nodeNamesFilename;
*/

class DialogSelectSignals : public QDialog {
    Q_OBJECT
public:
    DialogSelectSignals();
    virtual ~DialogSelectSignals();
private:
    Ui::DialogSelectSignals widget;
    
    void InitializeData();
    void fillFunctionsList();
    void fillIRList(std::vector<IRInstruction*>& toBeShownIRs);
    void fillSignalList(std::vector<HWSignal*>& toBeShownSignals);
    void fillSelectedSignalsList();
    void fillExtraSignalsList();
    void fillByDefaultAddedSignals();
    
    int DB_ID_ROLE;
    int selectedFunctionId;
    int selectedInstructionId;
    
    std::vector<std::string> filterNames;
    
public:
    std::vector<int> selectedSignalIds;    
    std::map<std::string, int> selectedExtraSignals;    
    std::map<std::string, int> nodeNames;
    std::map<std::string, int> byDefaultAddedSignals;
    int returnStatus;
    
public slots:
    void tableWidgetFunctions_ItemClicked(QTableWidgetItem*);
    void tableWidgetIRs_ItemClicked(QTableWidgetItem*);
    void tableWidgetSignals_ItemDoubleClicked(QTableWidgetItem*);
    void tableWidgetSelectedSignals_ItemDoubleClicked(QTableWidgetItem*);
    void tableWidgetExtra_ItemDoubleClicked(QTableWidgetItem*);
    void actionLoadAllOnChipSignals();
    void pushButtonLoadSignals_clicked();
};

#endif	/* _DIALOGSELECTSIGNALS_H */
