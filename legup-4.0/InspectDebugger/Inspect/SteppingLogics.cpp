#include "formMain.h"
#include "Globals.h"

void formMain::LineBasedDoStepping(bool shouldUpdateView) {
    if (!dbgEngine->isInitialized)
        dbgEngine->initialize();
               
    bool isReturnState = false;
    do {
        HandleFunctionCallings();
        bool ret = runForOneCycle(true);
        
        if (shouldUpdateView)
            updateView();
        
        if (IdsToStates[currentStateId]->name.find("memset") != std::string::npos ||
                IdsToStates[currentStateId]->name.find("memcpy") != std::string::npos ||
                IdsToStates[currentStateId]->name.find("memmove") != std::string::npos)
            continue;
        else
            std::cout << "state:" << IdsToStates[currentStateId]->name << std::endl;
        
        if (!isReturnState)
            isReturnState = ret;
        effectiveStatements = statesToEffectiveStatements[currentStateId];
        std::vector<int> lines = GetLineNumbers(effectiveStatements);
        
        //setting all lines as seen
        for (int i = 0; i < lines.size(); i++) {            
            seenLines[lines[i]] = true;
        }
        
        //setting all IRs as seen
        std::vector<IRInstruction*> IRs = DA->getStateInstructions(currentStateId);
        for (int ir = 0 ; ir < IRs.size(); ir++) {
            seenIRs[IRs[ir]] = true;
            //std::cout << "IRId " << IRs[ir]->id << " seen (stateId: " << currentStateId << ")" << std::endl;
        }

        //find if any seen line is passed or not
        //method: iterate through all seen lines, get their C statements, get their IRs from there
        //and check if all IRs are seen or not. If yes, the line is passed.
        //EXCEPTION: in case of function call lines: the line is passed if and only if we're in the target (calling) function!
        
        int callingFunctionLineNumber = -1;
        
        std::vector<int> visitedLines;
        std::map<int, bool>::iterator it;
        for (it = seenLines.begin(); it != seenLines.end(); ++it) {
            if ((*it).second == true) {
                int line = (*it).first;
                
                //checking calling function call condition first
                if (DA->isLineAFunctionCall(line)) {
                    int callOrBelongFId;
                    if (isReturnState && DA->getCallingFunctionIdByLineNumber(line) == finishingFunction->id) {
                        callingFunctionLineNumber = line;
                        callOrBelongFId = DA->getBelongingFunctionIdByLineNumber(line);
                        if (callOrBelongFId == currentFunction) {
                            
                            
                            std::vector<IRInstruction*> lineIRs = DA->getIRInstructions(line);
                            bool allSeen = true;
                            for (int ir = 0; ir < lineIRs.size(); ++ir) {
                                IRInstruction* IR = lineIRs[ir];
                                if (seenIRs.find(IR) == seenIRs.end() || !seenIRs[IR]) {
                                    allSeen = false;
                                    break;
                                }
                            }
                            if (allSeen) {
                                visitedLines.push_back(line);
                                (*it).second = false;
                                for (int ir = 0; ir < lineIRs.size(); ++ir)
                                    seenIRs[lineIRs[ir]] = false;
                            }
                            
                            
                            /*if (DA->getStateInstructions(currentStateId).size() > 0) //skipping dummy states (no instruction)                                
                                if (!DA->StateRelatedToHLStatement(currentStateId, callingFunctionLineNumber)) {
                                    visitedLines.push_back(line);
                                    (*it).second = false;
                                }*/
                        }
                    }
                    else {
                        callOrBelongFId = DA->getCallingFunctionIdByLineNumber(line);
                        if (callOrBelongFId == currentFunction) {
                            visitedLines.push_back(line);
                            (*it).second = false;
                        }
                    }
                } else {
                    if (lines.size() != 0) {
                        if (lineHasOnlyOneStatement(line) && lineDoesNotIncludeRetIR(line)) {
                            std::vector<IRInstruction*> lineIRs = DA->getIRInstructions(line);
                            bool allSeen = true;
                            for (int ir = 0; ir < lineIRs.size(); ++ir) {
                                IRInstruction* IR = lineIRs[ir];
                                if (seenIRs.find(IR) == seenIRs.end() || !seenIRs[IR]) {
                                    allSeen = false;
                                    break;
                                }
                            }
                            if (allSeen) {
                                visitedLines.push_back(line);
                                (*it).second = false;
                                for (int ir = 0; ir < lineIRs.size(); ++ir)
                                    seenIRs[lineIRs[ir]] = false;
                            }                            
                        }
                        else {
                            if (std::find(lines.begin(), lines.end(), line) == lines.end()) {
                                visitedLines.push_back(line);
                                (*it).second = false;
                            }
                        }
                    }                    
                }
            }
        }
        VisitLines(visitedLines);
        if (std::find(passedLines.begin(), passedLines.end(), gdbWrapper->previousGDBLine) != passedLines.end()) {
            if (isReturnState) {
                //if (std::find(passedLines.begin(), passedLines.end(), callStateHLStatement->line_number) != passedLines.end())
                if (std::find(passedLines.begin(), passedLines.end(), callingFunctionLineNumber) != passedLines.end()) {
                    //in this situation, we have to remove the entry from passedLine
                    //because this line is not being seen by GDB.
                    //all lines that are being seen by GDB are being removed after this function line (in pushButton)
                    passedLines.erase(std::find(passedLines.begin(), passedLines.end(), callingFunctionLineNumber));
                    break;
                }
            } else
                break;
        }        
                
        if (simulationFinished)
            break;                        
    } while(true);
    
    if (simulationFinished) {
            QString waitStr("Wait for the return_val...");
            this->widget.labelCurrentState->setText(waitStr);
            this->update();
            do {
                //waiting 2 seconds for the modelsim thread to be finished and the main return val to be calculated...
                sleep(2);
            } while(simulationMainReturnVal == "");
            //we can't examine the signal value cause the simulation is already finished....
            HWSignal* returnValSig = getMainReturnValSignal();
            returnValSig->setValue(simulationMainReturnVal, getCurrentStateObj()->id);            

            std::vector<std::pair<HWSignal*, int> > watch;
            int returnValInstrIdx = 0;
            for (int i = 0 ; i < IRInstructions.size(); i++)
            {
                if (std::find(IRInstructions[i]->signalList.begin(), IRInstructions[i]->signalList.end(), returnValSig) != IRInstructions[i]->signalList.end())
                {
                    returnValInstrIdx = i;
                    break;
                }
            }
            watch.push_back(std::make_pair(returnValSig, returnValInstrIdx));
            fillWatchTable(watch);

            this->widget.pushButtonSingleStepping->setEnabled(false);
            this->widget.pushButtonContinue->setEnabled(false);
            QString str("Program Finished.");
            this->widget.labelCurrentState->setText(str);

            //dumpVariablesValues("varlogs_hw.txt", true);

            //std::cout << "variables dumped to file..." << std::endl;            
    }
}

