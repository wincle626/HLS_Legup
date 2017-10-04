/* 
 * File:   GDBWrapper.cpp
 * Author: nazanin
 * GDBWrapper class is a wrapper class providing the gdb functionalities to the program
 * Created on January 23, 2014, 10:32 PM
 */

#include <vector>
#include "GDBWrapper.h"
#include "Function.h"
#include "Variable.h"
#include "Globals.h"

GDBWrapper::GDBWrapper() {
    this->currentGDBLine = -1;
    this->programExited = false;
    this->programStarted = false;
    
    this->prevStackFrameDepth = this->currStackFrameDepth = 0;
    this->currCallerLineNumber = this->prevCallerLineNumber = this->callerLineNumber = -1;
    this->isReturning = false;
    
    this->child_vt = NULL;       
}

GDBWrapper::~GDBWrapper() {
    
}

/* call back methods */
void cb_console(const char *str, void *data)
{
    //printf("CONSOLE> %s\n",str);
}
/* Note that unlike what's documented in gdb docs it isn't usable. */
void cb_target(const char *str, void *data)
{
    //printf("TARGET> %s\n",str);
}
void cb_log(const char *str, void *data)
{
    //printf("LOG> %s\n",str);
}
void cb_to(const char *str, void *data)
{
    //printf(">> %s",str); 
}
void cb_from(const char *str, void *data)
{
    //printf("<< %s\n",str); 
}
volatile int async_c;
void cb_async(mi_output *o, void *data)
{
    //printf("ASYNC\n");
    async_c++;
    if (o != NULL) {
        mi_frames* frames = mi_get_async_frame(o);
        if (frames != NULL) {
            gdbWrapper->previousGDBLine = gdbWrapper->currentGDBLine;
            gdbWrapper->currentGDBLine = frames->line;
            //gdbWrapper->frames = frames;                        
            
            /*std::cout << "function: " << frames->func << std::endl;

            std::cout << "level: " << frames->level << std::endl;
            
            gdbWrapper->currentFunctionName = frames->func;
            if (frames->next != NULL)
                std::cout << "caller line: " << frames->next->line << std::endl;*/
        }
        
        char** stop_reason = new char*[10];
        for (int i = 0 ; i < 10; i++)
            stop_reason[i] = new char[100];
            
        mi_get_async_stop_reason(o, stop_reason);
        if (stop_reason != NULL) {
                //std::cout << "REASON : " << *stop_reason << std::endl;
                std::string stop_reason_str(*stop_reason);
                if (stop_reason_str.find("exited") != -1)
                    gdbWrapper->programExited = true;
        }
        //TODO: potential memory leak? check later.
        delete[] stop_reason;
    }
}
int wait_for_stop(mi_h *h)
{
    int res=1;
    mi_stop *sr;

    while (!mi_get_response(h))
       usleep(1000);
    /* The end of the async. */
    sr=mi_res_stop(h);
    if (sr)
      {
       //printf("Stopped, reason: %s\n",mi_reason_enum_to_str(sr->reason));
       mi_free_stop(sr);
      }
    else
      {
       //printf("Error while waiting\n");
       res=0;
      }
    return res;
}
/* end of call back methods*/

