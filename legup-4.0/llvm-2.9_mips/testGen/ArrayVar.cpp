/*
 * ArrayVar.cpp
 *
 *  Created on: 2012-07-31
 *      Author: fire
 */

#include "ArrayVar.h"

ArrayVar::ArrayVar(int inType, int EleNum) {
	this->type = inType;
	this->numNodes = EleNum;
	if(inType){
		this->ptrNode = new Node(2);
	}else{
		this->ptrNode = new Node(3);
	}

}