void formMain::DoSteppingForCompleteSimulation() {
    if (functionFinished)
        functionFinished = false;
    
    if (callStateSeen && stepInto) {
        currentFunction = tempCallingFunction;
        callStack.push(IdsToFunctions[currentFunction]);
        callStateSeen = false;
    }
    else {
        currentFunction = callStack.top()->id;//current function is the id of callStack top function    
        callStateSeen = false;
    }
    if (currentState == -1) {
        while ((currentState != 0)) {
            runForOneCycle();
            dummy_cycle_counter++;
        }
    }
    
    runForOneCycle(true);
    
    if (simulationFinished) {
        QString waitStr("Wait for the return_val...");
        this->widget.labelCurrentState->setText(waitStr);
        this->update();
        do {
            //waiting 2 seconds for the modelsim thread to be finished and the main return val to be calculated...
            sleep(2);
        } while(simulationMainReturnVal == "");
        //we can't examine the signal value cause the simulation is already finished....
        /*HWSignal* returnValSig = getMainReturnValSignal();
        returnValSig->setValue(simulationMainReturnVal, getCurrentStateObj()->id);

        std::vector<std::pair<HWSignal*, int> > watch;
        int returnValInstrIdx = 0;
        for (int i = 0 ; i < IRInstructions.size(); i++)
        {
            if (std::find(IRInstructions[i]->signalList.begin(), IRInstructions[i]->signalList.end(), returnValSig) != IRInstructions[i]->signalList.end())
            {
                returnValInstrIdx = i;
                break;
            }
        }
        watch.push_back(std::make_pair(returnValSig, returnValInstrIdx));
        fillWatchTable(watch);*/

        this->widget.pushButtonSingleStepping->setEnabled(false);
        this->widget.pushButtonContinue->setEnabled(false);
        QString str("Program Finished.");
        this->widget.labelCurrentState->setText(str);      
    }
    
}

