/*
 * File:   newForm.cpp
 * Author: nazanin
 *
 * Created on July 10, 2013, 11:03 PM
 */

#include "formMain.h"

#ifndef PYTHON_WRAPPER
#include "OnChipDebugEngine.h"
#include "GDBWrapper.h"
#include <qt4/QtGui/qmessagebox.h>
#include <qt4/QtGui/qlabel.h>
#include <qt4/QtGui/qcolor.h>
#include <qt4/QtGui/qtextcursor.h>
#include <qt4/QtGui/qlistwidget.h>
#endif

#include <sys/param.h>
#include <math.h>

#ifndef PYTHON_WRAPPER
#include <qt4/QtGui/qscrollbar.h>
#endif

#include <time.h>
#include <dirent.h>
#include "Globals.h"

//default constructor of the form.
//customizing the UI based on the program's mode
#ifdef PYTHON_WRAPPER
formMain::formMain() {
#else
formMain::formMain(SYSTEM_MODE _mode) {
#endif

#ifndef PYTHON_WRAPPER
    //setting up the basic Qt GUI
    widget.setupUi(this);
#endif
    
#ifdef PYTHON_WRAPPER
	this->mode = MODELSIM;
#else
    this->mode = _mode;
#endif
       
#ifndef PYTHON_WRAPPER
    //setting the debug tab as the deafult tab
    this->widget.tabWidget->setCurrentIndex(0);
    
    if ((mode == GDB_BUG_DETECTION )) {
        
        //this->widget.tabDiscrepanyLog->setVisible(false);
        this->widget.tabWidget->removeTab(2);
        this->widget.tabWidget->removeTab(2);
        //this->widget.tabSourceCode->setVisible(false);
        //this->widget.tabVerilogCode->setVisible(false);
    }
    
    //hiding the vertical scroll bar for the text line number
    this->widget.textEditLineNumber->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    this->widget.textEditLineNumber->verticalScrollBar()->hide();
    
    //connecting the Code vertical scroll bar to line number scroll bar
    connect(this->widget.textEditHighLevel->verticalScrollBar(), SIGNAL(valueChanged(int)), 
            this->widget.textEditLineNumber->verticalScrollBar(), SLOT(setValue(int)));
        
    if (mode == GDB_SYNC || mode == GDB_BUG_DETECTION) {
        //adding a status label widget
        statusLabel = new QLabel(this->widget.statusbar);
        changeStatusLabelColor("black");
        this->widget.statusbar->addWidget(statusLabel);   
        
        //removing the varlogs files
        remove ((workDir + "varlogs_gdb.txt").c_str());
        remove ((workDir + "varlogs_hw.txt").c_str());
        
        if (mode == GDB_BUG_DETECTION) {
            this->widget.pushButtonContinue->hide();
            this->widget.pushButtonSingleStepping->setText("Start Debug!");
        }                
    } else if (mode == GDB) {
        this->widget.treeWidgetWatch->hide();
        this->widget.labelWatch->hide();
        
        this->widget.treeWidgetHWValues->hide();
        this->widget.labelWatch_3->hide();
        
        this->widget.radioButtonStepInto->hide();
        this->widget.radioButtonStepOver->hide();        
        //this->widget.label->setText("Status:");
        this->widget.label->hide();
        this->widget.labelCurrentState->hide();
        
        this->widget.actionView_IR_Instructions->setChecked(false);
        this->widget.treeWidgetIR->setVisible(false);
        this->widget.labelIR->setVisible(false);
        
        this->widget.actionView_verilog_Code->setChecked(false);
        this->widget.listWidgetHardware->setVisible(false);
        this->widget.labelHardware->setVisible(false);
        this->widget.tabWidget->removeTab(1);
        
    } else {
        //do not show the discrepancy check tab
        this->widget.tabWidget->removeTab(1);
        this->widget.tableWidgetSWVariables->setVisible(false);
        this->widget.labelWatchSWVariables->setVisible(false);
        
        if (mode == ONCHIP_VS_TIMING_SIM) {
            this->widget.pushButtonContinue->hide();
            this->widget.pushButtonSingleStepping->setText("On-Chip Debug!");
        }
    }
#endif
    
    Initialize();
}

formMain::~formMain() {
    
}

#ifndef PYTHON_WRAPPER
void formMain::changeStatusLabelColor(std::string color) {
    if (color == "black")
        this->statusLabel->setStyleSheet("QLabel { color: black; }");
    else if (color == "red")
        this->statusLabel->setStyleSheet("QLabel { color: red; }");
    else if (color == "green")
        this->statusLabel->setStyleSheet("QLabel { color: green; }");
    else
        this->statusLabel->setStyleSheet("QLabel { color: black; }");
}
#endif

//main initialize method being called during the form creation.
//it loads all data from the database as well as setting up the form class variables and lists
void formMain::Initialize() {
#ifndef PYTHON_WRAPPER
    this->activeStatement = NULL;
    this->selectedStatement = NULL;
    this->activeLine = -1;
    this->sourceFileLineOffset = 0;
    DB_ID_ROLE = 1000;
    this->currentState = -1;
    this->currentStateId = -1;
    this->callStateSeen = false;
    this->bugCounter = 0;
    
    this->simulationFinished = false;     
    
    this->isWorkingOnBreakPoints = false;
    this->lastStateIdObserved = -1;
    
    this->updateGUI = true;
    
    cycle_counter = 0;
    dummy_cycle_counter = 0;
#endif
        
    InitializeAlteraFPPaths();
#ifndef PYTHON_WRAPPER
    InitializeData();
    fillSourceCodeTab();
#endif
    
#ifndef PYTHON_WRAPPER
    //setting main function as the starting function
    for (int i = 0 ; i < functions.size(); i++) {
        if (functions[i]->name == "main")
        {
            this->currentFunction = functions[i]->id;
            while(!this->callStack.empty())
                this->callStack.pop();
            this->callStack.push(functions[i]);
            break;            
        }
    }
    this->stepInto = false;
    this->stepIntoRadioButtonSelected = false;
    this->simulationFinished = false; 
    this->functionFinished = false;
    
    this->stepInLabel = new QLabel(this);
    this->stepInLabel->setText("Step-In mode");
    this->stepInLabel->setVisible(false);
#endif
        
#ifndef PYTHON_WRAPPER
    writeOnChipDebugInfoOnFile = true;//this has to be true always... no database support for onchip signals anymore because of the overhead
    //signalSelectionMode = AUTO_MODE_SIGNAL_SELECTION;
#endif
    
#ifndef PYTHON_WRAPPER
    //setting the mode as step-in in case of ONCHIP_VS_TIMING_SIM
    if (mode == ONCHIP_VS_TIMING_SIM)
      this->stepInto = true;
#endif
    
#ifndef PYTHON_WRAPPER
    if (mode == GDB_BUG_DETECTION || mode == GDB_SYNC || mode == ONCHIP_VS_TIMING_SIM) {
        InitializeDiscrepancyLogTable();
    }
#endif
}

#ifndef PYTHON_WRAPPER
//loading all containers and objects from data base
void formMain::InitializeData() {
    DA->getAllFunctions();
    DA->getAllStates();
    DA->getAllIRInstructions();
    
    //loading hardwareInfo for each IR instruction
    for (int i = 0 ; i < IRInstructions.size(); i++)
        IRInstructions[i]->hardwareInfo = DA->getIRHardwareInfo(IRInstructions[i]->id);
    
    DA->getAllHLStatements();
    
    //setting IR instruction line numbers
    for (int i = 0; i < IRInstructions.size(); i++) {
        if (IRInstructions[i]->HLInstr_id != 0)
            IRInstructions[i]->lineNumber = HLIdsToStatements[IRInstructions[i]->HLInstr_id]->line_number;
    }
    
    //filling states to effective statements mapping... increasing speed!
    for (int i = 0 ; i < States.size(); i++) {
        statesToEffectiveStatements[States[i]->id] = DA->getEffectiveStatementsForState(States[i]->id);
    }        
    
    std::map<int, std::vector<HLStatement*> >::iterator it;
    for (it = lineNumToStatement.begin(); it != lineNumToStatement.end(); ++it) {
        std::vector<HLStatement*> statements = (*it).second;
        if (statements.size() == 1) {
            //statements[0]->end_column_number = statements[0]->start_column_number;
            statements[0]->end_column_number = 10000;
            statements[0]->start_column_number = 0;
            continue;
        }
        
        for (int i = 0 ; i < statements.size()-1; i++) {
            if (statements[i]->start_column_number == 1)
                statements[i]->start_column_number = 0;
        }
        
        for (int i = 0 ; i < statements.size()-1; i++) {
            statements[i]->end_column_number = statements[i+1]->start_column_number - 1;
        }
        statements[statements.size() - 1]->end_column_number = 10000;//this should be the last col in the line! I don't have the value for now so I put some maximum number!
    }
    
    //loading all variableType and variable objects
    DA->getAllVariableTypes();
    DA->getAllVariables();
    
    
    //mapping functions to variables...
    for (int i = 0; i < Variables.size(); i++) {
        Variable* var = Variables[i];
        if (var->isGlobal)
            globalVariables.push_back(var);
        else
            functionsToVariables[IdsToFunctions[var->functionId]].push_back(var);
    }
    
    //loading variables initial values (if exists)
    for (int i = 0 ; i < Variables.size(); i++) {
        std::vector<std::string> data = DA->getVariableData(Variables[i]->id);        
        for (int d = 0; d < data.size(); d++) {
            Variables[i]->setHWInitialValue(d, data[d]);
            Variables[i]->updateHWPlainValues(-1, d);
        }        
    }
    
    //loading all signals
    DA->getAllHWSignals();
    for (int i = 0 ; i < IRInstructions.size(); i++)
        IRInstructions[i]->signalList = DA->getIRSignals(IRInstructions[i]->id);
    for (int i = 0 ; i < Signals.size(); i++)
        Signals[i]->states = DA->getSignalStates(Signals[i]->id);
    
    //loading all variables that are changing in each FSM state
    for (int i = 0; i < States.size(); i++) {
        States[i]->updatingVariables = DA->getStateUpdatingVariables(States[i]->id);
    }
}

void formMain::InitializeDiscrepancyLogTable() {
    this->widget.plainTableEditLog->clear();
    this->widget.plainTableEditLog->setColumnCount(4);
    this->widget.plainTableEditLog->setColumnWidth(0, 220);
    this->widget.plainTableEditLog->setColumnWidth(1, 220);
    this->widget.plainTableEditLog->setColumnWidth(2, 220);
    this->widget.plainTableEditLog->setColumnWidth(3, 220);
    //this->widget.plainTableEditLog->setColumnWidth(4, 100);
    this->widget.plainTableEditLog->setRowCount(0);
    this->widget.plainTableEditLog->setHorizontalHeaderItem(0, new QTableWidgetItem("Variable Name"));
    this->widget.plainTableEditLog->setHorizontalHeaderItem(1, new QTableWidgetItem("SW Value"));
    this->widget.plainTableEditLog->setHorizontalHeaderItem(2, new QTableWidgetItem("HW Value"));
    this->widget.plainTableEditLog->setHorizontalHeaderItem(3, new QTableWidgetItem("SW Line"));
    //this->widget.plainTableEditLog->setHorizontalHeaderItem(4, new QTableWidgetItem("HW Line"));    
}

void formMain::setMainReturnVal(std::string msg) {
    std::string trimmedStr = trimMessage(msg);
    if (trimmedStr.size() == 0)
        return;
    HWSignal* returnValSig = getMainReturnValSignal();
    returnValSig->setValue(trimmedStr, getCurrentStateObj()->id);
}

void formMain::setCurrentState(std::string msg) {    
    std::string trimmedStr = trimMessage(msg);
    if (trimmedStr.size() == 0)
    {
        currentState = -1;
        currentStateId = -1;
        IdsToFunctions[currentFunction]->currentState = currentState;
        IdsToFunctions[currentFunction]->currentStateId = currentStateId;
        return;
    }
    currentState = atoi(trimmedStr.c_str());
    State* st = getCurrentStateObj();
    if (st != NULL)
        observedStates.push_back(st);
    std::string stateName = DA->getStateNameByNumberAndBelongingFunctionId(currentState, currentFunction, currentStateId);
    IdsToFunctions[currentFunction]->currentState = currentState;
    IdsToFunctions[currentFunction]->currentStateId = currentStateId;
    if (IdsToStates[currentStateId]->isCallState()) {
        //currentFunction = IdsToStates[currentStateId]->calledFunction->id;
        tempCallingFunction = IdsToStates[currentStateId]->calledFunction->id;
        callStateSeen = true;
        //callStateLineNumber = DA->getCallStateLineNumber(currentStateId);
        callStateHLStatement = DA->getCallStateHLStatement(currentStateId);
        //std::cout << "CALL STATE SEEEN" << std::endl;
        //callStack.push(IdsToFunctions[currentFunction]);
    }
    QString str(stateName.c_str());
    this->widget.labelCurrentState->setText(str);
}

void formMain::processBatchExamineCommandResults(std::vector<std::pair<HWSignal*, int> >& watchSignals, std::string result) {
    int stateId = getCurrentStateObj()->id;
    std::vector<std::string> splits = split(result, ' ');
    for (int i = 0 ; i < splits.size(); i++)
    {
        std::string trimmedStr = trimToHex(splits[i]);
        if (trimmedStr == "")
            trimmedStr = "X";
        watchSignals[i].first->setValue(trimmedStr, stateId);
    }
    fillWatchTable(watchSignals);
}

Variable* formMain::getVariableByTagNum(int tagNum) {
    for (int i = 0 ; i < Variables.size(); i++) {
        if (Variables[i]->tagNum == tagNum)
            return Variables[i];
    }
    return NULL;
}

bool formMain::isInsideLegUpSpecificFunctions() {
    Function* f = IdsToFunctions[currentFunction];
    if (f->name.find("legup_") != std::string::npos)
        return true;
    return false;
}

std::string formMain::getMainModuleVPath() {
    std::string path;
    if (mode == ONCHIP)
        path = "top:top_inst|main:main_inst|";
    else
        path = "/main_tb/top_inst/main_inst/";
    return path;
}

std::string formMain::getTopModuleVPath() {
    std::string path;
    if (mode == ONCHIP)
        path = "top:top_inst|";
    else
        path = "/main_tb/top_inst/";
    return path;
}

std::string formMain::getCurrentFunctionVPathForOnChip() {
    std::string path;
    
    path = "top:top_inst|main:main_inst|";
    
    std::stack<Function*> callStackCopy = callStack;
    std::string t = "";
    while(callStackCopy.size() != 1) {
        Function *func = callStackCopy.top();
        t = func->name + ":" + func->name + "_inst|" + t;        
        callStackCopy.pop();
    }
    return (path + t);
}

std::string formMain::getCurrentFunctionVPath() {
    std::string path;
    if (mode == ONCHIP)
        path = "top:top_inst|main:main_inst|";
    else
        path = "/main_tb/top_inst/main_inst/";
    std::stack<Function *> callStackCopy = callStack;
    std::string t = "";
    while (callStackCopy.size() != 1) {
        Function *func = callStackCopy.top();
        if (mode == ONCHIP)
            t = func->name + ":" + func->name + "|" + t;
        // t = func->name + ":" + func->name + "_inst|" + t;
        else
            t = func->name + "/" + t;
        // t =  func->name + "_inst/" + t;
        callStackCopy.pop();
    }
    return (path + t);
}
    
void formMain::batchExamineForVariables(State* state){
    if (state->updatingVariables.size() <= 0)
        return;
    
    std::vector<std::string> signalNames;
    //std::string command = "examine -unsigned ";
    std::string command = "examine -hex ";        
    
    for (int i = 0; i < state->updatingVariables.size(); i++) {
        
        std::string port = state->updatingVariables[i]->port;
        std::string valueSignal = getTopModuleVPath() + "memory_controller_in_" + port;
        if (mode == ONCHIP)
            signalNames.push_back(valueSignal);
        else
            command += "-value " + valueSignal + " ";
        
        //if there is any variable, then it is const...
        //if not, should examine...
        if (state->updatingVariables[i]->signal->variable == NULL) {
            std::string addressSignal = getCurrentFunctionVPath() + state->updatingVariables[i]->signal->name;
            if (mode == ONCHIP)
                signalNames.push_back(addressSignal);
            else
                command += "-value " + addressSignal + " ";            
        }
    }
        
    std::string result;
        
    if (mode == ONCHIP)
        result = dbgEngine->batchExamine(signalNames);
    else
        result = tcpc.sendMessage(command);
    //std::cout << "result of batch examine command for variables: " << result << std::endl;
    processBatchExamineCommandResultsForVariables(state, result);
}

void formMain::processBatchExamineCommandResultsForVariables(State* state, std::string result) {
    int stateId = getCurrentStateObj()->id;
        
    std::vector<std::string> splits = split(result, ' ');
    int splitIdx = 0;    
    
    for (int i = 0; i < state->updatingVariables.size(); i++) {                
        
        if (state->updatingVariables[i]->signal->variable == NULL) {
            //int value = atoi(splits[splitIdx].c_str());
            //TODO: potentially there is a bug here... the conversion should be hex to long long not to int...
            //int value = hexToInt(trimToHex(splits[splitIdx]));
            std::string hexValue = trimToHex(splits[splitIdx]);
            splitIdx++;
            //int address = atoi(splits[splitIdx].c_str());
            int address = hexToInt(trimToHex(splits[splitIdx]));
            //unsigned int address = hexToUnsignedInt(trimToHex(splits[splitIdx]));
            splitIdx++;
            int tag = address >> 23;    
            int byteIndex = (address & ((int)(pow(2, 23)) - 1));
            Variable* variable = getVariableByTagNum(tag);
            if (variable == NULL)
                continue;
            int index = variable->getVarIndexByByteIndex(byteIndex);            
            int lineNumber = state->updatingVariables[i]->lineNumber;
            bool isFunctionCallLine = DA->isLineAFunctionCall(lineNumber);
            bool insideLegUpSpecificFunctions = isInsideLegUpSpecificFunctions();
            //TODO
            if (insideLegUpSpecificFunctions && mode == GDB_SYNC)
                lineNumber = gdbWrapper->currentGDBLine;
            //variable->setHWValue(index, hexValue, lineNumber, isFunctionCallLine);            
            variable->setHWValueWithHex(index, hexValue, lineNumber, isFunctionCallLine);
            if (mode == GDB_BUG_DETECTION)
                variable->updateHWPlainValues(lineNumber, index);
            //variable->setHWValueLine(index, state->updatingVariables[i]->lineNumber);
            //std::cout << "byte index: " << byteIndex << " index: " << index << std::endl;
            //std::cout << variable->name << "[" << index << "] = " << value << std::endl;
        } else {
            //int value = atoi(splits[splitIdx].c_str());
            std::string value = trimToHex(splits[splitIdx]);
            splitIdx++;
            int offset = state->updatingVariables[i]->offset;
            int lineNumber = state->updatingVariables[i]->lineNumber;
            int index = state->updatingVariables[i]->signal->variable->getVarIndexByByteIndex(offset);
            bool isFunctionCallLine = DA->isLineAFunctionCall(lineNumber);
            bool insideLegUpSpecificFunctions = isInsideLegUpSpecificFunctions();
            if (insideLegUpSpecificFunctions && mode == GDB_SYNC)
                lineNumber = gdbWrapper->currentGDBLine;
            if (state->updatingVariables[i]->signal->variable == NULL)
                continue;                        
            state->updatingVariables[i]->signal->variable->setHWValueWithHex(index, value, lineNumber,isFunctionCallLine);
            if (mode == GDB_BUG_DETECTION)
                state->updatingVariables[i]->signal->variable->updateHWPlainValues(lineNumber, index);
            //state->updatingVariables[i]->signal->variable->setHWValueLine(index, lineNumber);
            //std::cout << "byte index: " << offset << " index: " << index << std::endl;
            //std::cout << state->updatingVariables[i]->signal->variable->name << "[" << index << "] = " << value << std::endl;
        }
    }
}

void formMain::batchExamine(std::vector<std::pair<HWSignal*, int> >& watchSignals) {    
    if (watchSignals.empty())
        return;    
    //std::string command = "examine -unsigned ";
    std::string command = "examine -hex ";
    std::vector<std::string> signalNames;
    for (int i = 0 ; i < watchSignals.size(); i++){
        std::string path = "";
        //if (watchSignals[i].first->name.find("memory_controller_") != std::string::npos)
        if (watchSignals[i].first->name.find("memory_controller_") == 0)
            path = getTopModuleVPath();
        else
            path = getCurrentFunctionVPath();
        std::string sigName = path + watchSignals[i].first->name;
        if (mode == ONCHIP){
            signalNames.push_back(sigName);
        }
        else
            command += "-value " + sigName + " ";
    }
    std::string result;
    if (mode == ONCHIP)
        result = dbgEngine->batchExamine(signalNames);
    else
        result = tcpc.sendMessage(command);
    //std::cout << "result of batch examine command: " << result << std::endl;
    processBatchExamineCommandResults(watchSignals, result);
}

bool formMain::isCurrentFunctionFinished() {
    std::string command = "examine -unsigned ";
    std::string checkFinish = getCurrentFunctionVPath() + "finish";
    command += checkFinish;
    std::string result;
    if (mode == ONCHIP)
        result = dbgEngine->examine(checkFinish);
    else 
        result = tcpc.sendMessage(command);
    std::string msg = trimMessage(result);
    if (msg.size() == 0)
        return false;    
    return atoi(msg.c_str());
}

bool formMain::runForOneCycle(bool checkFinishSignal) {
    int isReturnState = false;
    cycle_counter++;
    /*if ((cycle_counter % 1000) == 0)
        std::cout << "HW Cycle: " << cycle_counter << std::endl;*/
    std::string result;
    if (mode == ONCHIP)
        dbgEngine->runForOneCycle();
    else {
        std::string runCommand = "run 20ns";
        result = tcpc.sendMessage(runCommand);
    }
    
    if (mode == ONCHIP) {
        std::vector<std::string> signalNames;
        signalNames.push_back(getCurrentFunctionVPath() + "cur_state");
        signalNames.push_back(getCurrentFunctionVPath() + "finish");
        //signalNames.push_back(getMainModuleVPath() + "return_val");
result = dbgEngine->batchExamine(signalNames);
    }
    else {
        std::string examineCommand = "examine -unsigned -value ";
        examineCommand += getCurrentFunctionVPath();
        examineCommand += "cur_state ";
        examineCommand += "-value " + getCurrentFunctionVPath() + "finish ";
        //examineCommand += "-value " + getMainModuleVPath() + "return_val";
        result = tcpc.sendMessage(examineCommand);
    }
    
    std::string finishSigVal;
    
    //the only situation when the result is empty is when the modelsim is closed (design finished)
    //we're not able to query any more things so I'm setting finishSigVal as 1 here....    
    if (result != "") {
        std::vector<std::string> splits = split(result, ' ');
        setCurrentState(splits[0]);
        finishSigVal = trimMessage(splits[1]);
        //setMainReturnVal(splits[2]);
        
    } else {
        finishSigVal = "1";
    }
    
    if (finishSigVal == "1")
    {
        //do not set the isReturn for legup specific functions...
        if (IdsToFunctions[currentFunction]->name.find("memset") == std::string::npos &&
                IdsToFunctions[currentFunction]->name.find("memmove") == std::string::npos &&
                IdsToFunctions[currentFunction]->name.find("memcpy") == std::string::npos) {
            isReturnState = true;
            finishingFunction = IdsToFunctions[currentFunction];
        }
        //functionFinished = true;
                    
        //if the finish signal is for the main function, simulation is finished!
        if (IdsToFunctions[currentFunction]->name == "main")
            simulationFinished = true;
        else {
            ResetLocalVariablesLastSetValues();
            callStack.pop();
            currentFunction = callStack.top()->id;
            currentState = callStack.top()->currentState;
            currentStateId = callStack.top()->currentStateId;
        }
    }
    return isReturnState;        
}

//resetting the last set value field for the local variables in the current function
//this function is called *just* before returning from the function...
void formMain::ResetLocalVariablesLastSetValues() {
    std::map<Function*, std::vector<Variable*> >::iterator it;
    for (it = functionsToVariables.begin(); it != functionsToVariables.end(); ++it) {
        Function* f = (*it).first;
        if (f->id != currentFunction)
            continue;
        std::vector<Variable*> vars = (*it).second;
        for (int i = 0; i < vars.size(); i++)
            vars[i]->ResetLastSetValues();
        break;
    }
}

void formMain::dumpVariableValues(std::vector<std::pair<int, std::string> >& values, Variable* var, int index, std::string fileName) {
    variablesLog.open((workDir + fileName).c_str(), std::ios::app);
    variablesLog << IdsToFunctions[var->functionId]->name << " : " << var->name << "["  << IntToString(index) << "]" << std::endl;
    
    for (int v = 0; v < values.size(); v++) {        
        variablesLog << values[v].second << "(" << values[v].first << ")";
        if (v != (values.size() - 1))
            variablesLog << ",";
    }
    variablesLog << std::endl;
    
    variablesLog.flush();
    variablesLog.close();
}

std::vector<int> formMain::GetLineNumbers(std::vector<HLStatement*>& statements) {
    std::vector<int> lines;
    for (int i = 0; i < statements.size(); i++) {
        if (std::find(lines.begin(), lines.end(), statements[i]->line_number) == lines.end())
            lines.push_back(statements[i]->line_number);
    }
    return lines;
}

void formMain::HandleFunctionCallings() {
    //if the function is finished (in previous cycle) it should be popped out of callStack
    if (functionFinished) {
        functionFinished = false;
    }
    //first thing to do in stepping is to update current function if a call state is seen
    //I'm checking stepInto because function should be changed only the times that we want to step into the new function
    //times where we're at stepOver mode, callStateSeen becomes true but it should be ignored at the next move...
    if (callStateSeen && stepIntoRadioButtonSelected) {
        currentFunction = tempCallingFunction;
        callStack.push(IdsToFunctions[currentFunction]);
        callStateSeen = false;
    } else {
        currentFunction = callStack.top()->id;//current function is the id of callStack top function            
        callStateSeen = false;
    }
    if (currentState == -1) {
        while ((currentState != 0))
            runForOneCycle();
    }

    if (!stepIntoRadioButtonSelected) {
        this->widget.statusbar->removeWidget(stepInLabel);
        this->widget.statusbar->show();
    }
}

void formMain::setCurrentFunctionGDBMode() {
    //for now I'm assuming all functions are in one file.
    //functions are already sorted by their startLineNumber in DA level method...        
    
    //the last before the first function whose startLineNumber is greater than current gdb line is the current function!
    
    //first entry in functions list is ALWAYS global, thus not being considered here...
    for (int i = 1 ; i < functions.size(); i++) {
        if (functions[i]->startLineNumber > gdbWrapper->currentGDBLine) {
            currentFunctionGDBMode = functions[i-1]->id;
            return;
        }
    }
    //if currentFunction is not set yet, it is the last function in the list...
    currentFunctionGDBMode = functions.back()->id;
}

void formMain::plainTableEditLog_itemClicked(QTableWidgetItem* item) {
    int line = atoi(item->text().toStdString().c_str());
    if (item->column() == 3) {//SW cell
        this->widget.tabWidget->setCurrentIndex(2);
        HighlightBugInCCode(line);
        
    } else if (item->column() == 4) {//HW cell
        this->widget.tabWidget->setCurrentIndex(3);
    }
}

void formMain::HighlightBugInCCode(int highlightingLine) {    
    this->widget.textEditCSource->setText("");
    std::stringstream ss;
    int scrollIdx = 0;
    //std::string highlightedLine;
    for (int line = 0 ; line < sourceLines.size(); line++) {
        std::string sourceLine = sourceLines[line];
        
        if (line + 1 == highlightingLine) {
            ss << "<a name=\"scrollToMe\"> <SPAN style=\"BACKGROUND-COLOR: #fff000\">" << sourceLine << "<br>" << "</SPAN></a>";
            //highlightedLine = sourceLine;
            scrollIdx = line;
        }
        else
            ss << sourceLine << "<br>";
    }
    
    QString str(ss.str().c_str());
    this->widget.textEditCSource->setText(str);
    this->widget.textEditCSource->scrollToAnchor("scrollToMe");
}

void formMain::pushButtonSingleStepping_clicked() {
    switch (mode) {
        case GDB_SYNC:
            singleSteppingForGDBSync();
            break;
        case MODELSIM:
            DoStepping();
            break;
        case ONCHIP:
            DoStepping();            
            break;
        case GDB:
            singleSteppingForPureGDBMode();
            break;
        case GDB_BUG_DETECTION:
            singleSteppingForGDBBugDetection();
            break;
        case ONCHIP_VS_TIMING_SIM:
            DoOnChipVsTimingSim();
            break;
    }
    
    return;        
}

void formMain::fillWatchTable(std::vector<std::pair<HWSignal*, int> >& watchSignals) {
    std::vector<HWSignal*> signalsVector;
    this->widget.treeWidgetWatch->clear();
    this->widget.treeWidgetWatch->setColumnCount(1); //TODO: is it necessary or not    
    if (simulationFinished)
    {
        for (int i = 0 ; i < watchSignals.size(); i++)
        {
            std::string value = watchSignals[i].first->name + " : " + watchSignals[i].first->getValue();
            QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(value.c_str())));            
            this->widget.treeWidgetWatch->insertTopLevelItem(i, item);
        }        
    }
    else
    {
        std::vector<int> visibleIRs;
        for (int i = 0 ; i < watchSignals.size(); i++)
        {
            if (std::find(visibleIRs.begin(), visibleIRs.end(), watchSignals[i].second) == visibleIRs.end())
                visibleIRs.push_back(watchSignals[i].second);
            signalsVector.push_back(watchSignals[i].first);
        }
        
        for (int i = 0 ; i < visibleIRs.size(); i++)
        {
            IRInstruction* instr = activeIRs[visibleIRs[i]];
            QTreeWidgetItem* instrItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(instr->dump.c_str())));
            for (int s = 0 ; s < instr->signalList.size(); s++)
            {
                HWSignal* sig = instr->signalList[s];
                if (std::find(signalsVector.begin(), signalsVector.end(), sig) != signalsVector.end())
                {
                    std::string value = sig->name + " : " + sig->getValue();
                    QTreeWidgetItem* c = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(value.c_str())));
                    instrItem->addChild(c);
                }
            }            
            if (instrItem->childCount() != 0)
                this->widget.treeWidgetWatch->insertTopLevelItem(i, instrItem);
        }        
    }    
    this->widget.treeWidgetWatch->expandAll();
}

