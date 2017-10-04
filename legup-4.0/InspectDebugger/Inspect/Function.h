/* 
 * File:   Function.h
 * Author: nazanin
 *
 * Created on August 15, 2013, 10:02 AM
 */

#ifndef FUNCTION_H
#define	FUNCTION_H

#include <string>

class Function {
public:
    Function(int id, std::string name, int startLineNumber);
    virtual ~Function();
    
    int id;
    std::string name;
    int currentState;//local currentstate showing the running state in the function
    int currentStateId;
    int startLineNumber;
private:

};

#endif	/* FUNCTION_H */