void formMain::DoStepping(bool shouldUpdateView) {
       
    if (mode == ONCHIP && !dbgEngine->isInitialized)
        dbgEngine->initialize();
    
    //if the function is finished (in previous cycle) it should be popped out of callStack
    if (functionFinished) {
        functionFinished = false;
    }
    //first thing to do in stepping is to update current function if a call state is seen
    //I'm checking stepInto because function should be changed only the times that we want to step into the new function
    //times where we're at stepOver mode, callStateSeen becomes true but it should be ignored at the next move...
    //if (callStateSeen && stepInto)
    if (callStateSeen && stepIntoRadioButtonSelected)
    {
        currentFunction = tempCallingFunction;
        callStack.push(IdsToFunctions[currentFunction]);
        callStateSeen = false;
    }
    else {
        currentFunction = callStack.top()->id;//current function is the id of callStack top function    
        callStateSeen = false;
    }
    if (currentState == -1)
    {
        while ((currentState != 0))
            runForOneCycle();
    }

        //if (!stepInto)
        if (stepIntoRadioButtonSelected)
        {
            this->widget.statusbar->removeWidget(stepInLabel);
            this->widget.statusbar->show();
        }

        if (effectiveStatements.size() > 0 && effectiveLinesActiveIndex < (effectiveStatements.size()-1))
        {
            effectiveLinesActiveIndex++;
            activeStatement = effectiveStatements[effectiveLinesActiveIndex];
            if (shouldUpdateView)
                updateView();
            return;
        }

        if (stepInto && stepIntoRadioButtonSelected)
        {
            runForOneCycle(true);
            //effectiveStatements = DA->getEffectiveStatementsForState(currentState);
            effectiveStatements = statesToEffectiveStatements[currentStateId];
            if (effectiveStatements.size() > 0 && effectiveStatements[0] != activeStatement)
            {
                stepInto = false;
                this->widget.statusbar->removeWidget(stepInLabel);
                this->widget.statusbar->show();
            }
            effectiveLinesActiveIndex = 0;
            activeStatement = effectiveStatements[effectiveLinesActiveIndex];
            if (shouldUpdateView)
                updateView();
        }
        else
        {    
            int loopCount = 0;
            bool asked = false;
            do
            {
                if (loopCount > 0 && effectiveStatements.size() > 0)
                {           
                    if (stepIntoRadioButtonSelected) {
                        //this->widget.labelStepIn->setVisible(true);
                        this->widget.statusbar->addWidget(stepInLabel, 2000);
                        this->widget.statusbar->show();
                        stepInto = true;
                        effectiveLinesActiveIndex = 0;
                        activeStatement = effectiveStatements[effectiveLinesActiveIndex];
                        if (shouldUpdateView)
                            updateView();
                        return;
                    }
                }
                runForOneCycle(true);                
                //effectiveStatements = DA->getEffectiveStatementsForState(currentState);
                effectiveStatements = statesToEffectiveStatements[currentStateId];
                loopCount++;
                if (shouldUpdateView)
                    updateView();//this is just for updating state variables that are fast forwarded....                                
                if (simulationFinished)
                    break;
            } while(effectiveStatements.size() <= 0 || (effectiveStatements.size() == 1 && effectiveStatements[0] == activeStatement));

            effectiveLinesActiveIndex = 0;
            activeStatement = effectiveStatements[effectiveLinesActiveIndex];
            if (shouldUpdateView)
                updateView();
        }

        if (simulationFinished)
        {
            QString waitStr("Wait for the return_val...");
            this->widget.labelCurrentState->setText(waitStr);
            this->update();
            do {
                //waiting 2 seconds for the modelsim thread to be finished and the main return val to be calculated...
                sleep(2);
            } while(simulationMainReturnVal == "");
            //we can't examine the signal value cause the simulation is already finished....
            HWSignal* returnValSig = getMainReturnValSignal();
            returnValSig->setValue(simulationMainReturnVal, getCurrentStateObj()->id);
            
            std::vector<std::pair<HWSignal*, int> > watch;
            int returnValInstrIdx = 0;
            for (int i = 0 ; i < IRInstructions.size(); i++)
            {
                if (std::find(IRInstructions[i]->signalList.begin(), IRInstructions[i]->signalList.end(), returnValSig) != IRInstructions[i]->signalList.end())
                {
                    returnValInstrIdx = i;
                    break;
                }
            }
            watch.push_back(std::make_pair(returnValSig, returnValInstrIdx));
            fillWatchTable(watch);
            
            this->widget.pushButtonSingleStepping->setEnabled(false);
            this->widget.pushButtonContinue->setEnabled(false);
            QString str("Program Finished.");
            this->widget.labelCurrentState->setText(str);                                    
        }
}

void formMain::examineVariablesForGDB() {
    std::vector<Variable*> watchVars;
        
    if (!DA->isLineAFunctionCall(gdbWrapper->previousGDBLine)) {

        std::map<Function*, std::vector<Variable*> >::iterator it;
        for (it = functionsToVariables.begin(); it != functionsToVariables.end(); ++it) {
            Function* f = (*it).first;
            std::vector<Variable*> vars = (*it).second;


            if (gdbWrapper->changeFrame(f->name)) {
                for (int i = 0; i < vars.size(); i++) {
                    Variable* variable = vars[i];
                    if (variable->name[0] != '%') {
                        std::string exVal = gdbWrapper->examineVariable(variable->name, variable->type->type, variable->numElements);                            
                        variable->setSWValue(exVal, gdbWrapper->previousGDBLine);
                        if (gdbWrapper->isReturning) {                                        
                            variable->setSWValue(exVal, gdbWrapper->callerLineNumber);                            
                        }
                        watchVars.push_back(variable);
                    }
                }
            }                        
        }

        gdbWrapper->changeFrame(IdsToFunctions[currentFunctionGDBMode]->name);

        for (int i = 0; i < globalVariables.size(); i++) {
            Variable* variable = globalVariables[i];
            if (variable->name[0] != '%') {
                std::string exVal = gdbWrapper->examineGlobalVariable(variable->name, variable->type->type, variable->numElements);                    
                variable->setSWValue(exVal, gdbWrapper->previousGDBLine);
                if (gdbWrapper->isReturning) {                    
                    variable->setSWValue(exVal, gdbWrapper->callerLineNumber);                            
                }
                watchVars.push_back(variable);
            }
        }
    }                

    if (updateGUI) {
        fillHighLevelCodeTextEditForGDBMode();
        fillSWVariablesTable(watchVars);
        qApp->processEvents();
    }
}

void formMain::singleSteppingForGDBBugDetection() {
    assert (mode == GDB_BUG_DETECTION);
    
    this->updateGUI = true;

    changeStatusLabelColor("black");
    this->statusLabel->setText("Running ModelSim...");
    printf("Running Modelsim...\n");
    // running the ModelSim
    while (true) {
        DoStepping();
        if (updateGUI)
            qApp->processEvents();
        if (simulationFinished) {
            //dumpVariablesValues("varlogs_hw.txt", true);
            //std::cout << "HW variables dumped to file..." << std::endl;
            break;
        }
    }
    
    this->widget.pushButtonSingleStepping->setEnabled(true);
    this->widget.pushButtonContinue->setEnabled(true);

    // running the GDB
    this->statusLabel->setText("Running GDB...");
    printf("Running GDB...\n");
    while (true) {
        gdbWrapper->doStepping();
        setCurrentFunctionGDBMode();
        examineVariablesForGDB();
        if (gdbWrapper->programExited) {
            this->widget.pushButtonSingleStepping->setEnabled(false);
            this->widget.pushButtonContinue->setEnabled(false);
            QString str("Program Finished.");
            this->widget.labelCurrentState->setText(str);
            //dumpVariablesValues("varlogs_gdb.txt", false);
            //std::cout << "variables dumped to file..." << std::endl;
            break;
        }
    }

    this->statusLabel->setText("Comparing outputs...");
    printf("Comparing outputs...\n");

    detectBugs();
    if (bugMessages.size() > 0) {
        changeStatusLabelColor("red");
    int size = bugMessages.size(); 
       this->statusLabel->setText((IntToString(size) + " bugs detected...").c_str());
      
        //for (int i = 0; i < bugMessages.size(); i++)
       //     this->widget.plainTextEditLog->appendPlainText((bugMessages[i] + "\n").c_str());
     
    } else {
        changeStatusLabelColor("green");
        this->statusLabel->setText(" No bug is found...");
    }
}

