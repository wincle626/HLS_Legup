/* 
 * File:   DataAccess.cpp
 * Author: nazanin
 * 
 * Created on June 19, 2013, 9:06 AM
 * this file contains all methods to load and store data from/to the MySQL database
 */

#include "DataAccess.h"
#include "OnChipSignal.h"
#include "Globals.h"


DataAccess::DataAccess() {
    
    initializeDatabase();
}

DataAccess::~DataAccess() {
}

void DataAccess::runQuery(std::string query) {        
    //std::cout << "query: " << query << std::endl;
    int err_code = mysql_query(connection, query.c_str());
    if (err_code)//non-zero return means error!
    {
        std::cout << "something went wrong with the query! error code: " << err_code << std::endl;
        std::cout << mysql_error(connection) << std::endl;
        std::cout << "query: " << query << std::endl;
    }
    else
        result = mysql_store_result(connection);
}

void DataAccess::initializeDatabase() {
   mysql_init(&mysql);
   connection = mysql_real_connect(&mysql, dbHost.c_str(), dbUser.c_str(), dbPass.c_str(), dbName.c_str(), 3306, 0, CLIENT_MULTI_STATEMENTS);
   if (connection == NULL)
   {
       std::cout << "SQL connection error!" << std::endl;
   }      
}

void DataAccess::closeConnection() {    
    mysql_close(&mysql);
}

//queries
std::vector<IRInstruction*> DataAccess::getIRInstructions(int highLevelLineNumber, int stateNumber) {
    int stateId = -1;
    std::vector<IRInstruction*> res;
    std::string q = "SELECT id from State where number = ";
    q += IntToString(stateNumber);
    q += ";";
    runQuery(q);
    MYSQL_ROW r;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        stateId = atoi(row[0]);
    }
    else { 
        mysql_free_result(result);
        return res;
    }
        
    std::string query = "SELECT IRInstr.id from IRInstr join HLStatement on IRInstr.HLStatement_id = HLStatement.id AND HLStatement.line_number = ";
    query += IntToString(highLevelLineNumber);
    query += " AND IRInstr.id in (SELECT IRInstr_id from IRState where StartStateId <= ";
    query += IntToString(stateId);
    query += " AND EndStateId >= ";
    query += IntToString(stateId);
    query += ")";
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);        
        for (int i = 0 ; i < IRInstructions.size(); i++)
        {
            if (IRInstructions[i]->id == id)
            {                
                res.push_back(IRInstructions[i]);
                break;
            }
        }
    }    
    mysql_free_result(result);
    return res;
}

std::vector<IRInstruction*> DataAccess::getIRInstructions(int highLevelLineNumber) {
    //select IRInstr.dump from IRInstr join HLStatement on IRInstr.HLStatement_id = HLStatement.id AND HLStatement.line_number = 6
    std::vector<IRInstruction*> res;
    std::string query = "SELECT IRInstr.id from IRInstr join HLSTate on IRInstr.HLStatement_id = HLStatement.id AND HLStatement.line_number = ";
    query += IntToString(highLevelLineNumber);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);        
        for (int i = 0 ; i < IRInstructions.size(); i++)
        {
            if (IRInstructions[i]->id == id)
            {                
                res.push_back(IRInstructions[i]);
                break;
            }
        }
    }    
    mysql_free_result(result);
    return res;
}

//this method must be called only once unless there are memory leaks...
void DataAccess::getAllHWSignals() {
    IdsToSignals.clear();
    Signals.clear();
    std::string query = "select * from HWSignal;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        std::string name = row[1];
        int width = atoi(row[2]);
        int functionId = atoi(row[3]);
        bool isConst = atoi(row[4]) == 0 ? false : true;        
        
        Variable* variable = NULL;
        if (row[5] != NULL)
            variable = IdsToVariables[atoi(row[5])];
        
        HWSignal* sig = new HWSignal(id, name, width, IdsToFunctions[functionId], isConst, variable);
        IdsToSignals[id] = sig;
        Signals.push_back(sig);
    }
    mysql_free_result(result);
}