void formMain::textEditHighLevel_CursorPositionChanged() {
    QTextCursor textCur = this->widget.textEditHighLevel->textCursor();    
    
    int cursorPosition = textCur.position();
    std::string content = this->widget.textEditHighLevel->toPlainText().toStdString();
    int col = textCur.columnNumber();
    int line = 0;
    for (int i = 0 ; i < cursorPosition; i++)
    {                        
        if ((int)content[i] == 10)
            line++;
    }    
    
    std::map<int, HLStatement*>::iterator it;
    for (it = HLIdsToStatements.begin(); it != HLIdsToStatements.end(); ++it)
    {
        HLStatement* statement = (*it).second;
        if ((statement->line_number - 1) == line
                && statement->start_column_number <= col && statement->end_column_number >= col)
        {
            selectedStatement = statement;
            fillIRTextEdit();
        }
    }    
}

void formMain::fillIRTextEdit() {    
    this->widget.treeWidgetIR->clear();
    this->widget.listWidgetHardware->clear();
    
    activeIRs.clear();
    
    for (int i = 0 ; i < effectiveStatements.size(); i++)
    {
        HLStatement* effectiveStatement = effectiveStatements[i];
        for (int irIdx = 0 ; irIdx < effectiveStatement->IRs.size(); irIdx++)
        {
            activeIRs.push_back(effectiveStatement->IRs[irIdx]);
        }
    }
    
    /*if (activeStatement != NULL)
    {
        activeIRs = activeStatement->IRs;        
    }*/
    if (activeIRs.size() == 0)
    {        
        //TODO
        QTreeWidgetItem* stateNode = new QTreeWidgetItem((QTreeWidget*)0, QStringList(("No IR to show!")));
        this->widget.treeWidgetIR->insertTopLevelItem(0, stateNode);        
        return;
    }
    else
    {        
        std::map<int, std::vector<IRInstruction*> > stateNumToIRMapping;
        std::map<int, std::vector<IRInstruction*> >::iterator it;
        
        for (int i = 0 ; i < activeIRs.size(); i++)
        {
            IRInstruction* ir = activeIRs[i];
            for (int stateNum = ir->startState->number; stateNum <= ir->endState->number; stateNum++)
            {
                stateNumToIRMapping[stateNum].push_back(ir);
            }            
        }
        
        int idx = 0;
        for (it = stateNumToIRMapping.begin(); it != stateNumToIRMapping.end(); it++)
        {            
            int stateNum = (*it).first;
            std::string nodeText = "State : " + IntToString(stateNum);
            QTreeWidgetItem* stateNode = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(nodeText.c_str())));
            for (int i = 0 ; i < (*it).second.size(); i++)
            {
                std::string text = (*it).second[i]->dump;
                QTreeWidgetItem* instrNode = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(text.c_str())));
                bool isInActives = (activeStatement != NULL && std::find(activeStatement->IRs.begin(), activeStatement->IRs.end(), (*it).second[i]) != activeStatement->IRs.end());
                bool isInSelected = (selectedStatement != NULL && std::find(selectedStatement->IRs.begin(), selectedStatement->IRs.end(), (*it).second[i]) != selectedStatement->IRs.end());                
                if (isInActives && (*it).first == currentState)
                    instrNode->setBackgroundColor(0, Qt::yellow);
                else if (isInSelected && isInActives)
                    instrNode->setBackgroundColor(0, QColor(255, 255, 179));//kerem!
                else if (isInSelected && !isInActives)
                    instrNode->setBackgroundColor(0, Qt::green);
                instrNode->setData(0, DB_ID_ROLE, (*it).second[i]->id);
                instrNode->setData(0, Qt::CheckStateRole, QVariant());
                stateNode->addChild(instrNode);
            }
            /*if (stateNum == currentState)
                stateNode->setBackgroundColor(0, Qt::yellow);*/
            this->widget.treeWidgetIR->insertTopLevelItem(idx, stateNode);            
            idx++;
        }
        this->widget.treeWidgetIR->expandAll();        
    }        
}

