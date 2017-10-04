/* 
 * File:   newForm.h
 * Author: nazanin
 *
 * Created on July 10, 2013, 11:03 PM
 */

#ifndef _FORMMAIN_H
#define	_FORMMAIN_H

#ifndef PYTHON_WRAPPER
#include <iostream>
#include <sys/time.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "ui_newForm.h"
#endif

#include "Utility.h"
#include "TCPClient.h"

#ifndef PYTHON_WRAPPER
#include "HLStatement.h"
#include "DataAccess.h"
#include "STPCreator.h"
#include "CSVReader.h"
#include "OnChipDebugEngine.h"
#include "DialogSelectSignals.h"
#include "DialogAutomaticSignalSelection.h"
#endif

#include "GDBWrapper.h"

#include <stack>
#include <math.h>

/*
extern TCPClient tcpc;
extern std::string remoteTclFileName;
extern std::string initilizeDesignTclFileName;
extern std::string vsimRunCommand;
extern std::string vsimDir;
extern std::string workDir;
extern std::string fileName;
extern std::string legUpDir;
extern std::string alteraMFLibPath;
extern std::vector<std::string> alteraFPPaths;
extern std::string designFilename;
extern std::string codeFilename;
extern std::string rawSourceFilename;
extern std::string nodeNamesFilename;
extern std::string stpFilename;
extern std::string csvFileName;

extern DataAccess *DA;
extern std::vector<IRInstruction*> IRInstructions;
extern std::vector<HWSignal*> Signals;
extern std::vector<State*> States;
extern std::map<int, State*> IdsToStates;
extern std::vector<Variable*> Variables;
extern std::map<int, Variable*> IdsToVariables;
extern std::map<Variable*, VariableUpdateInfo*> variablesToUpdateInfo;
extern std::map<int, std::vector<HLStatement*> > statesToEffectiveStatements;
extern std::vector<Function*> functions;
extern std::map<int, Function*> IdsToFunctions;

extern std::map<Function*, std::vector<Variable*> > functionsToVariables;
extern std::vector<Variable*> globalVariables;

extern bool writeOnChipDebugInfoOnFile;
extern std::string onChipDebugInfoFileAddress;
extern OnChipDebugEngine* dbgEngine;

extern std::string dbgMakeFilePath;
extern std::string increamentalDebugMakeFilePath;

extern int onChipDebugWindowSize;

extern GDBWrapper *gdbWrapper;
extern std::vector<State*> observedStates;
extern std::string simulationMainReturnVal;

extern int cycle_counter;
extern int dummy_cycle_counter;

extern std::string referenceSimulationDataFilename;
extern std::string referenceSimulationCyclesToPathsFileName;
extern std::string onchipBugLogFilename;
*/

//enumeration representing four different general modes of the debugger
enum SYSTEM_MODE {
    MODELSIM,
    ONCHIP,
    GDB_SYNC,
    GDB,
    GDB_BUG_DETECTION,
    ONCHIP_VS_TIMING_SIM
};

#ifdef PYTHON_WRAPPER
// simplified formMain class for python wrapper
class formMain {
public:
    formMain();
    virtual ~formMain();
    void Initialize();    
    void pushButtonLoadDesign_clicked();
#ifdef PYTHON_WRAPPER
    bool pushButtonOpenConnection_clicked();    
#else
    void pushButtonOpenConnection_clicked();    
#endif
    
private:
    enum SYSTEM_MODE mode;
};