void formMain::detectBugsForCompositeTypeVariables(Variable* var) {
    
    std::vector<std::pair<int, std::string> > swValues = var->SWValues;
        
    std::vector<std::vector<std::string> > swValueTokens(swValues.size());
    
    for (int v = 0; v < swValues.size(); v++) {
        std::string swVal = swValues[v].second;
        std::string newVal = "";
        std::vector<std::string> tokens = split(swVal, ',');        
        std::vector<char> trimCharList; trimCharList.push_back('{'); trimCharList.push_back('}'); trimCharList.push_back(' ');
        for (int t = 0; t < tokens.size(); ++t) {                
            std::string varNameRemoved = trimVarName(tokens[t]);
            std::string trimmedToken = trim(varNameRemoved, trimCharList);
            tokens[t] = trimmedToken;
            /*newVal += trimmedToken;
            if (t != (tokens.size() - 1))
                newVal += ",";*/
        }        
        //swValueTokens.push_back(tokens);        
        swValueTokens[v].resize(tokens.size());
        for (int t = 0; t < tokens.size(); t++)
            swValueTokens[v][t] = tokens[t];
    }
    
    
    for (int index = 0; index < var->numElements; index++) {
        std::vector<std::pair<int, std::string> > hwValues = var->HWPlainValues[index];
        std::vector<std::pair<int, int> > hwValueCycles = var->HWPlainValueCycles[index];

        std::vector<std::pair<int, std::string> > compositeSWValues;


        std::vector<std::pair<int, std::string> > hwFinalValues;
        std::vector<std::pair<int, int> > hwFinalValueCycles;        
        
        for (int v = 0; v < swValues.size(); v++) {
            /*std::string swVal = swValues[v].second;
            std::string newVal = "";
            std::vector<std::string> tokens = split(swVal, ',');            
            std::vector<char> trimCharList; trimCharList.push_back('{'); trimCharList.push_back('}'); trimCharList.push_back(' ');
            for (int t = 0; t < tokens.size(); ++t) {                
                std::string varNameRemoved = trimVarName(tokens[t]);
                std::string trimmedToken = trim(varNameRemoved, trimCharList);
                tokens[t] = trimmedToken;                
            }*/
            if (index < swValueTokens[v].size()) {
                std::string newVal = swValueTokens[v][index];
                compositeSWValues.push_back(std::make_pair(swValues[v].first, newVal));
            }
        }
        for (int l = 0; l < hwValues.size(); ++l)
            if (hwValues[l].first != -1) {
                hwFinalValues.push_back(hwValues[l]);
                hwFinalValueCycles.push_back(hwValueCycles[l]);
            }

        dumpVariableValues(compositeSWValues, var, index, "varlogs_gdb.txt");
        dumpVariableValues(hwFinalValues, var, index, "varlogs_hw.txt");

        int hIdx = 0;
            int sIdx = 0;
            for (hIdx = 0; hIdx < hwFinalValues.size(); hIdx++) {
                int hwLine = hwFinalValues[hIdx].first;
                bool hwRecordResolved = false;
                for (; sIdx < compositeSWValues.size(); sIdx++) {
                    if (hwLine == swValues[sIdx].first) {                        
                        if (!entriesMatch(compositeSWValues[sIdx].second, compositeSWValues[sIdx].first, hwFinalValues[hIdx].second, hwFinalValues[hIdx].first)) {
                            std::string message = "Bug: Variable: " + var->name + " in Function: " + IdsToFunctions[var->functionId]->name + 
                                    " Line: " + IntToString(compositeSWValues[sIdx].first) + " GDB Value: " + compositeSWValues[sIdx].second 
                                    + " HW Value: "  + hwFinalValues[hIdx].second + " HW Cycle: " + IntToString(hwFinalValueCycles[hIdx].second);
                            bugMessages.push_back(message);
                            
                            bugCounter++;
                            addBugEntry(var->name, compositeSWValues[sIdx].second, hwFinalValues[hIdx].second, compositeSWValues[sIdx].first, hwFinalValues[hIdx].first);
                        }
                        hwRecordResolved = true;
                        sIdx++;
                        break;
                    }
                }
                if (!hwRecordResolved) {
                    std::string message = "Bug: Variable: " + var->name + " in Function: " + IdsToFunctions[var->functionId]->name + 
                                    " No SW value is found for HW Value: "  + hwFinalValues[hIdx].second + " at line: " +
                            IntToString(hwValues[hIdx].first) + " HW Cycle: " + IntToString(hwFinalValueCycles[hIdx].second);
                    bugMessages.push_back(message);
                    
                    bugCounter++;
                    addBugEntry(var->name, "N/A", hwFinalValues[hIdx].second, compositeSWValues[sIdx].first, hwFinalValues[hIdx].first);                    
                }
            }
    }
}