void formMain::fillHardwareTextEdit(std::vector<std::string> items) {
    this->widget.listWidgetHardware->clear();    
    if (items.size() == 0)
    {        
        QListWidgetItem* item = new QListWidgetItem("No Hardware info to show!");
        this->widget.listWidgetHardware->insertItem(0, item);
        return;
    }
    else
    {        
        for (int i = 0 ; i < items.size(); i++)
        {
            std::string itemText = items[i];
            //itemText = processLine(itemText);
            QString str(itemText.c_str());            
            QListWidgetItem* item = new QListWidgetItem(str);   
            this->widget.listWidgetHardware->insertItem(i, item);
        }
    }
}

void formMain::textEditLineNumbers_CursorPositionChanged() {
    if (isWorkingOnBreakPoints)
        return;
    isWorkingOnBreakPoints = true;
    QTextCursor textCur = this->widget.textEditLineNumber->textCursor();
    
    int cursorPosition = textCur.position();
    std::string content = this->widget.textEditLineNumber->toPlainText().toStdString();
    int col = textCur.columnNumber();
    int line = 0;
    for (int i = 0 ; i < cursorPosition; i++)
    {                        
        if ((int)content[i] == 10)
            line++;
    }    
    
    HLStatement* closestButNotEqualStatement = NULL;
    int closestDistance = 1000;
    bool exactMatchFound = false;
    std::map<int, HLStatement*>::iterator it;
    for (it = HLIdsToStatements.begin(); it != HLIdsToStatements.end(); ++it)
    {
        HLStatement* statement = (*it).second;        
        if ((statement->line_number - 1) == line)
        {       
            std::vector<HLStatement*>::iterator it = std::find(breakPointStatements.begin(), breakPointStatements.end(), statement);
            if (it == breakPointStatements.end())
                breakPointStatements.push_back(statement);
            else
                breakPointStatements.erase(it);
            //selectedStatement = statement;
            exactMatchFound = true;            
            break;
        } else {
            int dist = abs(statement->line_number - 1 - line);
            if (dist < closestDistance) {
                closestDistance = dist;
                closestButNotEqualStatement = statement;
            }                
        }            
    }
    
    if (!exactMatchFound) {
        if (closestButNotEqualStatement != NULL && closestDistance < 3) {
                std::vector<HLStatement*>::iterator it = std::find(breakPointStatements.begin(), breakPointStatements.end(), closestButNotEqualStatement);
                if (it == breakPointStatements.end())
                    breakPointStatements.push_back(closestButNotEqualStatement);
                else
                    breakPointStatements.erase(it);
        }
    }
    
    fillLineNumbers();
    
    isWorkingOnBreakPoints = false;
}

