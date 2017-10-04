/* 
 * File:   ModelsimStarter.cpp
 * Original Author: nazanin
 *
 * Created on Aug 15, 2014
 */

#include "ModelsimStarter.h"
#include "Globals.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

std::string GetStdoutFromCommand(std::string cmd) {

    std::string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1"); // Do we want STDERR?

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

void* RunModelSim(void* threadId) {
    std::string remoteFileLoadCommand = "\"source " + modelsimListenerFilename +"\"";
    //system((vsimDir+vsimRunCommand+remoteFileLoadCommand).c_str());
    std::string output = GetStdoutFromCommand((vsimDir+vsimRunCommand+remoteFileLoadCommand).c_str());
        
    //when we're here, the modelsim is finished....
    //process the modelsim console output to find the return_val signal value...
    
    //first, get rid of the very large string...the value is at the end of the output
    std::string endOfOutput = output.substr(output.size() - MIN(output.size(), 500));
    //find the position of return_val term
    int returnValPos = endOfOutput.find("return_val=");
    std::string value = endOfOutput.substr(returnValPos + 11, 50);
    std::vector<std::string> splits = split(value, ' ');
    std::string finalVal;
    for (int i = 0 ; i < splits.size(); i++)
        if (splits[i] != "") {
            finalVal = splits[i];
            break;
        }
    finalVal = trimMessage(finalVal);
    simulationMainReturnVal = finalVal;
        
    pthread_exit(NULL);
}

void StartModelsim() {
#ifdef PYTHON_WRAPPER
	setFileNames();
#endif
    pthread_t modelSimThread;
    int rc = pthread_create(&modelSimThread, NULL, RunModelSim, NULL);
}

#ifdef PYTHON_WRAPPER

#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(ModelsimStarter)
{
	def("StartModelsim", StartModelsim);
}
#endif