void formMain::detectBugs() {
    assert (mode == GDB_BUG_DETECTION);
    
    bugMessages.clear();
    bugCounter = 0;
    
    for (int i = 0; i < Variables.size(); i++) {
        Variable* var = Variables[i];
        if (var->name[0] == '%')
            continue;        
            
        if (var->type->type == POINTER)
            continue;
        
        if (var->isSWValuesEmpty())
            continue;        
        
        if (var->type->type == ARRAY || var->type->type == STRUCT) {
            detectBugsForCompositeTypeVariables(var);
            continue;
        }
        
        std::vector<std::pair<int, std::string> > hwValues = var->HWPlainValues[0];
        std::vector<std::pair<int, int> > hwValueCycles = var->HWPlainValueCycles[0];
        std::vector<std::pair<int, std::string> > swValues = var->SWValues;        
        
        
        std::vector<std::pair<int, std::string> > hwFinalValues;
        std::vector<std::pair<int, int> > hwFinalValueCycles;
        //std::vector<std::pair<int, std::string> > swFinalValues;
        
        for (int l = 0; l < hwValues.size(); ++l)
            if (hwValues[l].first != -1 && hwValues[l].second.find("Not Initialized") == std::string::npos) {
                hwFinalValues.push_back(hwValues[l]);
                hwFinalValueCycles.push_back(hwValueCycles[l]);
            }
        
        dumpVariableValues(swValues, var, 0, "varlogs_gdb.txt");
        dumpVariableValues(hwFinalValues, var, 0, "varlogs_hw.txt");
        
        int hIdx = 0;
        int sIdx = 0;
        for (hIdx = 0; hIdx < hwFinalValues.size(); hIdx++) {
            int hwLine = hwFinalValues[hIdx].first;
            bool hwRecordResolved = false;
            for (; sIdx < swValues.size(); sIdx++) {
                if (hwLine == swValues[sIdx].first) {
                    if (!entriesMatch(swValues[sIdx], hwFinalValues[hIdx])) {                        
                        
                        std::string message = "Bug: Variable: " + var->name + " in Function: " + IdsToFunctions[var->functionId]->name + 
                                " Source Line: " + IntToString(swValues[sIdx].first) + " GDB Value: " + swValues[sIdx].second 
                                + " HW Value: "  + hwFinalValues[hIdx].second + " Verilog line: " + IntToString(hwValues[hIdx].first) + " HW Cycle: " + IntToString(hwFinalValueCycles[hIdx].second);
                        bugMessages.push_back(message);
                        
                        bugCounter++;
                        
                        addBugEntry(var->name, swValues[sIdx].second, hwFinalValues[hIdx].second, swValues[sIdx].first, hwFinalValues[hIdx].first);
                    }
                    hwRecordResolved = true;
                    sIdx++;
                    break;
                }
            }
            if (!hwRecordResolved) {                
                std::string message = "Bug: Variable: " + var->name + " in Function: " + IdsToFunctions[var->functionId]->name + 
                                " No SW value is found for HW Value: "  + hwFinalValues[hIdx].second + " at line: " +
                        IntToString(hwValues[hIdx].first) + " HW Cycle: " + IntToString(hwFinalValueCycles[hIdx].second);
                bugMessages.push_back(message);
                
                bugCounter++;
                addBugEntry(var->name, "N/A", hwFinalValues[hIdx].second, swValues[sIdx].first, hwFinalValues[sIdx].first);
            }
        }
    }
    
}

void formMain::addBugEntry(std::string varName, std::string SWValue, std::string HWValue, int SWLine, int HWLine) {
    
    this->widget.plainTableEditLog->setRowCount(bugCounter);
    QString varNameStr(varName.c_str());
    QTableWidgetItem* itemVarName = new QTableWidgetItem(varNameStr);
    QString SWValueStr(SWValue.c_str());
    QTableWidgetItem* itemSWValue = new QTableWidgetItem(SWValueStr);
    //QString HWValueStr(HWValue.c_str());
    QString HWValueStr(split(HWValue, '|')[0].c_str());
    QTableWidgetItem* itemHWValue = new QTableWidgetItem(HWValueStr);
    QString SWLineStr(IntToString(SWLine).c_str());
    QTableWidgetItem* itemSWLine = new QTableWidgetItem(SWLineStr);
   // QString HWLineStr(IntToString(HWLine).c_str());
   // QTableWidgetItem* itemHWLine = new QTableWidgetItem(HWLineStr);
    this->widget.plainTableEditLog->setItem(bugCounter-1, 0, itemVarName);
    this->widget.plainTableEditLog->setItem(bugCounter-1, 1, itemSWValue);
    this->widget.plainTableEditLog->setItem(bugCounter-1, 2, itemHWValue);
    this->widget.plainTableEditLog->setItem(bugCounter-1, 3, itemSWLine);
   // this->widget.plainTableEditLog->setItem(bugCounter-1, 4, itemHWLine);
}

bool formMain::entriesMatch(std::string swValue, int swLine, std::string hwValue, int hwLine) {
    std::vector<std::string> splits = split(hwValue, '|');
    std::string hwSignedVal = splits[0];
    std::string hwUSignedVal = splits[1];
    if (swLine != hwLine)
        return false;
    if (hwSignedVal == "Not Initialized")
        return true;
    
    double swDValue = atof(swValue.c_str());
    double hwSignedDValue = atof(hwSignedVal.c_str());
    double hwUSignedDValue = atof(hwUSignedVal.c_str());
    if (fabs(swDValue - hwSignedDValue) <= 0.1 || fabs(swDValue - hwUSignedDValue) <= 0.1)    
        return true;
    return false;
}

