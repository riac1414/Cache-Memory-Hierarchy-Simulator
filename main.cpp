//============================================================================
// Name        : trial.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include "cacheL1.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <bits/stdc++.h>
#include<stdio.h>
#include "Decoupled.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 9)
	{
		cout << "Error" << endl;
		return 1;
	}
	ifstream myfile(argv[8]);
	int blocksize = atoi(argv[1]);
	int cachesize = atoi(argv[2]);
	int associtivity = atoi(argv[3]);
	int L2_associtivity = atoi(argv[5]);
	int L2_cachesize = atoi(argv[4]);
	int L2_datablocks =atoi(argv[6]);
	int addresstags = atoi(argv[7]);
	cacheL1 *pL2;
	Decoupled *p;
	cacheL1 cache_1(blocksize, associtivity, cachesize);
	cacheL1 cache_L2(blocksize, L2_associtivity, L2_cachesize);
	Decoupled  desectored( blocksize, L2_cachesize,L2_associtivity ,L2_datablocks, addresstags);
	pL2 = &cache_L2;
	p = &desectored;
	//cout<< blocksize << " " << cachesize <<" " << associtivity;
	string myText;
	long int address;
	char operation;
	while (myfile >> operation >> hex >> address)
	{
		//cout << operation << " " << hex<<address << endl;
		cache_1.seggregate(address, operation);
		//cache_1.read();
		//cache_1.write();
		//cache_L2.seggregate(address, operation);

		if(operation=='r')
		{
			//desectored.read(address);
			cache_1.read(p);
		}
		else
		{
			//desectored.write(address);
			cache_1.write(p);
		}
	}
	//cache_1.readDummy(cache_L2);
	myfile.close();
	cache_1.printOutput();
	desectored.printOutput2();
	//cache_L2.printOutput();
	return 0;
}
