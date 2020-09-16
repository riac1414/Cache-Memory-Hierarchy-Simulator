/*
 * cacheL1.cpp
 *
 *  Created on: 23-Apr-2020
 *      Author: Ria
 */
#include "Decoupled.h"
#include "cacheL1.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <bits/stdc++.h>
#include<stdio.h>
using namespace std;

cacheL1::cacheL1(int L1blocksize, int L1_assoc, int L1size)
{
	L1associativity = L1_assoc;
	no_readmiss = 0;
	no_readhit = 0;
	no_writemiss = 0;
	no_writehit = 0;
	no_write = 0;
	no_read = 0;
	address1 = 0;
	index_addr = 0;
	serialNumber = 0;
	tag_address = 0;
	write_back = 0;
	publicAddress =0;
	set_size = (L1size) / (L1blocksize * L1associativity);
//cout << set_size << endl;
	index_bits = log2(set_size);
	number_block_offset_bits = log2(L1blocksize);
	number_tag_bits = 32 - index_bits - number_block_offset_bits;
//cout << index_bits << " " <<number_block_offset_bits<<" " << number_tag_bits << endl;
	array1 = new cache_array*[set_size];
	for (int i = 0; i < set_size; i++)
		array1[i] = new cache_array[L1associativity];

	for (int i = 0; i < set_size; i++)
	{
		for (int j = 0; j < L1associativity; j++)
		{
			array1[i][j].tag = 0;           // i=0 tag
			array1[i][j].valid = 0;           //i=1 valid
			array1[i][j].LRU = 0;           //i=2 LRU
			array1[i][j].dirty = 0;           //I=3 DIRTY
		}
	}

}
cacheL1::~cacheL1()
{
}
void cacheL1::seggregate(long int address, char operation)
{
	publicAddress = address;
	serialNumber++;
	cout << dec << serialNumber << endl;
	cout << hex << address << endl;
	tag_address = address >> (number_block_offset_bits + index_bits);
	cout << "TAG::   " << hex << tag_address << endl;
	address1 = address << number_tag_bits;
//cout<< hex << address1 << endl;
	index_addr = address1 >> (number_block_offset_bits + number_tag_bits);
	cout << "INdex ::" << hex << index_addr << endl;
}