void formMain::fillLineNumbers() {
    std::stringstream ss;
    std::string str;
    for (int i = 0 ; i < sourceLines.size(); i++)
    {        
        bool lineIsBreakPointed = false;
        for (int h = 0 ; h < breakPointStatements.size(); h++)
        {
            if (breakPointStatements[h]->line_number == (i+1))
            {
                lineIsBreakPointed = true;
                break;
            }
        }
        if (lineIsBreakPointed)
            ss << "*" << std::endl;
        else
            ss << (i+1) << std::endl;
    }
    int scrollValue = this->widget.textEditLineNumber->verticalScrollBar()->value();
    this->widget.textEditLineNumber->setText(QString(ss.str().c_str()));
    this->widget.textEditLineNumber->verticalScrollBar()->setValue(scrollValue);
}

bool formMain::shouldBeActive(int line, int col) {    
    if (activeStatement == NULL)
        return false;
    if (activeStatement->line_number == (line + 1))
        if (col >= activeStatement->start_column_number && col <= activeStatement->end_column_number)
            return true;
    return false;
}

bool formMain::shouldBeEffective(int line, int col) {
    for (int i = 0 ; i < effectiveStatements.size(); i++)
    {
        HLStatement* statement = effectiveStatements[i];
        if (statement->line_number == (line+1))
        if (col >= statement->start_column_number && col <= statement->end_column_number)
            return true;
    }
    return false;
}