void DataAccess::getAllOnChipSignals() {
    IdsToOnChipSignals.clear();
    OnChipSignals.clear();    
    std::string query = "select * from OnChipSignal;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        std::string name = row[1];
        
        OnChipSignal* sig = new OnChipSignal(id, name);
        IdsToOnChipSignals[id] = sig;
        OnChipSignals.push_back(sig);
    }
    mysql_free_result(result);
}

std::vector<HWSignal*> DataAccess::getAllFinishSignals() {
    std::vector<HWSignal*> res;
    std::string query = "select id from HWSignal where (name = 'finish');";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        HWSignal* sig = IdsToSignals[id];
        res.push_back(sig);        
    }
    mysql_free_result(result);
    return res;
}

std::vector<HWSignal*> DataAccess::getAllCurStateSignals() {
    std::vector<HWSignal*> res;
    std::string query = "select id from HWSignal where (name = 'cur_state');";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        HWSignal* sig = IdsToSignals[id];
        res.push_back(sig);        
    }
    mysql_free_result(result);
    return res;
}

std::vector<HWSignal*> DataAccess::getOnChipWatchSignals() {
    std::vector<HWSignal*> res;
    std::string query = "select id from HWSignal where ((name = 'cur_state') OR (id in (select HWSignal_id from InstructionSignal) OR id in (select HWSignal_id from StateVariableIndex)));";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        HWSignal* sig = IdsToSignals[id];
        res.push_back(sig);        
    }
    mysql_free_result(result);
    return res;
}

void DataAccess::getAllHLStatements() {
    HLIdsToStatements.clear();
    lineNumToStatement.clear();
    std::string query = "select IRInstr.id as IRId, HLStatement.* from IRInstr join HLStatement on IRInstr.HLStatement_id = HLStatement.id;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int IRid = atoi(row[0]);
        int id = atoi(row[1]);
        int lineNum = atoi(row[2]);
        int colNum = atoi(row[3]);
        std::string fileName = row[4];
        HLStatement* statement;
        if (HLIdsToStatements.find(id) != HLIdsToStatements.end())
            statement = HLIdsToStatements[id];
        else
        {
            statement = new HLStatement(id, lineNum, colNum, fileName);
            HLIdsToStatements[id] = statement;
        }        
        if (std::find(lineNumToStatement[lineNum].begin(), lineNumToStatement[lineNum].end(), statement) == lineNumToStatement[lineNum].end())
            lineNumToStatement[lineNum].push_back(statement);        
        
        if (IRIdsToInstructions.find(IRid) != IRIdsToInstructions.end()) {
            statement->IRs.push_back(IRIdsToInstructions[IRid]);
        }
        
        //statement->IRs.push_back(IRIdsToInstructions[IRid]);
    }
    mysql_free_result(result);
}

bool functionsStartLineNumberSortFunc(Function* f, Function *g) {
    return (f->startLineNumber < g->startLineNumber);
}

void DataAccess::getAllFunctions() {
    IdsToFunctions.clear();
    functions.clear();
    std::string query = "SELECT id, name, startLineNumber from Function;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) {
        int id = atoi(row[0]);
        std::string name = row[1];
        int startLineNumber = atoi(row[2]);
        Function *f = new Function(id, name, startLineNumber);
        functions.push_back(f);
        IdsToFunctions[id] = f;
    }
    //sort functions by their startLineNumber...
    //TODO: this should be changed when we want to support multiple file entries...
    std::sort(functions.begin(), functions.end(), functionsStartLineNumberSortFunc);
    mysql_free_result(result);
    /*for (int i = 0 ; i < functions.size(); i++)
    {
        std::cout << functions[i]->name << " : " << functions[i]->startLineNumber << std::endl;
    }*/
}

void DataAccess::getAllStates() {
    IdsToStates.clear();
    States.clear();
    std::string query = "SELECT id, belongingFunctionId, calledFunctionId, number, design_name from State;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);        
        Function *belongingFunc = IdsToFunctions[atoi(row[1])];
        Function *calledFunc = NULL;
        if (row[2] != NULL)
            calledFunc = IdsToFunctions[atoi(row[2])];
        int number = atoi(row[3]);        
        std::string name = row[4];
        State* state = new State(id, number, name, belongingFunc, calledFunc);
        IdsToStates[id] = state;
        States.push_back(state);
    }
    mysql_free_result(result);
}

