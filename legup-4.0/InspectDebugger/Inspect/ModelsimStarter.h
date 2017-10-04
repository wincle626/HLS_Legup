/* 
 * File:   ModelsimStarter.h
 * Author: nazanin
 *
 * Created on July 6, 2014, 11:48 PM
 */

#ifndef MODELSIMSTARTER_H
#define	MODELSIMSTARTER_H

#include <pthread.h>
#include "Utility.h"
/*
extern std::string modelsimListenerFilename;
extern std::string vsimDir;
extern std::string vsimRunCommand;
*/

std::string GetStdoutFromCommand(std::string cmd);

void* RunModelSim(void* threadId);

void StartModelsim();

#endif	/* MODELSIMSTARTER_H */