void formMain::fillHighLevelCodeTextEditForGDBMode() {
    fillLineNumbers();
    this->widget.textEditHighLevel->setText("");
    std::stringstream ss;
    int scrollIdx = 0;
    //std::string highlightedLine;
    for (int line = 0 ; line < sourceLines.size(); line++) {
        std::string sourceLine = sourceLines[line];
        
        if (line + 1 == gdbWrapper->currentGDBLine) {
            ss << "<a name=\"scrollToMe\"> <SPAN style=\"BACKGROUND-COLOR: #fff000\">" << sourceLine << "<br>" << "</SPAN></a>";
            //highlightedLine = sourceLine;
            scrollIdx = line;
        }
        else
            ss << sourceLine << "<br>";
    }
    
    QString str(ss.str().c_str());
    this->widget.textEditHighLevel->setText(str);
    this->widget.textEditHighLevel->scrollToAnchor("scrollToMe");
}

void formMain::fillHighLevelCodeTextEdit() {    
    assert(mode != GDB_SYNC);
    
    fillLineNumbers();
    this->widget.textEditHighLevel->setText("");   
    
    std::string normalText = "";
    std::string effectiveText = "";
    std::string activeText = "";
    std::stringstream ss;
    for (int line = 0 ; line < sourceLines.size(); line++)
    {        
        bool normalFlag = true;
        bool effectiveFlag = false;
        bool activeFlag = false;
        std::string sourceLine = sourceLines[line];
        for (int col = 0; col < sourceLine.size(); col++)
        {
            char ch = sourceLine[col];
            bool changed = false;
            if (shouldBeActive(line, col))
            {
                changed = !activeFlag;
                activeFlag = true;
                normalFlag = false;
                effectiveFlag = false;
            } else if (shouldBeEffective(line, col))
            {
                changed = !effectiveFlag;
                activeFlag = false;
                normalFlag = false;
                effectiveFlag = true;             
            } else
            {
                changed = !normalFlag;
                activeFlag = false;
                normalFlag = true;
                effectiveFlag = false;                
            }
            if (changed)
            {
                if (normalText != "") {
                    ss << normalText;
                    normalText = "";
                } else if (effectiveText != "") {
                    ss << "<SPAN style=\"BACKGROUND-COLOR: #00ff00\">" << effectiveText << "</SPAN>";
                    effectiveText = "";
                } else if (activeText != "") {
                    ss << "<a name=\"scrollToMe\"><SPAN style=\"BACKGROUND-COLOR: #fff000\">" << activeText << "</SPAN></a>";
                    activeText = "";
                }
            }            
            
            std::stringstream s;
            s << ch;
            std::string str = s.str();
            normalText += normalFlag ? str : "";
            activeText += activeFlag ? str : "";
            effectiveText += effectiveFlag ? str : "";
        }
        
        if (normalText != "") {
            ss << normalText << "<br>" << std::endl;
            normalText = "";
        } else if (effectiveText != "") {
            ss << "<SPAN style=\"BACKGROUND-COLOR: #00ff00\">" << effectiveText << "</SPAN><br>" << std::endl;
            effectiveText = "";
        } else if (activeText != "") {
            ss << "<a name=\"scrollToMe\"><SPAN style=\"BACKGROUND-COLOR: #fff000\">" << activeText << "</SPAN><br></a>" << std::endl;
            activeText = "";
        }        
        //ss << std::endl;
    }
    if (normalText != "")
        ss << normalText << "<br>" << std::endl;
    else if (effectiveText != "")
        ss << "<SPAN style=\"BACKGROUND-COLOR: #00ff00\">" << effectiveText << "</SPAN><br>" << std::endl;
    else if (activeText != "")
        ss << "<a name=\"scrollToMe\"><SPAN style=\"BACKGROUND-COLOR: #fff000\">" << activeText << "</SPAN><br></a>" << std::endl;        
    
    QString str(ss.str().c_str());
    this->widget.textEditHighLevel->setText(str);
    
    this->widget.textEditHighLevel->scrollToAnchor("scrollToMe");
}

bool formMain::breakPointIsHitForGDB() {
    for (int i = 0 ; i < breakPointStatements.size(); i++)
    {
        if (gdbWrapper->currentGDBLine == breakPointStatements[i]->line_number - 1)
            return true;
    }
    return false;
}

bool formMain::breakPointIsHit() {
    for (int i = 0 ; i < breakPointStatements.size(); i++) {
        if (activeStatement == breakPointStatements[i])
            return true;
    }
    return false;
}

void formMain::fillSourceCodeTab() {
    sourceLines.clear();
    std::ifstream in;
    in.open((workDir + codeFilename).c_str());
    std::string line;
    while(in.good())
    {        
        std::string rawLine;
        getline(in, rawLine);
        line = processLine(rawLine);
        sourceLines.push_back(line);
    }
    in.close();
    
    updateView(false);
    //this->widget.labelStepIn->setVisible(false);
    
    if (mode == GDB_SYNC)
        fillHighLevelCodeTextEditForGDBMode();
    else
        fillHighLevelCodeTextEdit();
}