std::vector<std::string> DataAccess::getVariableData(int varId) {
    std::vector<std::string> res;
    std::string query = "SELECT value from VariableData where variable_id = ";
    query += IntToString(varId);
    query += " order by order_num asc;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) {
        res.push_back(row[0]);        
    }
    mysql_free_result(result);
    return res;
}

void DataAccess::getAllVariableTypes() {
    IdsToVariableTypes.clear();
    VariableTypes.clear();
    std::string query = "SELECT * from VariableType;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        int typeId = atoi(row[1]);
        int numElements = atoi(row[2]);
        int byteSize = atoi(row[3]);
        VariableType* vt = new VariableType(id, getTypeByDBId(typeId), numElements, byteSize);
        IdsToVariableTypes[id] = vt;
        VariableTypes.push_back(vt);
    }
    mysql_free_result(result);
}

void DataAccess::fillVariableTypeElements(VariableType* type) {
    std::string query = "SELECT * from TypeElement where parentTypeId = ";
    query += IntToString(type->id) + ";";
    runQuery(query);
    std::vector<int> typeIds;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) {
        int elementTypeId = atoi(row[1]);
        type->elementTypes.push_back(IdsToVariableTypes[elementTypeId]);        
        typeIds.push_back(elementTypeId);
    }
    mysql_free_result(result);
    for (int i = 0; i < typeIds.size(); i++)
        fillVariableTypeElements(IdsToVariableTypes[typeIds[i]]);    
}

void DataAccess::getAllVariables() {
    IdsToVariables.clear();
    Variables.clear();
    std::string query = "SELECT * from Variable;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        std::string name = row[1];
        int functionId = atoi(row[2]);
        std::string tag = row[3];
        int tagNum = atoi(row[4]);
        std::string tagAddressName = row[5];
        int addressWidth = atoi(row[6]);
        std::string mifFileName = row[7];
        int dataWidth = atoi(row[8]);
        int numElements = atoi(row[9]);
        bool isStruct = atoi(row[10]) == 0 ? false : true;
        IRInstruction* IR = NULL;
        if (row[11] != NULL)
            IR = IRIdsToInstructions[atoi(row[11])];
        int typeId = atoi(row[13]);
        
        Variable* variable = new Variable(id, functionId, name, tag, tagNum, tagAddressName, addressWidth, mifFileName, dataWidth, numElements, isStruct, IR, IdsToVariableTypes[typeId]);
        
        IdsToVariables[id] = variable;
        Variables.push_back(variable);
    }
    mysql_free_result(result);
    for (int i = 0; i < Variables.size(); i++) {
        fillVariableTypeElements(Variables[i]->type);
        Variables[i]->numElements = Variables[i]->calculateNumElements(Variables[i]->type);
        Variables[i]->InitializeContainers();
    }    
    
}

Type DataAccess::getTypeByDBId(int id) {
    switch(id) {
        case 0:
            return PRIMITIVE_INT;
        case 1:
            return PRIMITIVE_FLOAT;
        case 2:
            return PRIMITIVE_DOUBLE;
        case 3:
            return POINTER;
        case 4:
            return ARRAY;
        case 5:
            return STRUCT;
    }
    return PRIMITIVE_INT;
}

VariableType* DataAccess::getVariableType(int typeId) {
    std::string query = "SELECT * from VariableType where id = ";
    query += IntToString(typeId);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result)) != NULL) {
        int varTypeId = atoi(row[1]);
        int numElements = atoi(row[2]);
        int byteSize = atoi(row[3]);
        VariableType *vt = new VariableType(typeId, getTypeByDBId(varTypeId), numElements, byteSize);        
    }
    mysql_free_result(result);
}