void GDBWrapper::initialize() {
    h = mi_connect_local();
    if (!h) {
        std::cout << "GDB connect failed!" << std::endl;
        return;
    }
    std::cout << "connected to GDB!" << std::endl;
    
    mi_set_console_cb(h,cb_console,NULL);
    mi_set_target_cb(h,cb_target,NULL);
    mi_set_log_cb(h,cb_log,NULL);
    mi_set_async_cb(h,cb_async,NULL);
    mi_set_to_gdb_cb(h,cb_to,NULL);
    mi_set_from_gdb_cb(h,cb_from,NULL);    
    
    child_vt=gmi_look_for_free_vt();
    if (!child_vt)
        printf("Error opening auxiliar terminal, we'll use current one.\n");
    else {
        printf("Free VT @ %s\n",child_vt->tty);
        printf("\n\n***************************************\n");
        printf("Switch to the above mentioned terminal!\n");
        printf("***************************************\n\n\n");
    }
    
     /* Tell gdb to attach the child to a terminal. */
    if (!gmi_target_terminal(h,child_vt ? child_vt->tty : ttyname(STDIN_FILENO)))
    {
        printf("Error selecting target terminal\n");
        mi_disconnect(h);
        return;
    }

    /* Set the name of the child and the command line aguments. */
    if (!gmi_set_exec(h,(workDir + SWBinaryFilename).c_str(),""))
    {
        printf("Error setting exec y args\n");
        mi_disconnect(h);
        return;
    }
    
    gmi_gdb_set(h, "print repeats", "100000");
    
    gmi_gdb_set(h, "print elements", "100000");
        
    /* Set a breakpoint at the first line of the main function. */
    /*mi_bkpt *bk;
    bk=gmi_break_insert(h,codeFilename.c_str(),18);
    if (!bk)
    {
        printf("Error setting breakpoint\n");
        mi_disconnect(h);
        return;
    }
    printf("Breakpoint %d @ function: %s\n",bk->number,bk->func);
        
    if (!gmi_exec_run(h))
    {
        printf("Error in run!\n");
        mi_disconnect(h);
        return;
    }
    
     // Here we should be stopped at the breakpoint.
    if (!wait_for_stop(h))
    {
        mi_disconnect(h);
        return;
    }*/        
}

std::string GDBWrapper::examinePointerToArray(std::string varName, int elemCount) {
    if (varName[0] == '%')
        return "N/A";
    char* value = gmi_data_evaluate_expression(h, varName.c_str());
    if (value != NULL) {        
        std::string ret;
        ret = ret.append(value);
        free(value);
        return ret;
    }
    return "N/A";
}

std::string GDBWrapper::examineGlobalVariable(std::string varName, Type varType, int elemCount) {
    if (varName[0] == '%')
        return "N/A";
    varName = normalizeVariableNameForGDB(varName);
    char* value = gmi_data_evaluate_expression(h, ("'" + codeFilename + "'" + "::" + varName).c_str());    
    if (value == NULL)
        return "N/A";
    std::string ret;
    ret = ret.append(value);
    free(value);
    return ret;
}

std::string GDBWrapper::examineVariable(std::string varName, Type varType, int elemCount) {
    if (varName[0] == '%')
        return "N/A";
    varName = normalizeVariableNameForGDB(varName);
    char* value = gmi_data_evaluate_expression(h, varName.c_str());    
    
    if (value == NULL)
        return "N/A";
    
    /*if (strlen(value) > 2 && value[0] == '0' && value[1] == 'x') {
        if (varType == ARRAY)
            value = gmi_data_evaluate_expression(h, ("*" + varName + "@" + IntToString(elemCount)).c_str());
        else
            value = gmi_data_evaluate_expression(h, ("*" + varName).c_str());
    }
    
    if (value == NULL)
        return "N/A";*/
    
    std::string ret;
    ret = ret.append(value);
    free(value);
    return ret;
    
    /*if (value != NULL && strlen(value) > 2) {
        
        if (value[0] == '0' && value[1] == 'x') { //the returned value is address
            if (varType == ARRAY) {
                char *newValue = gmi_data_evaluate_expression(h, ("*" + varName + "@" + IntToString(elemCount)).c_str());
                std::string ret;
                ret = ret.append(newValue);
                free(newValue);
                free(value);
                return ret;
                //return newValue;
            }
        } else  {
            std::string ret;
            ret = ret.append(value);
            free(value);
            return ret;
        }
    }*/       
}

std::string GDBWrapper::normalizeVariableNameForGDB(std::string varName) {
    std::string output = "";
    for (int i = 0; i < varName.size(); i++) {
        if (varName[i] == '.')
            output += "::";
        else
            output += varName[i];
    }
    return output;
}

/*std::string GDBWrapper::examineVariable(std::string varName) {
    return examineVariable(varName, PRIMITIVE_INT, 0);    
}*/

