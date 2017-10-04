/* 
 * File:   OnChipSignal.h
 * Author: nazanin
 *
 * Created on October 25, 2013, 4:35 PM
 */

#ifndef ONCHIPSIGNAL_H
#define	ONCHIPSIGNAL_H

#include <string>
#include <vector>
#include <map>

class OnChipSignal {
public:
    OnChipSignal(int id, std::string name);
    OnChipSignal(int id, std::string name, std::vector<int> values);
    OnChipSignal(const OnChipSignal& orig);
    virtual ~OnChipSignal();
    
    int id;
    std::string name;
    std::vector<int> values;
    
private:        

};

#endif	/* ONCHIPSIGNAL_H */

