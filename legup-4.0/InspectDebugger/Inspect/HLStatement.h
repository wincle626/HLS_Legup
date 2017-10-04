/* 
 * File:   HLStatement.h
 * Author: nazanin
 *
 * Created on June 27, 2013, 2:21 PM
 */

#ifndef HLSTATEMENT_H
#define	HLSTATEMENT_H

#include <string>
#include "IRInstruction.h"

class HLStatement {
public:
    HLStatement();
    HLStatement(int id, int line_number,int column_number, std::string fileName);
    virtual ~HLStatement();
    
    int id;
    int line_number;
    int start_column_number;
    int end_column_number;
    std::string fileName;
    std::vector<IRInstruction*> IRs;
    
private:
    
    

};

#endif	/* HLSTATEMENT_H */