void formMain::updateHWWatchTable() {    
    std::vector<HWSignal*> signalsVector;
    std::vector<std::pair<HWSignal*, int> > watchList;//first: signal , second: related activeIR index
    for (int i = 0 ; i < activeIRs.size(); i++)
    {    
        bool is_load = activeIRs[i]->dump.find("load") != std::string::npos;//TODO: not a good check! at least I need to have type for each instruction!
        std::vector<HWSignal*> list = activeIRs[i]->signalList;
        for (int s = 0 ; s < list.size(); s++)
        {
            HWSignal* sig = list[s];
            bool shouldAdd = false; 

            if (std::find(signalsVector.begin(), signalsVector.end(), sig) != signalsVector.end())
                continue;

            if (is_load)
            {
                if (sig->name.find("memory_controller") != std::string::npos)
                    //shouldAdd = (activeIRs[i]->startState->number == currentState);
                    shouldAdd = (activeIRs[i]->startState->id == currentStateId);
                else
                    shouldAdd = (activeIRs[i]->endState->id == currentStateId);
            }
            else
                shouldAdd = (activeIRs[i]->endState->id == currentStateId);

            if (shouldAdd)
            {
                signalsVector.push_back(sig);
                watchList.push_back(std::make_pair(sig, i));
            }
        }
    }        
    batchExamine(watchList);
}

void formMain::updateSWVariablesTable() {
    //TODO: I should filter Variables...
    std::vector<Variable*> variablesToShow;    
    for (int i = 0 ; i < Variables.size(); i++)
    {
        if (Variables[i]->isGlobal)
            variablesToShow.push_back(Variables[i]);        
        else if (Variables[i]->functionId == currentFunction)
            variablesToShow.push_back(Variables[i]);            
    }
    fillSWVariablesTable(variablesToShow);
}

void formMain::fillSWVariablesTable(std::vector<Variable*>& watchVariables) {    
    this->widget.tableWidgetSWVariables->clear();
    this->widget.tableWidgetSWVariables->setColumnCount(2);
    this->widget.tableWidgetSWVariables->setColumnWidth(0, 100);
    this->widget.tableWidgetSWVariables->setColumnWidth(1, 100);
    this->widget.tableWidgetSWVariables->setRowCount(watchVariables.size());
    this->widget.tableWidgetSWVariables->setHorizontalHeaderItem(0, new QTableWidgetItem("Variable Name"));
    this->widget.tableWidgetSWVariables->setHorizontalHeaderItem(1, new QTableWidgetItem("Variable Value"));
    
    for (int i = 0 ; i < watchVariables.size(); i++)
    {
        Variable* variable = watchVariables[i];
        std::string var = variable->name;
        if (variable->isArrayType)
        {            
            if (variablesToUpdateInfo.find(variable) != variablesToUpdateInfo.end())
            {
                VariableUpdateInfo* updateInfo = variablesToUpdateInfo[variable];                
            }
        }
        QString varName(var.c_str());        
        QString varValue(variable->getLastSWValue().c_str());
        
        if (varName == "")
            varName = "N/A";
        if (varValue == "")
            varValue = "N/A";
        QTableWidgetItem* itemVarName = new QTableWidgetItem(varName);
        QTableWidgetItem* itemVarValue = new QTableWidgetItem(varValue);
        this->widget.tableWidgetSWVariables->setItem(i, 0, itemVarName);
        this->widget.tableWidgetSWVariables->setItem(i, 1, itemVarValue);
    }
}

void formMain::updateHWVariablesTable() {
    //TODO: I should filter Variables...    
    std::vector<Variable*> variablesToShow;
    
    int currentFunctionId;
    if (mode == GDB)
        currentFunctionId = currentFunctionGDBMode;
    else
        currentFunctionId = currentFunction;
    
    for (int i = 0 ; i < Variables.size(); i++)
    {
        if (Variables[i]->name[0] == '%')
            continue;
        if (Variables[i]->isGlobal)
            variablesToShow.push_back(Variables[i]);
        else if (Variables[i]->functionId == currentFunctionId) {
            variablesToShow.push_back(Variables[i]);
        }
    }    
    fillHWVariablesTable(variablesToShow);
}

bool formMain::isPrimitive(Type type) {
    switch(type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case PRIMITIVE_DOUBLE:
        case POINTER:
            return true;
        default:
            return false;
    }
}

//this method returns the typed value for the given input value
//mostly used for float/double type values
std::string formMain::getTypedValueString(std::string rawValue, Type type) {
    switch(type) {
        case PRIMITIVE_INT:
        case POINTER:
            return rawValue;
        case PRIMITIVE_FLOAT: {            
            //return rawValue;            
            int decimal = atoi(rawValue.c_str());
            int n = decimal;
            std::string binary;
            for (int i = 0; i < sizeof(int)*8; i++) {
                if (n & 1)
                    binary = "1" + binary;
                else
                    binary = "0" + binary;
                n = n >> 1; 
            }
            //return binary;
            return ftostr(getFloat32(binary));
        }
        case PRIMITIVE_DOUBLE: {
            return rawValue;
        }
        default:
            return "N/A";
    }
}

std::string formMain::getVariableHWValueStr(int index, Variable* variable) {
    if (mode == GDB_SYNC)
        return variable->getHWValueStr(index, gdbWrapper->previousGDBLine);
    else {
        if (activeStatement != NULL)
            return variable->getHWValueStr(index, activeStatement->line_number);
        else
            return variable->getHWValueStr(index, -100);//some line that I'm sure that it doesn't hit, so the initial value will be returned
    }
}

void formMain::fillHWVariableNode(Variable* variable, VariableType* varType, QTreeWidgetItem* treeItem, std::string leafText) {
    switch(varType->type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case PRIMITIVE_DOUBLE:
        case POINTER: {
            std::string valueText = getVariableHWValueStr(HWVariableTableValueIndex, variable);
            std::string text =  leafText + " : " + valueText;
            HWVariableTableValueIndex++;
            if (updateGUI) {
                QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(text.c_str())));
                if (treeItem == NULL)
                    this->widget.treeWidgetHWValues->insertTopLevelItem(this->widget.treeWidgetHWValues->topLevelItemCount(), item);
                else
                    treeItem->addChild(item);
            }
            break;
        }
        case ARRAY: {
            if (isPrimitive(varType->elementTypes[0]->type)) {
                std::string parentText = leafText;
                QTreeWidgetItem* parentItem;
                if (updateGUI) {
                    parentItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(parentText.c_str())));
                    if (treeItem == NULL)
                        this->widget.treeWidgetHWValues->insertTopLevelItem(this->widget.treeWidgetHWValues->topLevelItemCount(), parentItem);
                    else
                        treeItem->addChild(parentItem);
                }
                
                for (int i = 0 ; i < varType->numElements; i++) {
                    std::string text = leafText + "[" + IntToString(i) + "] : " + getVariableHWValueStr(HWVariableTableValueIndex, variable);
                    HWVariableTableValueIndex++;
                    if (updateGUI) {
                        QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(text.c_str())));
                        parentItem->addChild(item);
                    }
                }
            } else {
                std::string text = leafText;
                QTreeWidgetItem* item;
                if (updateGUI) {
                    item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(text.c_str())));
                    if (treeItem == NULL)
                        this->widget.treeWidgetHWValues->insertTopLevelItem(this->widget.treeWidgetHWValues->topLevelItemCount(), item);
                    else
                        treeItem->addChild(item);
                }
                
                for (int i = 0 ; i < varType->numElements; i++) {
                    fillHWVariableNode(variable, varType->elementTypes[0], item, leafText + "[" + IntToString(i) + "]");
                }
            }
            break;
        }
        case STRUCT: {
            std::string text = leafText;
            QTreeWidgetItem* item;
            if (updateGUI) {
                item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(text.c_str())));
                if (treeItem == NULL)
                    this->widget.treeWidgetHWValues->insertTopLevelItem(this->widget.treeWidgetHWValues->topLevelItemCount(), item);
                else
                    treeItem->addChild(item);
            }
            for (int i = 0; i < varType->numElements; i++) {
                fillHWVariableNode(variable, varType->elementTypes[i], item, leafText + ".Slot" + IntToString(i));
            }
            break;
        }
    }
}

void formMain::fillHWVariablesTable(std::vector<Variable*>& watchVariables) {
    this->widget.treeWidgetHWValues->clear();
    this->widget.treeWidgetHWValues->setColumnCount(1);
    
    for (int i = 0 ; i < watchVariables.size(); i++) {
        Variable* var = watchVariables[i];        
        HWVariableTableValueIndex = 0;
        fillHWVariableNode(var, var->type, NULL, var->name);                
    }    
}

void formMain::fillBreakPointsList() {
}

void formMain::handleHighLevelCodeText() {    
    if (mode == GDB_SYNC || mode == GDB)
        fillHighLevelCodeTextEditForGDBMode();
    else
        fillHighLevelCodeTextEdit();
}

void formMain::updateView(bool updateWatchTable) {    
    if (simulationFinished)
        return;
    
    if (updateGUI) {
        fillBreakPointsList();
        handleHighLevelCodeText();
        fillIRTextEdit();
    }
    
    //this->widget.treeWidgetWatch->clear();
    if (updateWatchTable)
    {
        if (lastStateIdObserved != currentStateId || (mode != GDB_BUG_DETECTION && lastActiveStatement != activeStatement)) {
            lastStateIdObserved = currentStateId;
            lastActiveStatement = activeStatement;
            updateVariableValues();
            if (updateGUI)
                updateHWWatchTable();
            updateHWVariablesTable();
        }
    }    
}