bool formMain::entriesMatch(std::pair<int, std::string>& swValue, std::pair<int, std::string>& hwValue) {
    std::vector<std::string> splits = split(hwValue.second, '|');
    std::string hwSignedVal = splits[0];
    std::string hwUSignedVal = splits[1];
    if (swValue.first != hwValue.first)
        return false;
    
    double swDValue = atof(swValue.second.c_str());
    double hwSignedDValue = atof(hwSignedVal.c_str());
    double hwUSignedDValue = atof(hwUSignedVal.c_str());
    
    if (fabs(swDValue - hwSignedDValue) <= 0.1 || fabs(swDValue - hwUSignedDValue) <= 0.1)    
        return true;
    return false;
}

void formMain::singleSteppingForGDBSync() {
    
    assert(mode == GDB_SYNC);
    
    gdbWrapper->doStepping();
    setCurrentFunctionGDBMode();
    std::cout << "previous gdb line: " << gdbWrapper->previousGDBLine << std::endl;
    std::cout << "function (GDB mode):" << IdsToFunctions[currentFunctionGDBMode]->name << std::endl;
    if (std::find(passedLines.begin(), passedLines.end(), gdbWrapper->previousGDBLine) == passedLines.end()) {
        LineBasedDoStepping();
    }
    std::vector<int>::iterator passedIt = std::find(passedLines.begin(), passedLines.end(), gdbWrapper->previousGDBLine);
    if (passedIt != passedLines.end())
        passedLines.erase(passedIt);
    std::cout << "line " << gdbWrapper->previousGDBLine << " seen and removed..." << std::endl;

    fillHighLevelCodeTextEditForGDBMode();
    std::vector<Variable*> watchVars;
    for (int i = 0 ; i < Variables.size(); i++) {
        Variable *variable = Variables[i];
        if (variable->name[0] != '%' && (variable->functionId == currentFunctionGDBMode || variable->isGlobal)) {
            variable->setSWValue(gdbWrapper->examineVariable(variable->name, variable->type->type, variable->numElements), gdbWrapper->previousGDBLine);
            watchVars.push_back(variable);
        }
    }

    fillHWVariablesTable(watchVars);
    fillSWVariablesTable(watchVars);

    checkDiscrepancy(watchVars);
}

void formMain::singleSteppingForPureGDBMode() {
    gdbWrapper->doStepping();
    setCurrentFunctionGDBMode();
    std::vector<Variable*> watchVars;
    
    std::map<Function*, std::vector<Variable*> >::iterator it;
    for (it = functionsToVariables.begin(); it != functionsToVariables.end(); ++it) {
        Function* f = (*it).first;
        std::vector<Variable*> vars = (*it).second;                

        if (gdbWrapper->changeFrame(f->name)) {
            for (int i = 0; i < vars.size(); i++) {
                Variable* variable = vars[i];
                if (variable->name[0] != '%') {
                    if (variable->name == "result") {
                        int a;
                        a = 10;
                    }
                    std::string exVal = gdbWrapper->examineVariable(variable->name, variable->type->type, variable->numElements);
                    variable->setSWValue(exVal, gdbWrapper->previousGDBLine);
                    if (gdbWrapper->isReturning) {                    
                        variable->setSWValue(exVal, gdbWrapper->callerLineNumber);
                    }
                    watchVars.push_back(variable);
                }
            }
        }                        
    }

    gdbWrapper->changeFrame(IdsToFunctions[currentFunctionGDBMode]->name);

    for (int i = 0; i < globalVariables.size(); i++) {
        Variable* variable = globalVariables[i];
        if (variable->name[0] != '%') {
            std::string exVal = gdbWrapper->examineGlobalVariable(variable->name, variable->type->type, variable->numElements);
            variable->setSWValue(exVal, gdbWrapper->previousGDBLine);
            if (gdbWrapper->isReturning) {                    
                variable->setSWValue(exVal, gdbWrapper->callerLineNumber);                            
            }
            watchVars.push_back(variable);
        }
    }     
    
    fillHighLevelCodeTextEditForGDBMode();
    fillSWVariablesTable(watchVars);
    if (gdbWrapper->programExited) {
        this->widget.pushButtonSingleStepping->setEnabled(false);
        this->widget.pushButtonContinue->setEnabled(false);
        QString str("Program Finished.");
        this->widget.labelCurrentState->setText(str);
        //dumpVariablesValues("varlogs_gdb.txt", false);
        //std::cout << "variables dumped to file..." << std::endl;
    }
}