void cacheL1::readDummy(cacheL1 cacheL1)
{

}
void cacheL1::read(Decoupled *p) //to get the object access
{
	int LRU_original;
	int original_Jvalue;
	int n_LRU_original;
	int n_original_Jvalue;
	int n1_original_Jvalue;
	bool flag = false;
	bool n_flag = false;
	int biggest;
	long int L1addr;
	long int L1address;
	no_read++;
	for (int j = 0; j < L1associativity; j++) //check for hit
	{
		if (array1[index_addr][j].valid == 1)
		{
			if (array1[index_addr][j].tag == tag_address)
			{
				original_Jvalue = j; //we store the j value so we can use later
				flag = true; // when valid is 1 flag is 1
				break;
			}
		}
	}
	if (flag) // the flag is true
	{
		no_readhit++; // there is a read hit
		//array1[index_addr][original_Jvalue].tag = tag_address;
		//array1[index_addr][original_Jvalue].valid = 1;
		LRU_original = array1[index_addr][original_Jvalue].LRU;
		array1[index_addr][original_Jvalue].LRU = 0;
		for (int j = 0; j < L1associativity; j++)
		{
			if (j != original_Jvalue
					&& array1[index_addr][j].LRU < LRU_original) //LRU should get updated, if hit
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!flag) // when miss and valid is 1
	{
		no_readmiss++;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].valid == 0) //when valid is 0
			{
				n_original_Jvalue = j;
				n_flag = true;
				//no_readmiss++;
				break;
			}
		}
	}
	if (n_flag && !flag) // when valid is 0, and there is a miss update LRU value
	{
		p->read(publicAddress);
		LRU_original = array1[index_addr][n_original_Jvalue].LRU;
		array1[index_addr][n_original_Jvalue].tag = tag_address;
		array1[index_addr][n_original_Jvalue].valid = 1; // setting the valid as 1
		array1[index_addr][n_original_Jvalue].LRU = 0;

		for (int j = 0; j < L1associativity; j++)
		{
			if (n_original_Jvalue != j && array1[index_addr][j].valid == 1)
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!n_flag && !flag) //when valid is 1 and there is a miss
	{
		//no_readmiss++;
		biggest = 0;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].LRU > biggest)
			{
				biggest = array1[index_addr][j].LRU;
				n1_original_Jvalue = j;
			}
		}
		L1addr = array1[index_addr][n1_original_Jvalue].tag;
		L1address = (L1addr
				<< (number_block_offset_bits + index_bits)
						| (index_addr << number_block_offset_bits));

		array1[index_addr][n1_original_Jvalue].tag = tag_address;
		n_LRU_original = array1[index_addr][n1_original_Jvalue].LRU;
		array1[index_addr][n1_original_Jvalue].LRU = 0;
		array1[index_addr][n1_original_Jvalue].valid = 1;
		if (array1[index_addr][n1_original_Jvalue].dirty == 1)
		{
			cout << "write to L2" << endl;
			//pl2->seggregate(L1address, 'w');
			//p->write(L1address);
			p->write(L1address);
			write_back++;
			array1[index_addr][n1_original_Jvalue].dirty = 0; //dirty will be set to 0 as, read
		}
		//pl2->seggregate(
			//	(tag_address
			//			<< (number_block_offset_bits + index_bits)
			//					+ (index_addr << number_block_offset_bits)),
			//	'r');
		p->read(publicAddress);
		for (int j = 0; j < L1associativity; j++)
		{
			if (n1_original_Jvalue != j
					&& n_LRU_original > array1[index_addr][j].LRU)
			{
				array1[index_addr][j].LRU++;
			}
		}
	}
}
void cacheL1::write(Decoupled *p)
{

	int LRU_original;
	int original_Jvalue;
	int n_LRU_original;
	int n_original_Jvalue;
	int n1_original_Jvalue;
	bool flag = false;
	bool n_flag = false;
	int biggest;
	long int L1addr;
	long int L1address;

	no_write++;
	for (int j = 0; j < L1associativity; j++) //check for hit
	{
		if (array1[index_addr][j].valid == 1)
		{
			if (array1[index_addr][j].tag == tag_address)
			{
				original_Jvalue = j; //we store the j value so we can use later
				flag = true; // when valid is 1 flag is 1
				break;
			}
		}
	}
	if (flag) // the flag is true
	{
		no_writehit++; // there is a read hit
		//array1[index_addr][original_Jvalue].tag = tag_address;
		//array1[index_addr][original_Jvalue].valid = 1;
		LRU_original = array1[index_addr][original_Jvalue].LRU;
		array1[index_addr][original_Jvalue].LRU = 0;
		array1[index_addr][original_Jvalue].dirty = 1;
		for (int j = 0; j < L1associativity; j++)
		{
			if (j != original_Jvalue
					&& array1[index_addr][j].LRU < LRU_original) //LRU should get updated, if hit
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!flag) // when miss and valid is 1
	{
		no_writemiss++;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].valid == 0) //when valid is 0
			{
				n_original_Jvalue = j;
				n_flag = true;
				//no_readmiss++;
				break;
			}
		}
	}
	if (n_flag && !flag) // when valid is 0, and there is a miss update LRU value
	{
		p->read(publicAddress);
		LRU_original = array1[index_addr][n_original_Jvalue].LRU;
		array1[index_addr][n_original_Jvalue].tag = tag_address;
		array1[index_addr][n_original_Jvalue].valid = 1; // setting the valid as 1
		array1[index_addr][n_original_Jvalue].LRU = 0;
		array1[index_addr][n_original_Jvalue].dirty = 1;
		for (int j = 0; j < L1associativity; j++)
		{
			if (n_original_Jvalue != j && array1[index_addr][j].valid == 1)
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!n_flag && !flag) //when valid is 1 and there is a miss
	{
		//no_readmiss++;
		biggest = 0;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].LRU > biggest)
			{
				biggest = array1[index_addr][j].LRU;
				n1_original_Jvalue = j;
			}
		}
		L1addr = array1[index_addr][n1_original_Jvalue].tag;
		L1address = (L1addr
				<< (number_block_offset_bits + index_bits)
						| (index_addr << number_block_offset_bits)); //sending the whole address with offset and index bits

		array1[index_addr][n1_original_Jvalue].tag = tag_address;
		n_LRU_original = array1[index_addr][n1_original_Jvalue].LRU;
		array1[index_addr][n1_original_Jvalue].LRU = 0;
		array1[index_addr][n1_original_Jvalue].valid = 1;
		if (array1[index_addr][n1_original_Jvalue].dirty == 1)
		{
			cout << "write to L2" << endl;
			//pl2->seggregate(L1address, 'w');
			p->write(L1address); //write back is done so that, the modified address which is going to be replaced is send to L2 cache, write back the address
			write_back++;
		}
		//pl2->seggregate(
			//	(tag_address
			//			<< (number_block_offset_bits + index_bits)
			//\					+ (index_addr << number_block_offset_bits)),
			//	'r');
		p->read(publicAddress); //again read request so that L2 also updates the address.
		array1[index_addr][n1_original_Jvalue].dirty = 1; //dirty = 1 as write

		for (int j = 0; j < L1associativity; j++)
		{
			if (n1_original_Jvalue != j
					&& n_LRU_original > array1[index_addr][j].LRU)
			{
				array1[index_addr][j].LRU++;
			}
		}
	}
}

