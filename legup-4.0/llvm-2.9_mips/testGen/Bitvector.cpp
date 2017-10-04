/*
 * Bitvector.cpp
 *
 *  Created on: 2012-08-09
 *      Author: fire
 */

#include "Bitvector.h"

Bitvector::Bitvector(int size) {
	int ele = 0;
	for(int i=0;i<size; i++){
		bv.push_back(ele);
	}
}
void Bitvector::setAllOnes(){
	for(unsigned i;i<bv.size(); i++){
		bv.at(i) = 1;
	}
}

void Bitvector::setBit(int pos, char val){
	bv.at(pos) = val;
}
char Bitvector::getBit(int pos){
	return bv.at(pos);
}
int Bitvector::getSize(){
	return bv.size();
}

//This is taking Union
Bitvector* Bitvector::BVor(Bitvector* bv_0, Bitvector* bv_1){
	if(bv_0->getSize()!=bv_1->getSize()){
		printf("ERROR!! 2 bv are having different sizes\n");
		return NULL;
	}
	int bv_size = bv_0->getSize();
	Bitvector* result_bv = new Bitvector(bv_size);

	for(int i;i<bv_size; i++){
		if(bv_0->getBit(i)==1 || bv_1->getBit(i)==1){
			result_bv->setBit(i,1);
		}
	}

	return result_bv;
}

//This is taking Intersection
Bitvector* Bitvector::BVand(Bitvector* bv_0, Bitvector* bv_1){
	if(bv_0->getSize()!=bv_1->getSize()){
		printf("ERROR!! 2 bv are having different sizes\n");
		return NULL;
	}
	int bv_size = bv_0->getSize();
	Bitvector* result_bv = new Bitvector(bv_size);

	for(int i=0;i<bv_size; i++){
		if(bv_0->getBit(i)==1 && bv_1->getBit(i)==1){
			result_bv->setBit(i,1);
		}
	}

	return result_bv;
}
void Bitvector::BVcopy(Bitvector* tar_bv){
	for(unsigned i;i<bv.size(); i++){
		bv.at(i) = tar_bv->getBit(i);
	}
}
int Bitvector::BVsame(Bitvector* tar_bv){
	int same =1;
	for(unsigned i;i<bv.size(); i++){
		if(bv.at(i) != tar_bv->getBit(i)){
			same = 0;
			break;
		}
	}

	return same;
}

void Bitvector:: printBV(){
	for(unsigned i;i<bv.size(); i++){
		printf("%d", bv.at(i));
	}
	printf("\n");
}