void GDBWrapper::runForFirstTime() {
    
    //set break points at start of all functions...
    for (int i = 1 ; i < functions.size(); i++) {            
        //inserting breakpoint at (line + 1) because we don't want to stop at the function signature line but the first line...
        //this is not accurate. we have to set this line as the first line in the function that we have to stop... finding it from Debug DB
        
        //for now i'm skipping the breakpoint on legup_ functions such as legup_memcpy...
        //this is because basically these functions are in another file and now I'm only supporting one file
        //the other thing is that even if we have the multi-file support, we don't want to debug the legup_ functions... so it has to be handled some how.
        //it is being skipped for now because it causes problems and make the program stop at unwanted lines in main file.
        if (functions[i]->name.find("legup_") == std::string::npos) {
            gmi_break_insert(h, (workDir + codeFilename).c_str(), functions[i]->startLineNumber + 1);
            std::cout << "breakpoint set at line " << functions[i]->startLineNumber + 1 << " function: " << functions[i]->name << std::endl;
        }
    }
    
    if (!gmi_exec_run(h))
    {
        printf("Error in run!\n");
        mi_disconnect(h);
        return;
    }
    
     // Here we should be stopped at the breakpoint.
    if (!wait_for_stop(h))
    {
        mi_disconnect(h);
        return;
    }
    programStarted = true;
    
    //this is because we want to have gdb one step ahead in execution all the times.
    //so the variable examinations is synced with the HW flow.
    //for pure GDB (SW) mode this should be skipped.
    doStepping();
}

void GDBWrapper::checkFunctionReturn() {
    mi_frames* f = gmi_stack_list_frames(h);
    isReturning = false;
    functionNamesToFrameNumbers.clear();
    
    if (!f)
        return;
    
    int level = 0;
    mi_frames* ff = f;
    while (ff) {
        level = ff->level;
        ff = ff->next;
    }
    
    prevStackFrameDepth = currStackFrameDepth;
    currStackFrameDepth = level;
    
    prevCallerLineNumber = currCallerLineNumber;
    if (level > 0) {        
        currCallerLineNumber = f->next->line;
    }
    
    if (currStackFrameDepth < prevStackFrameDepth) {
        isReturning = true;
        callerLineNumber = prevCallerLineNumber;
    }
            
    while (f) {
        functionNamesToFrameNumbers[f->func] = f->level;
        f = f->next;
    }
    
    /*stackFrameFile.open((workDir + "stackFrameLog.txt").c_str(), std::ios::app);
    while (f) {        
        stackFrameFile << "Level " << f->level << ", addr " << f->addr << ", func " << f->func << ", where:"
                << f->line << std::endl;
        //printf("Level %d, addr %p, func %s, where: %s:%d args? %c\n",f->level,f->addr,
        //   f->func,f->file,f->line,f->args ? 'y' : 'n');
        f = f->next;
    }
    stackFrameFile << "*******************************************" << std::endl;
    stackFrameFile.flush();
    stackFrameFile.close();*/
}

bool GDBWrapper::changeFrame(std::string fName) {    
    if (functionNamesToFrameNumbers.find(fName) == functionNamesToFrameNumbers.end())
        return false;
    
    int frameNum = functionNamesToFrameNumbers[fName];    
    gmi_stack_select_frame(h, frameNum);
    return true;
    
}

void GDBWrapper::doStepping() {
    
    if (!programStarted) {
        runForFirstTime();        
        return;
    }        
    
    gmi_exec_next(h);
    //gmi_exec_step(h);
    /* Here we should be terminated. */
    if (!wait_for_stop(h))
    {
        mi_disconnect(h);
        return;
    }      
    
    checkFunctionReturn();
}

void GDBWrapper::finalize() {
    /* Exit from gdb. */
    gmi_gdb_exit(h);    
    /* Close the connection. */
    mi_disconnect(h);
    gmi_end_aux_term(child_vt);
}

void GDBWrapper::test_gdb() {        
    
    return;
    
    /* Continue execution. */
    if (!gmi_exec_continue(h))
    {
        printf("Error in continue!\n");
        mi_disconnect(h);
        return;
    }
    /* Here we should be terminated. */    
    if (!wait_for_stop(h))
    {
        mi_disconnect(h);
        return;
    }   
}