HWSignal* formMain::getMainReturnValSignal() {        
    for (int i = 0 ; i < Signals.size(); i++) {
        if (Signals[i]->name == "return_val")
            return Signals[i];            
    }    
}

//this function finds the current state object regarding the current_state and current_function values
State* formMain::getCurrentStateObj() {    
    for (int i = 0 ; i < States.size(); i++)
    {
        if (States[i]->number == currentState && States[i]->belongingFunction->id == currentFunction)
            return States[i];
    }
    return NULL;
}

void formMain::updateVariableValues() {
    State* curState = getCurrentStateObj();
    //curState normally shouldn't be null... but I just check for preventing the code for potential exceptions...
    if (curState != NULL)
        batchExamineForVariables(curState);
}

void formMain::createSTPFile(int triggerValue, std::vector<int>& selectedSignalIds, std::map<std::string, int>& selectedExtraSignals, std::map<std::string, int>& byDefaultAddedSignals) {    
    std::vector<HWSignal*> onChipWatchSignals;
    for (int i = 0 ; i < selectedSignalIds.size(); i++)
        onChipWatchSignals.push_back(IdsToSignals[selectedSignalIds[i]]);
    
    //I should also pass selectedExtraSignals list to the STPCreator and add those separately to the file...    
    STPCreator* stpCreator = new STPCreator(onChipWatchSignals, selectedExtraSignals, byDefaultAddedSignals, signalSelectionMode);
    
    if (mode == ONCHIP_VS_TIMING_SIM) {
        stpCreator->generateSTPForTimingSim(triggerValue);
        
        std::map<std::string, std::vector<FPGANode*> >::iterator it;
        for (it = stpCreator->signalsToNodesList.begin(); it != stpCreator->signalsToNodesList.end(); ++it) {            
            if (onChipSignalNamesToFullNames.find((*it).first) == onChipSignalNamesToFullNames.end())
                for (int i = 0; i < (*it).second.size(); i++)
                    onChipSignalNamesToFullNames[(*it).first].push_back((*it).second[i]->fullName);
        }        
    }
    else if (mode == ONCHIP){
        stpCreator->generateSTP(triggerValue);
        
        std::map<std::string, std::vector<FPGANode*> >::iterator it;
        for (it = stpCreator->signalsToNodesList.begin(); it != stpCreator->signalsToNodesList.end(); ++it) {            
            if (onChipSignalNamesToFullNames.find((*it).first) == onChipSignalNamesToFullNames.end())
                for (int i = 0; i < (*it).second.size(); i++)
                    onChipSignalNamesToFullNames[(*it).first].push_back((*it).second[i]->fullName);
        }
    }
    
    delete stpCreator;
}

bool formMain::readCSVFile() {
    CSVReader* csvReader = new CSVReader();
    bool isFinished = csvReader->readCSV();
        
    std::vector<std::string> counterValues = ((*(csvReader->signalsToValues.find("top:top_inst|counter"))).second);    
    std::map<std::string, std::vector<std::string> >::iterator it;
    bool finishSignal = false;
    for (it = csvReader->signalsToValues.begin(); it != csvReader->signalsToValues.end(); ++it) {
        if ((*it).first.find("top:top_inst|counter") != std::string::npos)
            continue;
        if ((*it).first.find("top:top_inst|finish") != std::string::npos)
            finishSignal = true;
        else
            finishSignal = false;        
                
        for (int i = 0 ; i < counterValues.size(); i++) {
            std::string counterStr = counterValues[i];
            int counterValue = BinaryToDecimal(counterStr);
            std::string signalValue = (*it).second[i];            
            if (finishSignal)
            {
                int signalInt = BinaryToDecimal(signalValue);
                if (signalInt != 0)
                    isFinished = true;
            }
            onChipValues[(*it).first][counterValue] = signalValue;            
        }
    }
    
    delete csvReader;
    return isFinished;
}

void formMain::RunCompleteSimulation() {
    assert (mode == ONCHIP_VS_TIMING_SIM);
    
    do {        
        DoSteppingForCompleteSimulation();        
        
        simulationValues[cycle_counter - dummy_cycle_counter] = std::make_pair(currentFunction, IdsToStates[currentStateId]->number);
        
        std::string path = getCurrentFunctionVPathForOnChip();
        simulationCycleToCallStackPath[cycle_counter - dummy_cycle_counter] = path;
    } while(!simulationFinished);        
}

void formMain::loadReferenceSimulationData() {
    std::ifstream in;
    in.open((workDir + referenceSimulationDataFilename).c_str());
    std::string line;
    if (in.good())
        getline(in, line);//skipping the first line (headers)
    
    while(in.good())
    {
        std::string rawLine;
        getline(in, rawLine);
        if (rawLine == "")
            continue;
        std::vector<std::string> tokens = split(rawLine, '|');        
        simulationValues[atoi(tokens[0].c_str())] = std::make_pair(atoi(tokens[1].c_str()), atoi(tokens[2].c_str()));
    }
    in.close(); 
    
    in.open((workDir + referenceSimulationCyclesToPathsFileName).c_str());
    if (in.good())
        getline(in, line);
    
    while (in.good()) {
        std::string rawLine;
        getline(in, rawLine);
        if (rawLine == "")
            continue;
        std::vector<std::string> tokens = split(rawLine, ',');
        simulationCycleToCallStackPath[atoi(tokens[0].c_str())] = tokens[1];
    }
    in.close();
    
}

// Event handler functions

void formMain::actionView_IR_Instructions_changed() {
    this->widget.treeWidgetIR->setVisible(!this->widget.treeWidgetIR->isVisible());
    this->widget.labelIR->setVisible(!this->widget.labelIR->isVisible());
}

void formMain::actionView_HW_Info_changed() {
    this->widget.listWidgetHardware->setVisible(!this->widget.listWidgetHardware->isVisible());
    this->widget.labelHardware->setVisible(!this->widget.labelHardware->isVisible());
}

void formMain::actionView_Watch_changed() {
    this->widget.treeWidgetWatch->setVisible(!this->widget.treeWidgetWatch->isVisible());
    this->widget.treeWidgetHWValues->setVisible(!this->widget.treeWidgetHWValues->isVisible());
    this->widget.tableWidgetSWVariables->setVisible(!this->widget.tableWidgetSWVariables->isVisible());
    this->widget.labelWatch->setVisible(!this->widget.labelWatch->isVisible());
    this->widget.labelWatchSWVariables->setVisible(!this->widget.labelWatchSWVariables->isVisible());
    this->widget.labelWatch_3->setVisible(!this->widget.labelWatch_3->isVisible());
}

//obsolete right now
void formMain::actionReBuildCode_clicked() {
    std::string newSource = this->widget.textEditHighLevel->toPlainText().toStdString();
    std::vector<std::string> lines = split(newSource, '\n');
    std::ofstream out((workDir + rawSourceFilename).c_str());
    if (out.is_open()) {    
        for (int i = 1; i < lines.size(); i++) {
            out << lines[i] << std::endl;
        }
        out.flush();
        out.close();
    }
    //executing LegUp make script    
    std::string xtermCommand = "xterm";
    int result = std::system(xtermCommand.c_str());
    
    //should re-initialize the code again...
    //Initialize();   
}

void formMain::actionRunReferenceSim_clicked() {
    
    RunCompleteSimulation();
    
    std::ofstream out;
    out.open((workDir + referenceSimulationDataFilename).c_str());
    out << "Cycle|FunctionId|StateNumber" << std::endl;
    
    std::map<int, std::pair<int, int> >::iterator it;
    for (it = simulationValues.begin(); it != simulationValues.end(); ++it) {
        out  << (*it).first << "|" <<
                (*it).second.first << "|" <<
                (*it).second.second << std::endl;
    }
    
    out.flush();
    out.close();
    
    out.open((workDir + referenceSimulationCyclesToPathsFileName).c_str());
    out << "Cycle,Path" << std::endl;
    
    std::map<int, std::string>::iterator it2;
    for (it2 = simulationCycleToCallStackPath.begin(); it2 != simulationCycleToCallStackPath.end(); ++it2) {
        out << (*it2).first << "," << (*it2).second << std::endl;
    }
    
    out.flush();
    out.close();
}