#else // PYTHON_WRAPPER
class formMain : public QMainWindow {
    Q_OBJECT
public:
    //newForm(bool _gdbMode = false);
    formMain(SYSTEM_MODE _mode = MODELSIM);
    virtual ~formMain();
    void fillSourceCodeTab();
    void InitializeData();
    void Initialize();    
    void InitializeDiscrepancyLogTable();
    
public slots:
    void pushButtonLoadDesign_clicked();
#ifdef DISCREP
    bool pushButtonOpenConnection_clicked();    
#else
    void pushButtonOpenConnection_clicked();    
#endif
    void pushButtonSingleStepping_clicked();
    void singleSteppingForGDBSync();
    void singleSteppingForPureGDBMode();
    void singleSteppingForGDBBugDetection();    
    void treeWidgetIR_itemClicked(QTreeWidgetItem* item, int c);
    void pushButtonExit_clicked();
    void actionView_IR_Instructions_changed();
    void actionView_HW_Info_changed();
    void actionView_Watch_changed();
    void radioButtonStepInto_Toggled(bool b);
#ifdef DISCREP
    void printBugMessagesToFile(std::string file);
#endif
    void textEditHighLevel_CursorPositionChanged();
    void textEditLineNumbers_CursorPositionChanged();
    void pushButtonContinue_clicked();
    void actionReBuildCode_clicked();
    void actionRunOnChip_clicked();
    void actionRunReferenceSim_clicked();
    void loadReferenceSimulationData();
    void actionSelectSignals_clicked();
    void actionAutomaticSignalSelection_clicked();
    void plainTableEditLog_itemClicked(QTableWidgetItem*);
    void HighlightBugInCCode(int highlightingLine);
    
private:
    Ui::formMain widget;
    QLabel *statusLabel;
    
    enum SYSTEM_MODE mode;
    
    DialogSelectSignals *selectSignalsDialog;
    DialogAutomaticSignalSelection *automaticSignalSelectionDialog;
    QLabel *stepInLabel;
    std::vector<std::string> sourceLines;//keeping all C lines
    HLStatement* activeStatement;
    HLStatement* lastActiveStatement;
    HLStatement* selectedStatement;
    
    int activeLine;    
    std::vector<int> passedLines;
    std::map<int, bool> seenLines;
    std::map<IRInstruction*, bool> seenIRs;
    
    std::vector<HLStatement*> breakPointStatements;
    int sourceFileLineOffset;
    int currentState;//current state number (not unique across functions)
    int currentStateId;//current state id (always unique)
    int currentFunction;
    int currentFunctionGDBMode;
    int tempCallingFunction;//temporary variable holding next calling function id
    //int callStateLineNumber;
    HLStatement* callStateHLStatement;
    Function* finishingFunction;//this is the function that its finish signal is being set to one...
    bool simulationFinished;//checking finish signal for main function
    bool functionFinished;//checking if current function is finished or not. (In order to find if the program finished or not SimulationFinished should be checked)
    bool stepInto;
    
    bool isWorkingOnBreakPoints;
    int lastStateIdObserved;
    std::vector<std::string> bugMessages;
    int bugCounter;
    
    std::vector<IRInstruction*> activeIRs;
    std::stack<Function*> callStack;    
    std::string getCurrentFunctionVPath();
    std::string getCurrentFunctionVPathForOnChip();
    std::string getTopModuleVPath();
    std::string getMainModuleVPath();
    
    int DB_ID_ROLE;
    
    bool stepIntoRadioButtonSelected;
    bool callStateSeen;
    
    std::vector<HLStatement*> effectiveStatements;
    int effectiveLinesActiveIndex;
    
    std::vector<int> GetLineNumbers(std::vector<HLStatement*>& statements);
    void VisitLines(std::vector<int> &lines);
    void HandleFunctionCallings();
    
    void changeStatusLabelColor(std::string color);
    