std::vector<VariableUpdateInfo*> DataAccess::getStateUpdatingVariables(int stateId) {   
    std::vector<VariableUpdateInfo*> res;
    std::string query = "SELECT * from StateStoreInfo where State_id = ";
    query += IntToString(stateId);
    query += ";";
    runQuery(query);
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int infoID = atoi(row[0]);
        int signalId = atoi(row[2]);
        std::string port = row[3];
        int offset = 0;
        if (row[4] != NULL)
            offset = atoi(row[4]);
        int IRId = atoi(row[5]);
        int lineNumber = IRIdsToInstructions[IRId]->lineNumber;
        
                
        res.push_back(new VariableUpdateInfo(infoID, IdsToSignals[signalId], port, offset, lineNumber));
    }
    mysql_free_result(result);
    return res;
}

void DataAccess::getAllIRInstructions() {
    IRIdsToInstructions.clear();
    IRInstructions.clear();
        
    std::string query = "SELECT IRInstr.id, IRInstr.function_id, IRInstr.dump, IRState.StartStateId, IRState.EndStateId, IRInstr.HLStatement_id from IRInstr join IRState on IRInstr.id = IRState.IRInstr_id;";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        int function_id = atoi(row[1]);
        std::string dump = row[2];
        int startStateId = atoi(row[3]);
        int endStateId = atoi(row[4]);
        
        
        IRInstruction *instr = new IRInstruction(id, dump);
                        
        if (row[5] != NULL) {
            instr->HLInstr_id = atoi(row[5]);            
        }
        instr->startState = IdsToStates[startStateId];
        instr->endState = IdsToStates[endStateId];
        instr->function_id = function_id;        
        IRIdsToInstructions[id] = instr;
        IRInstructions.push_back(instr);
    }    
    mysql_free_result(result);
}

std::vector<std::string> DataAccess::getIRHardwareInfo(int IRid) {
    std::vector<std::string> res;
    std::string query = "SELECT info from HardwareInfo where IRid = ";
    query += IntToString(IRid);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {        
        std::string info = row[0];        
        res.push_back(info);
    }
    mysql_free_result(result);
    return res;
}
 
std::string DataAccess::getStateNameByNumberAndBelongingFunctionId(int stateNum, int belongingFunctionId, int &stateId) {
    std::string res = "";
    std::string query = "SELECT id, design_name from State where number = ";
    query += IntToString(stateNum);
    query += " AND belongingFunctionId = ";
    query += IntToString(belongingFunctionId);
    query += ";";
    runQuery(query);
    MYSQL_ROW r;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        stateId = atoi(row[0]);
        res = row[1];
    }
    mysql_free_result(result);
    return res;
}

std::vector<HWSignal*> DataAccess::getIRSignals(int instructionId) {
    std::vector<HWSignal*> res;
    std::string query = "SELECT HWSignal_id from InstructionSignal where IRInstr_id = ";
    query += IntToString(instructionId);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);        
        res.push_back(IdsToSignals[id]);
    }
    mysql_free_result(result);
    return res;            
}

int DataAccess::findEndState(IRInstruction *instr) {
    std::string query = "SELECT EndStateId from IRState where IRInstr_id = ";
    query += IntToString(instr->id);
    query += ";";
    runQuery(query);
    int stateId;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        stateId = atoi(row[0]);
    }
    
    std::string q = "SELECT number from State where id = ";
    q += IntToString(stateId);
    q += ";";
    runQuery(q);
    int res;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = atoi(row[0]);
    }
    mysql_free_result(result);
    return res;    
}

std::vector<IRInstruction*> DataAccess::getStateInstructions(int stateId) {
    std::vector<IRInstruction*> res;
    std::string query = "SELECT IRInstr_id from IRState where StartStateId <= ";
    query += IntToString(stateId);
    query += " AND EndStateId >= ";
    query += IntToString(stateId);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        res.push_back(IRIdsToInstructions[id]);
    }
    mysql_free_result(result);
    return res;
}