void formMain::checkDiscrepancy(std::vector<Variable*>& watchVariables) {
    for (int i = 0; i < watchVariables.size(); i++) {
        Variable* var = watchVariables[i];
        //skipping variables that don't have correct name
        if (var->name.find("%") != std::string::npos)
            continue;
        
        //currently only support the primitive value types
        //Arrays and Structs are a little bit different as their GDB value is not parsed for each element in the type. I mean all the values are returned in one string and need to be parsed correctly...
        if (isPrimitive(var->type->type)) {
            std::string hwVal = var->getHWValueStr(0, gdbWrapper->previousGDBLine);
            std::string hwValUnsigned = var->getHWValueStr(0, gdbWrapper->previousGDBLine, false);
            std::string swVal = var->getLastSWValue();
            /*std::string hwUnsignedVal = IntToString((unsigned int)(atoi(hwVal.c_str())));
            std::string hwUnsignedShortVal = IntToString((unsigned short)(atoi(hwVal.c_str())));
            */
            
            bool valuesEqual = false;
            if (hwVal != "Not Initialized") {
                double hwDVal = atof(hwVal.c_str());
                double swDVal = atof(swVal.c_str());
                if (fabs(hwDVal - swDVal) <= 0.1)
                    valuesEqual = true;
                else {
                    double hwDValUnsigned = atof(hwValUnsigned.c_str());
                    if (fabs(hwDValUnsigned - swDVal) <= 0.1)
                        valuesEqual = true;                    
                }
            } else valuesEqual = true;
            
            if (!valuesEqual) {
                changeStatusLabelColor("red");
                std::string message = "(Bug Detected) Variable: " + var->name +  " HW value: " + hwVal + " SW value: " + swVal + "\n";
//                this->widget.plainTextEditLog->appendPlainText(QString(message.c_str()));

                this->statusLabel->setText("Bug Detected!");
            }            
        } else {
            if (var->type->type == ARRAY || var->type->type == STRUCT) {
                std::string swVal = var->getLastSWValue();
                std::vector<std::string> finalSWValues;
                std::vector<std::string> tokens = split(swVal, ',');
                std::vector<char> trimCharList; trimCharList.push_back('{'); trimCharList.push_back('}'); trimCharList.push_back(' ');
                
                for (int t = 0; t < tokens.size(); ++t) {
                    std::string varNameRemoved = trimVarName(tokens[t]);
                    std::string trimmedToken = trim(varNameRemoved, trimCharList);
                    finalSWValues.push_back(trimmedToken);                    
                }
                
                //now that we have the SW values separated we iterate through the HW list and check the values one by one. (they're all ordered)
                for (int i = 0; i < finalSWValues.size(); i++) {
                    std::string hwVal = var->getHWValueStr(i, gdbWrapper->previousGDBLine);
                    std::string hwValUnsigned = var->getHWValueStr(i, gdbWrapper->previousGDBLine, false);
                    std::string swVal = finalSWValues[i];
                    
                    /*std::string hwUnsignedVal = IntToString((unsigned int)(atoi(hwVal.c_str())));
                    std::string hwUnsignedShortVal = IntToString((unsigned short)(atoi(hwVal.c_str())));*/                    
                    
                    bool valuesEqual = false;
                    if (hwVal != "Not Initialized") {
                        double hwDVal = atof(hwVal.c_str());
                        double swDVal = atof(swVal.c_str());
                        if (fabs(hwDVal - swDVal) <= 0.1)
                            valuesEqual = true;
                        else {
                            double hwDValUnsigned = atof(hwValUnsigned.c_str());
                            if (fabs(hwDValUnsigned - swDVal) <= 0.1)
                                valuesEqual = true;                            
                        }
                    } else valuesEqual = true;
                        

                    if (!valuesEqual) {
                        changeStatusLabelColor("red");
                        std::string message = "(Bug Detected) Variable: " + var->name + "[" + IntToString(i) + "]" + " HW value: " + hwVal + " SW value: " + swVal + "\n";
//                        this->widget.plainTextEditLog->appendPlainText(QString(message.c_str()));
                        this->statusLabel->setText("Bug Detected!");
                    }                                        
                }
            }
        }
    }
}

void formMain::VisitLines(std::vector<int> &lines) {
    for (int i = 0 ; i < lines.size(); i++) {
        if (std::find(passedLines.begin(), passedLines.end(), lines[i]) == passedLines.end()) {
            passedLines.push_back(lines[i]);
            std::cout << "line " << lines[i] << " passed...!" << std::endl;            
        }
    }
}

bool formMain::lineDoesNotIncludeRetIR(int line) {
    std::map<int, HLStatement*>::iterator it;
    for (it = HLIdsToStatements.begin(); it != HLIdsToStatements.end(); ++it) {
        if ((*it).second->line_number == line) {
            for (int idx = 0; idx < (*it).second->IRs.size(); ++idx) {
                if ((*it).second->IRs[idx]->dump.find("ret") != std::string::npos)
                    return false;
            }
        }
    }
    return true;
}

bool formMain::lineHasOnlyOneStatement(int line) {
    int count = 0;
    std::map<int, HLStatement*>::iterator it;
    for (it = HLIdsToStatements.begin(); it != HLIdsToStatements.end(); ++it) {
        if ((*it).second->line_number == line)
            count++;
    }
    return (count == 1);    
}