    void setCurrentFunctionGDBMode();    
    /*void dumpVariablesValues(std::string fileName, bool isHW);*/
    void dumpVariableValues(std::vector<std::pair<int, std::string> >& values, Variable* var, int index, std::string fileName);
    void DoSteppingForCompleteSimulation();
    void DoStepping(bool updateView = true);    
    void ResetLocalVariablesLastSetValues();
    void DoOnChipVsTimingSim();    
    void RunCompleteSimulation();
    void LineBasedDoStepping(bool shouldUpdateView = true);
    bool lineHasOnlyOneStatement(int line);
    bool lineDoesNotIncludeRetIR(int line);
    bool breakPointIsHit();
    bool breakPointIsHitForGDB();
    void fillLineNumbers();
    void fillBreakPointsList();
    void fillHighLevelCodeTextEdit();
    void fillHighLevelCodeTextEditForGDBMode();
    void fillIRTextEdit();
    void fillHardwareTextEdit(std::vector<std::string> items);
    //void fillWatchTable(std::vector<HWSignal*>& watchSignals);    
    void fillWatchTable(std::vector<std::pair<HWSignal*, int> >& watchSignals);
    void handleHighLevelCodeText();    
    void updateView(bool updateWatchTable = true);
    void setCurrentState(std::string msg);
    void setMainReturnVal(std::string msg);
    bool runForOneCycle(bool checkFinishSignal = false);    
    //void batchExamine(std::vector<HWSignal*>& watchSignals);
    void batchExamine(std::vector<std::pair<HWSignal*, int> >& watchSignals);
    void batchExamineForVariables(State* state);
    //void processBatchExamineCommandResults(std::vector<HWSignal*>& watchSignals, std::string result);
    void processBatchExamineCommandResults(std::vector<std::pair<HWSignal*, int> >& watchSignals, std::string result);
    void processBatchExamineCommandResultsForVariables(State* state, std::string result);
    bool isInsideLegUpSpecificFunctions();
    //std::string trimMessage(std::string msg);    
        
    void examineVariablesForGDB();
    void detectBugs();    
    void detectBugsForCompositeTypeVariables(Variable* var);
    bool entriesMatch(std::pair<int, std::string>& swValue, std::pair<int, std::string>& hwValue);
    bool entriesMatch(std::string swValue, int swLine, std::string hwValue, int hwLine);
    void addBugEntry(std::string varName, std::string SWValue, std::string HWValue, int SWLine, int HWLine);
    
    
    HWSignal* getMainReturnValSignal();
    State* getCurrentStateObj();
        
    bool shouldBeActive(int line, int col);
    bool shouldBeEffective(int line, int col);
    
    void updateHWWatchTable();
    void updateHWVariablesTable();
    void updateSWVariablesTable();
    void updateVariableValues();
    void fillHWVariableNode(Variable* variable, VariableType* varType, QTreeWidgetItem* treeItem, std::string leafText);
    std::string getVariableHWValueStr(int index, Variable* variable);
    std::string getTypedValueString(std::string rawValue, Type type);
    int HWVariableTableValueIndex;
    bool isPrimitive(Type type);
    void checkDiscrepancy(std::vector<Variable*>& watchVariables);
    void fillHWVariablesTable(std::vector<Variable*>& watchVariables);
    void fillSWVariablesTable(std::vector<Variable*>& watchVariables);
    bool isCurrentFunctionFinished();
    
    Variable* getVariableByTagNum(int tagNum);
    
    //void createSTPFile(int triggerValue);      
    void createSTPFile(int triggerValue, std::vector<int>& selectedSignalIds, std::map<std::string, int>& selectedExtraSignals, std::map<std::string, int>& byDefaultAddedSignals);
    bool readCSVFile();
    
    std::map<std::string, std::map<int, std::string> > onChipValues;// signal Name -> (cycle, value)
    //std::map<std::string, std::string> onChipSignalNamesToFullNames;
    std::map<std::string, std::vector<std::string> > onChipSignalNamesToFullNames;
    OnChipDebugMode signalSelectionMode;        
    
    //std::vector<simCycleOrder*> simulationCycleOrder;
    std::map<int, std::pair<int, int> > simulationValues;//cycle -> (function, stateNumber)
    std::map<int, std::string> simulationCycleToCallStackPath;
       
        
    std::ofstream variablesLog;
    bool updateGUI;
    std::ofstream onchipBugLog;
};
#endif // PYTHON_WRAPPER

#endif	/* _NEWFORM_H */