std::vector<HLStatement*> DataAccess::getEffectiveStatementsForState(int stateId) {    
    std::vector<HLStatement*> res;    
    //select id from HLStatement where id in (select HLStatement_id from IRInstr where id in (select IRInstr_id from IRState where StartStateId <= 2 AND EndStateId >= 2))    
    std::string query = "SELECT id from HLStatement where id in (select HLStatement_id from IRInstr where id in (select IRInstr_id from IRState where StartStateId <= ";
    query += IntToString(stateId);
    query += " AND EndStateId >= ";
    query += IntToString(stateId);
    query += "));";
    runQuery(query);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int id = atoi(row[0]);
        res.push_back(HLIdsToStatements[id]);
    }
    mysql_free_result(result);
    return res;
}

int DataAccess::getSignalId(std::string name) {
    std::string query = "SELECT id from OnChipSignal where name= '";
    query += name;
    query += "';";
    runQuery(query);
    int res;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = atoi(row[0]);
    }
    mysql_free_result(result);
    return res; 
    
}

std::vector<State*> DataAccess::getSignalStates(int signalId) {    
    std::string query = "select StartStateId, EndStateId from IRState where IRInstr_id in (";
    query += "select IRInstr_id from InstructionSignal where HWSignal_id = ";
    query += IntToString(signalId);
    query += ");";
    runQuery(query);
    std::vector<State*> res;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int startStateId = atoi(row[0]);
        int endStateId = atoi(row[1]);
        for (int idx = startStateId; idx <= endStateId; idx++) {
            State* state = IdsToStates[idx];
            if (std::find(res.begin(), res.end(), state) == res.end())
                res.push_back(state);
        }        
    }
    mysql_free_result(result);
    return res;
}

int DataAccess::insertOnChipSignal(std::string name) {
    std::string query = "INSERT into OnChipSignal(name) values('" + name + "'";
    query += ");select LAST_INSERT_ID() as id;";
    //mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
    runQuery(query);
    int res = -1;
    mysql_next_result(connection);
    result = mysql_store_result(connection);
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = atoi(row[0]);
    }
    //mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_OFF);    
    mysql_free_result(result);
    return res;
}

int DataAccess::handleOnChipSignal(std::string name) {    
    return insertOnChipSignal(name);    
}
//obsolete function
void DataAccess::DeleteOnChipSignals() {
    std::string query = "DELETE from OnChipSignal where 1 = 1;";
    runQuery(query);
    mysql_free_result(result);//TODO!?!
}

int DataAccess::getOnChipSignalID(std::string name) {
    std::string query = "SELECT id from OnChipSignal where name = '";
    query += name;
    query += "';";
    runQuery(query);
    int res = -1;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = atoi(row[0]);
    }
    mysql_free_result(result);
    return res;
}

std::string DataAccess::OnChipGetSignalNameByPartialName(std::string partialName) {
    std::string query = "SELECT name from OnChipSignal where name like '%" + partialName + "%';";
    runQuery(query);
    std::string res;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = row[0];
        return res;
    }
    mysql_free_result(result);
    return "";
}

std::string DataAccess::onChipGetValue(int sigId, int time) {
    std::string query = "SELECT value from OnChipSignalValue where OnChipSignal_id = ";
    query += IntToString(sigId);
    query += " AND time = ";
    query += IntToString(time);
    query += ";";
    runQuery(query);
    std::string res;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = row[0];
        return res;
    }
    mysql_free_result(result);
    return NULL;
}

bool DataAccess::onChipValueExists(int sigId, int time) {
    std::string query = "SELECT count(*) from OnChipSignalValue where OnChipSignal_id = ";
    query += IntToString(sigId);
    query += " AND time = ";
    query += IntToString(time);
    query += ";";
    runQuery(query);
    int res = 0;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        res = atoi(row[0]);
    }
    mysql_free_result(result);
    if (res > 0)
        return true;
    return false;
}

void DataAccess::handleOnChipValue(int sigId, int time, std::string value) {
    //if (!onChipValueExists(sigId, time))
    insertOnChipValue(sigId, time, value);
}