void formMain::actionRunOnChip_clicked() {
    
    assert (mode == ONCHIP);    
    
    writeOnChipDebugInfoOnFile=true;
    //TODO: I'm now working with files not DB anymore, so the debug...dat file should be removed here...
        
    bool isFinished = false;
    int triggerValue = 0;
    bool firstTime = true;
    onChipDebugWindowSize = 120;
    
    std::vector<int> selectedSignalIds;
    std::map<std::string, int> selectedExtraSignals;
    std::map<std::string, int> byDefaultAddedSignals;
    
    selectSignalsDialog = new DialogSelectSignals();
    if (selectSignalsDialog->exec() == QDialog::Accepted) {
        selectedSignalIds = selectSignalsDialog->selectedSignalIds;
        selectedExtraSignals = selectSignalsDialog->selectedExtraSignals;
        byDefaultAddedSignals = selectSignalsDialog->byDefaultAddedSignals;
    }
    else {
        delete selectSignalsDialog;
        return;
    }
    delete selectSignalsDialog; 
    
    if (signalSelectionMode == AUTO_MODE_SIGNAL_SELECTION)
        loadStatesToCyclesFile();
    
    do
    {
        std::cout << "trigger value: " << triggerValue << std::endl;
        triggerValue += onChipDebugWindowSize;
        createSTPFile(triggerValue, selectedSignalIds, selectedExtraSignals, byDefaultAddedSignals);
        std::cout << "STP created" << std::endl;

        //should run the design with stp file in here...
        if (firstTime) {
            char *argv[]={NULL};
            const char *path = dbgMakeFilePath.c_str();
            int result = system_alternative(path, argv);
            firstTime = false;
            std::cout << "after run in dbg make!" << std::endl;
        }
        else
        {            
            char *argv[]={NULL};
            const char *path = increamentalDebugMakeFilePath.c_str();            
            int result = system_alternative(path, argv); 
            std::cout << "after run in incremental!" << std::endl;
        }
                
        isFinished = readCSVFile();        
        
    } while (!isFinished);
    
    std::cout << "running finished." << std::endl;
    
    if (writeOnChipDebugInfoOnFile) {
        std::ofstream out;
        out.open((workDir + onChipDebugInfoFileAddress).c_str());
        std::map<std::string, std::map<int, std::string> >::iterator it;
        for (it = onChipValues.begin(); it != onChipValues.end(); ++it) {
            std::string signalName = (*it).first;
            out << "*" << std::endl;
            out << signalName << std::endl;
            std::map<int, std::string>::iterator valIt;
            for (valIt = (*it).second.begin(); valIt != (*it).second.end(); ++valIt) {
                int clockVal = (*valIt).first;
                std::string sigVal = (*valIt).second;
                out << clockVal << "," << sigVal << std::endl;
            }
        }
        out.flush();
        out.close();
        std::cout << "on chip debug info file created..." << std::endl;
    }
    else {
        //insert debug values to DB.... (not to be used anymore)
        std::map<std::string, std::map<int, std::string> >::iterator it;
        for (it = onChipValues.begin(); it != onChipValues.end(); ++it) {
            std::string signalName = (*it).first;
            int sigId = DA->handleOnChipSignal(signalName);
            std::map<int, std::string>::iterator valIt;
            //DA->insertOnChipValues(sigId, (*it).second);
            for (valIt = (*it).second.begin(); valIt != (*it).second.end(); ++valIt) {
                int clockVal = (*valIt).first;
                std::string sigVal = (*valIt).second;
                DA->insertOnChipValue(sigId, clockVal, sigVal);
            }
        }    
    }
}

void formMain::actionAutomaticSignalSelection_clicked() {
    automaticSignalSelectionDialog = new DialogAutomaticSignalSelection();
    if (automaticSignalSelectionDialog->exec() == QDialog::Accepted) {
        
    } else {
        
    }
    delete automaticSignalSelectionDialog;
}

void formMain::actionSelectSignals_clicked() {
    selectSignalsDialog = new DialogSelectSignals();    
    if (selectSignalsDialog->exec() == QDialog::Accepted) {        
    }
    else {        
    }
    delete selectSignalsDialog;
}

void formMain::pushButtonExit_clicked() {    
    this->close();
}
#endif

#if defined(PYTHON_WRAPPER) || defined(DISCREP)
bool formMain::pushButtonOpenConnection_clicked() {
#else
void formMain::pushButtonOpenConnection_clicked() {
#endif

#if defined(PYTHON_WRAPPER) || defined(DISCREP)
	// need to do initialization
	loadConfigs();
	printf("OpenConnection from workDir = %s\n", workDir.c_str());
	setFileNames();
	InitializeAlteraFPPaths();
	//printf("LoadDesign - workDir = %s ;\n legUpDir = %s ;\n alteraFPPaths[0] = %s\n", workDir.c_str(), legUpDir.c_str(), alteraFPPaths[0].c_str());
#endif

#if !defined(PYTHON_WRAPPER) && !defined(DISCREP)
    if (mode == ONCHIP)
        return;
#endif

#if defined(PYTHON_WRAPPER) || defined(DISCREP)
	return tcpc.openConnection();
#else
	tcpc.openConnection();
#endif
}

void formMain::pushButtonLoadDesign_clicked() {           

#ifdef PYTHON_WRAPPER
	// need to do initialization
	loadConfigs();
	printf("LoadDesign from workDir = %s\n", workDir.c_str());
	setFileNames();
	InitializeAlteraFPPaths();
	//printf("LoadDesign - workDir = %s ;\n legUpDir = %s ;\n alteraFPPaths[0] = %s\n", workDir.c_str(), legUpDir.c_str(), alteraFPPaths[0].c_str());
#endif

    if (mode == ONCHIP)
        return;

    std::ofstream out;
    out.open((workDir + initilizeDesignTclFileName).c_str());
    out << "cd " << workDir << std::endl;
    out << "vlib work" << std::endl;
    out << "vlog -work work " << workDir << designFilename << " ";
    for (int i = 0; i < alteraFPPaths.size(); i++)
        out << alteraFPPaths[i] << " ";    
    out << std::endl;
    out << "vsim -novopt -Lf " << alteraMFLibPath << " work.main_tb" << std::endl;
    out << "add wave *" << std::endl;
    //    
    out.flush();
    out.close();
    
    std::string doCommand = "do \"" + workDir + initilizeDesignTclFileName + "\"";
    std::string result = tcpc.sendMessage(doCommand);
    // TODO std::cout << "result of do command: " << result << std::endl;
}

#ifndef PYTHON_WRAPPER
void formMain::pushButtonContinue_clicked() {
    if (mode != GDB_SYNC && mode != GDB) {
        do {
            DoStepping(true);
            if (breakPointIsHit() || simulationFinished)
                break;
        } while(true);
        if (!simulationFinished)
            updateView();
    } else {//gdb mode
        
        do {
            gdbWrapper->doStepping();
            setCurrentFunctionGDBMode();            
            std::vector<Variable*> watchVars;
            for (int i = 0 ; i < Variables.size(); i++) {
                Variable *variable = Variables[i];
                if (variable->name[0] != '%' && variable->functionId == currentFunctionGDBMode) {
                    variable->setSWValue(gdbWrapper->examineVariable(variable->name, variable->type->type, variable->numElements), gdbWrapper->previousGDBLine);
                    watchVars.push_back(variable);
                }
            }            
            fillHighLevelCodeTextEditForGDBMode();
            fillSWVariablesTable(watchVars);
            qApp->processEvents();
            
            if (gdbWrapper->programExited || breakPointIsHitForGDB())
                break;
        } while(true);
        
        //dumpVariablesValues("varlogs_gdb.txt", false);
        //std::cout << "variables are dumped (gdb mode)" << std::endl;
    }
}

void formMain::radioButtonStepInto_Toggled(bool b) {
    stepIntoRadioButtonSelected = b;
}

void formMain::treeWidgetIR_itemClicked(QTreeWidgetItem* item, int c){
    int id = item->data(0, DB_ID_ROLE).toInt();
    for (int i = 0 ; i < IRInstructions.size(); i++)
    {
        if (IRInstructions[i]->id == id)
        {
            fillHardwareTextEdit(IRInstructions[i]->hardwareInfo);
            return;
        }
    }
}
#endif

#ifdef DISCREP
void formMain::printBugMessagesToFile(std::string file) {
	// file handling
	FILE *fp = fopen(file.c_str(), "w");
	if (fp == NULL) {
		printf("Cannot open file %s for bugMessages\n");
		return;
	}

	if (bugMessages.size() > 0) {
		std::vector<std::string>::iterator it;
		for (it = bugMessages.begin(); it != bugMessages.end(); ++it) {
			fprintf(fp, "%s\n", it->c_str());
		}
	} else {
		fprintf(fp, "No bugs found\n");
	}
	return;
}
#endif

// End Event Handler Functions

#ifdef PYTHON_WRAPPER
bool SingleStep() {
	std::string runCommand = "run 20ns";
    std::string result = tcpc.sendMessage(runCommand);
	return (result != "");
}

std::string ExamineSignal(std::string signal) {
	std::string command = "examine -hex -value " + signal;
	std::string result = tcpc.sendMessage(command);
	std::vector<std::string> splits = split(result, '\r');
	return splits[0];
}

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/return_value_policy.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(formMain)
{
	def("SingleStep", SingleStep);
	def("ExamineSignal", ExamineSignal);
	class_<formMain>("formMain")
		.def("OpenConnection", &formMain::pushButtonOpenConnection_clicked)
		//.def("OpenConnection", &formMain::OpenConnection)
		.def("LoadDesign", &formMain::pushButtonLoadDesign_clicked)
	;
}
#endif // PYTHON_WRAPPER