void cacheL1::readl2(long int address)
{

	tag_address = address >> (number_block_offset_bits + index_bits);
		cout << "TAG::   " << hex << tag_address << endl;
		address1 = address << number_tag_bits;
	//cout<< hex << address1 << endl;
		index_addr = address1 >> (number_block_offset_bits + number_tag_bits);
		cout << "INdex ::" << hex << index_addr << endl;

	int LRU_original;
	int original_Jvalue;
	int n_LRU_original;
	int n_original_Jvalue;
	int n1_original_Jvalue;
	bool flag = false;
	bool n_flag = false;
	int biggest;
	no_read++;
	for (int j = 0; j < L1associativity; j++) //check for hit
	{
		if (array1[index_addr][j].valid == 1)
		{
			if (array1[index_addr][j].tag == tag_address)
			{
				original_Jvalue = j; //we store the j value so we can use later
				flag = true; // when valid is 1 flag is 1
				break;
			}
		}
	}
	if (flag) // the flag is true
	{
		no_readhit++; // there is a read hit
		//array1[index_addr][original_Jvalue].tag = tag_address;
		//array1[index_addr][original_Jvalue].valid = 1;
		LRU_original = array1[index_addr][original_Jvalue].LRU;
		array1[index_addr][original_Jvalue].LRU = 0;
		for (int j = 0; j < L1associativity; j++)
		{
			if (j != original_Jvalue
					&& array1[index_addr][j].LRU < LRU_original) //LRU should get updated, if hit
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!flag) // when miss and valid is 1
	{
		no_readmiss++;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].valid == 0) //when valid is 0
			{
				n_original_Jvalue = j;
				n_flag = true;
				//no_readmiss++;
				break;
			}
		}
	}
	if (n_flag && !flag) // when valid is 0, and there is a miss update LRU value
	{
		LRU_original = array1[index_addr][n_original_Jvalue].LRU;
		array1[index_addr][n_original_Jvalue].tag = tag_address;
		array1[index_addr][n_original_Jvalue].valid = 1; // setting the valid as 1
		array1[index_addr][n_original_Jvalue].LRU = 0;

		for (int j = 0; j < L1associativity; j++)
		{
			if (n_original_Jvalue != j && array1[index_addr][j].valid == 1)
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!n_flag && !flag) //when valid is 1 and there is a miss
	{
		//no_readmiss++;
		biggest = 0;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].LRU > biggest)
			{
				biggest = array1[index_addr][j].LRU;
				n1_original_Jvalue = j;
			}
		}
		array1[index_addr][n1_original_Jvalue].tag = tag_address;
		n_LRU_original = array1[index_addr][n1_original_Jvalue].LRU;
		array1[index_addr][n1_original_Jvalue].LRU = 0;
		array1[index_addr][n1_original_Jvalue].valid = 1;
		if (array1[index_addr][n1_original_Jvalue].dirty == 1)
		{
			write_back++;
			array1[index_addr][n1_original_Jvalue].dirty = 0; //dirty will be set to 0 as, read
		}

		for (int j = 0; j < L1associativity; j++)
		{
			if (n1_original_Jvalue != j
					&& n_LRU_original > array1[index_addr][j].LRU)
			{
				array1[index_addr][j].LRU++;
			}
		}
	}
}
void cacheL1::writel2(long int address)
{
	tag_address = address >> (number_block_offset_bits + index_bits);
		cout << "TAG::   " << hex << tag_address << endl;
		address1 = address << number_tag_bits;
	//cout<< hex << address1 << endl;
		index_addr = address1 >> (number_block_offset_bits + number_tag_bits);
		cout << "INdex ::" << hex << index_addr << endl;
	int LRU_original;
	int original_Jvalue;
	int n_LRU_original;
	int n_original_Jvalue;
	int n1_original_Jvalue;
	bool flag = false;
	bool n_flag = false;
	int biggest;
	no_write++;
	for (int j = 0; j < L1associativity; j++) //check for hit
	{
		if (array1[index_addr][j].valid == 1)
		{
			if (array1[index_addr][j].tag == tag_address)
			{
				original_Jvalue = j; //we store the j value so we can use later
				flag = true; // when valid is 1 flag is 1
				break;
			}
		}
	}
	if (flag) // the flag is true
	{
		no_writehit++; // there is a read hit
		//array1[index_addr][original_Jvalue].tag = tag_address;
		//array1[index_addr][original_Jvalue].valid = 1;
		LRU_original = array1[index_addr][original_Jvalue].LRU;
		array1[index_addr][original_Jvalue].LRU = 0;
		array1[index_addr][original_Jvalue].dirty = 1;
		for (int j = 0; j < L1associativity; j++)
		{
			if (j != original_Jvalue
					&& array1[index_addr][j].LRU < LRU_original) //LRU should get updated, if hit
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!flag) // when miss and valid is 1
	{
		no_writemiss++;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].valid == 0) //when valid is 0
			{
				n_original_Jvalue = j;
				n_flag = true;
				//no_readmiss++;
				break;
			}
		}
	}
	if (n_flag && !flag) // when valid is 0, and there is a miss update LRU value
	{
		LRU_original = array1[index_addr][n_original_Jvalue].LRU;
		array1[index_addr][n_original_Jvalue].tag = tag_address;
		array1[index_addr][n_original_Jvalue].valid = 1; // setting the valid as 1
		array1[index_addr][n_original_Jvalue].LRU = 0;
		array1[index_addr][n_original_Jvalue].dirty = 1; //
		for (int j = 0; j < L1associativity; j++)
		{
			if (n_original_Jvalue != j && array1[index_addr][j].valid == 1)
			{
				array1[index_addr][j].LRU++;

			}
		}
	}
	if (!n_flag && !flag) //when valid is 1 and there is a miss
	{
		//no_readmiss++;
		biggest = 0;
		for (int j = 0; j < L1associativity; j++)
		{
			if (array1[index_addr][j].LRU > biggest)
			{
				biggest = array1[index_addr][j].LRU;
				n1_original_Jvalue = j;
			}
		}
		array1[index_addr][n1_original_Jvalue].tag = tag_address;
		n_LRU_original = array1[index_addr][n1_original_Jvalue].LRU;
		array1[index_addr][n1_original_Jvalue].LRU = 0;
		array1[index_addr][n1_original_Jvalue].valid = 1;
		if (array1[index_addr][n1_original_Jvalue].dirty == 1)
		{
			write_back++;
		}
		array1[index_addr][n1_original_Jvalue].dirty = 1; //dirty = 1 as write

		for (int j = 0; j < L1associativity; j++)
		{
			if (n1_original_Jvalue != j
					&& n_LRU_original > array1[index_addr][j].LRU)
			{
				array1[index_addr][j].LRU++;
			}
		}
	}
}

void cacheL1::printOutput()
{
	for (int i = 0; i < set_size; i++)
	{
		cout << "Set  " << dec << i << ": " << "\t\t";
		for (int j = 0; j < L1associativity; j++)
		{
			cout << hex << array1[i][j].tag;
			cout << " " << array1[i][j].LRU;
			array1[i][j].dirty == 1 ? cout << " D\t ||" : cout << " N\t ||";
		}
		cout << endl;
	}

	cout << dec << no_read << endl;
	cout << dec << no_readmiss << endl;
	//cout << dec << no_readhit << endl;
	cout << no_write << endl;
	cout << no_writemiss << endl;
	//cout << no_writehit << endl;
	cout << write_back << endl;
}