void formMain::DoOnChipVsTimingSim() {
    assert (mode == ONCHIP_VS_TIMING_SIM);    
    
    if (simulationValues.size() == 0)
        loadReferenceSimulationData();
    
    writeOnChipDebugInfoOnFile = true;
    //TODO: I'm now working with files not DB anymore, so the debug...dat file should be removed here...
        
    bool isFinished = false;
    int triggerValue = 0;
    bool firstTime = true;
    onChipDebugWindowSize = 250;
    int windowStart = 0;
    int windowEnd = 0;
    
    std::vector<int> selectedSignalIds;    
    std::map<std::string, int> selectedExtraSignals;
    std::map<std::string, int> byDefaultAddedSignals;
    
    bool doFullCompile = true;
                
    timeval t1, t2;
    double fullTime = 0;
    double bugFindTime = 0;
    
    std::vector<double> stpTimes;
    std::vector<double> csvTimes;
    std::vector<double> compareTimes;
    std::vector<double> incScriptTimes;
    std::vector<double> dbgScriptTimes;
    
    bool bugFound = false;
  
    do
    {
        std::cout << "trigger value: " << triggerValue << std::endl;
        triggerValue += onChipDebugWindowSize;
        windowEnd = triggerValue;
        gettimeofday(&t1, NULL);
        createSTPFile(triggerValue, selectedSignalIds, selectedExtraSignals, byDefaultAddedSignals);
        gettimeofday(&t2, NULL);
        double stpDiff = getDiffTime(t1, t2);
        stpTimes.push_back(stpDiff);
        fullTime += stpDiff;
        std::cout << "STP created time: " << stpDiff << " seconds." << std::endl;                
        
        
        //should run the design with stp file in here...
        if (doFullCompile && firstTime) {
            char *argv[]={NULL};
            const char *path = dbgMakeFilePath.c_str();
            gettimeofday(&t1, NULL);
            int result = system_alternative(path, argv);
            gettimeofday(&t2, NULL);
            firstTime = false;
            double diff = getDiffTime(t1, t2);
            dbgScriptTimes.push_back(diff);
            fullTime += diff;
            std::cout << "after run in dbg make! time: " << diff << " seconds." << std::endl;                                 
        }
        else
        {
            char *argv[]={NULL};
            const char *path = increamentalDebugMakeFilePath.c_str();            
            gettimeofday(&t1, NULL);
            int result = system_alternative(path, argv); 
            gettimeofday(&t2, NULL);
            double diff = getDiffTime(t1, t2);
            incScriptTimes.push_back(diff);
            fullTime += diff;
            std::cout << "after run in incremental! time: " << diff << " seconds." << std::endl;
        }                    
        
        gettimeofday(&t1, NULL);
        isFinished = readCSVFile();
        gettimeofday(&t2, NULL);
        double csvDiff = getDiffTime(t1, t2);
        csvTimes.push_back(csvDiff);
        fullTime += csvDiff;
        std::cout << "reading csv time: " << csvDiff << " seconds." << std::endl;
        
        gettimeofday(&t1, NULL);
        for (int s = windowStart; s < windowEnd; s++) {
                        
            if (simulationValues.find(s) == simulationValues.end()){                
                continue;
            }                
            
            int currentFunctionId = simulationValues[s].first;
            
            std::string funcName = IdsToFunctions[currentFunctionId]->name;
            std::string curStateSigPartialName = funcName + "_inst|cur_state";
            
            std::vector<std::string> fullNames = onChipSignalNamesToFullNames[curStateSigPartialName];
            std::string cyclePath = simulationCycleToCallStackPath[s];
            std::string fullName = "";
            for (int i = 0; i < fullNames.size(); i++)
                if (fullNames[i].find(cyclePath) != std::string::npos) {
                    fullName = fullNames[i];
                    break;
                }
            
            if (onChipValues[fullName].find(s) == onChipValues[fullName].end())
                continue;
            
            std::string onChipCurStateValue = onChipValues[fullName][s];
            
            int simulationCurStateValue = simulationValues[s].second;
            int onChipVal = BinaryToDecimal(onChipCurStateValue);
            
            if (!bugFound && simulationCurStateValue != onChipVal) {                
                gettimeofday(&t2, NULL);
                double bDiff = getDiffTime(t1, t2);
                fullTime += bDiff;
                bugFindTime = fullTime;
                
                std::cout << "Bug Detected: Cycle: " << s << " RefSim: " << simulationCurStateValue
                          << " OnChip: " << onChipVal << std::endl;
                
                onchipBugLog.open((workDir + onchipBugLogFilename).c_str(), std::ios::app);
                onchipBugLog << "Bug Detected: Cycle: " << s << " RefSim: " << simulationCurStateValue
                          << " OnChip: " << onChipVal << "Time: " << fullTime << std::endl;
                onchipBugLog.flush();
                onchipBugLog.close();
                
                bugFound = true;
                
                //do not return and continue to find more bugs...
                //return;
            }
        }
        gettimeofday(&t2, NULL);
        double compareDiff = getDiffTime(t1, t2);
        compareTimes.push_back(compareDiff);
        fullTime += compareDiff;
        std::cout << "compare time: " << compareDiff << " seconds." << std::endl;
        windowStart = windowEnd;
    } while (!isFinished);
    
    
    std::cout << "running finished." << std::endl;
    if (bugFound)
        std::cout << "bug find time: " << bugFindTime << " seconds." << std::endl;
    else
        std::cout << "no bug found." << std::endl;
    std::cout << "full run time: " << fullTime << " seconds." << std::endl;  
    if (doFullCompile)
        std::cout << "full-compile: ON" << std::endl;
    else
        std::cout << "full-compile: OFF" << std::endl;
    
    std::cout << "STP Creation Times: ";
    for (int i = 0; i < stpTimes.size(); i++)
        std::cout << stpTimes[i] << " , ";
    std::cout << std::endl;        
    
    std::cout << "CSV Reading Times: ";
    for (int i = 0; i < csvTimes.size(); i++)
        std::cout << csvTimes[i] << " , ";
    std::cout << std::endl;
    
    std::cout << "Inc Script Times: ";
    for (int i = 0; i < incScriptTimes.size(); i++)
        std::cout << incScriptTimes[i] << " , ";
    std::cout << std::endl;
    
    std::cout << "Dbg Script Times: ";
    for (int i = 0; i < dbgScriptTimes.size(); i++)
        std::cout << dbgScriptTimes[i] << " , ";
    std::cout << std::endl;
    
    std::cout << "Compare Times: ";
    for (int i = 0; i < compareTimes.size(); i++)
        std::cout << compareTimes[i] << " , ";
    std::cout << std::endl;
    
    //TODO: why return? can't remember...
    return;
    
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
        return;
    }
    else {
        //insert debug values to DB....
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