void DataAccess::insertOnChipValues(int sigId, std::map<int, std::string>& sigValMapping) {
    std::string query = "";
    std::map<int, std::string>::iterator it;
    for (it = sigValMapping.begin(); it != sigValMapping.end(); ++it) {
        query += "INSERT into OnChipSignalValue(time, OnChipSignal_id, value) values( "
                + IntToString((*it).first)
                + ", " + IntToString(sigId)
                + ", '" + (*it).second
                + "');";
    }
    //mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
    runQuery(query); 
    mysql_free_result(result);
    //mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_OFF);
}

void DataAccess::insertOnChipValue(int sigId, int time, std::string value) {
    std::string query = "INSERT into OnChipSignalValue(time, OnChipSignal_id, value) values( ";
    query += IntToString(time);
    query += ", " + IntToString(sigId);
    query += ", '" + value;
    query += "'";    
    query += ");";
    runQuery(query);
    mysql_free_result(result);
}

int DataAccess::getBelongingFunctionIdByLineNumber(int lineNumber) {
    std::string query = "select belongingFunctionId from State where id in (";
    query += "select StartStateId from IRState where IRInstr_id in (";
    query += "select id from IRInstr where HLStatement_id in (";
    query += "select id from HLStatement where line_number = ";
    query += IntToString(lineNumber);
    query += ") AND dump like '%call%'));";
    runQuery(query);
    int ret;
    if ((row = mysql_fetch_row(result)) != NULL)
        ret = atoi(row[0]);
    else
        ret = -1;    
    mysql_free_result(result);
    return ret;
    
}

int DataAccess::getCallingFunctionIdByLineNumber(int lineNumber) {
    std::string query = "select calledFunctionId from State where id in (";
    query += "select StartStateId from IRState where IRInstr_id in (";
    query += "select id from IRInstr where HLStatement_id in (";
    query += "select id from HLStatement where line_number = ";
    query += IntToString(lineNumber);
    query += ") AND dump like '%call%'));";
    runQuery(query);
    int ret;
    if ((row = mysql_fetch_row(result)) != NULL)
        ret = atoi(row[0]);
    else ret = -1;
    mysql_free_result(result);
    return ret;
    
}

bool DataAccess::isLineAFunctionCall(int lineNumber) {
    std::string query = "select * from IRInstr where HLStatement_id in (";
    query += "select id from HLStatement where line_number = ";
    query += IntToString(lineNumber);
    query += ") AND dump like '%call%' AND dump not like '%printf%';";//TODO: needs refactoring.. do not add skipped_function names (printf) here....
    runQuery(query);
    if ((row = mysql_fetch_row(result)) != NULL) {
        mysql_free_result(result);
        return true;
    }
    mysql_free_result(result);
    return false;
}

HLStatement* DataAccess::getCallStateHLStatement(int stateId) {
    std::string query = "select id from HLStatement where id in(";
    query += "select HLStatement_id from IRInstr where id in(";
    query += "select IRInstr_id from IRState where startStateId = ";
    query += IntToString(stateId);
    query += "));";
    runQuery(query);
    if ((row = mysql_fetch_row(result)) != NULL) {
        int id = atoi(row[0]);
        mysql_free_result(result);
        return HLIdsToStatements[id];
    }
    mysql_free_result(result);
    return NULL;
}

int DataAccess::getCallStateLineNumber(int stateId) {
    HLStatement* hl = getCallStateHLStatement(stateId);
    if (hl != NULL)
        return hl->line_number;
    return -1;
}

//assuming that there is only one function call in each line number
bool DataAccess::StateRelatedToHLStatement(int stateId, int lineNumber) {
    std::string query = "select line_number from HLStatement where id in (";
    query += "select HLStatement_id from IRInstr where id in(";
    query += "select IRInstr_id from IRState where (StartStateId <= ";
    query += IntToString(stateId);
    query += " AND EndStateId >= ";
    query += IntToString(stateId);
    query += ")))";
    runQuery(query);
    while ((row = mysql_fetch_row(result)) != NULL) {
        if (atoi(row[0]) == lineNumber) {
            mysql_free_result(result);
            return true;
        }
    }
    mysql_free_result(result);
    return false;
}
