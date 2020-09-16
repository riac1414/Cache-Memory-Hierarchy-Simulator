/*
 * Decoupled.cpp
 *
 *  Created on: 22-Jun-2020
 *      Author: Ria
 */

#include "Decoupled.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include "cacheL1.h"
using namespace std;
Decoupled::Decoupled(int in_BlockSize, int in_L2size, int L2_assoc,
		int L2_datablocks, int addresstags)
{
	BlockSize = in_BlockSize;
	Size_L2 = in_L2size;
	Associativity = L2_assoc;
	Datablocks_L2 = L2_datablocks;
	Addresstags_L2 = addresstags;

	set_no = (Size_L2) / ((BlockSize) * (Associativity) * (Datablocks_L2));
	Offset = log2(BlockSize);
	Index = log2(set_no);
	Number_of_Tags = log2(addresstags);
	Bits_of_Data = log2(L2_datablocks);
	Tag = 32 - (Offset + Index + Number_of_Tags + Bits_of_Data);
//cout << set_no << endl;
//cout << Offset << endl;
//cout << Index << endl;
//cout << Number_of_Tags << endl;
//cout << Bits_of_Data << endl;
//cout << Tag << endl;
	Mask_index = (pow(2, Index)) - 1;
	Mask_index = Mask_index << (Offset + Bits_of_Data);
	datasel_mask = (pow(2, Bits_of_Data)) - 1;
	datasel_mask = datasel_mask << (Offset);
	tagsel_mask = (pow(2, Number_of_Tags)) - 1;
	tagsel_mask = tagsel_mask << (Offset + Bits_of_Data + Index);
	tag_address = 0;
	tagsel = 0;
	datasel = 0;
	index_is = 0;
	no_read = 0;
	read_miss = 0;
	writes = 0;
	writeback = 0;
	write_miss = 0;
	CacheBlockMiss = 0;
	SectorMiss = 0;

	TM = new tag_array*[set_no];
	for (int i = 0; i < set_no; i++)
		TM[i] = new tag_array[Datablocks_L2];

	DM = new data_array*[set_no];
	for (int i = 0; i < set_no; i++)
		DM[i] = new data_array[Datablocks_L2];

	for (int i = 0; i < set_no; i++)
	{
		for (int j = 0; j < Datablocks_L2; j++)
		{
			TM[i][j].tag = 0;
		}
	}
	for (int i = 0; i < set_no; i++)
	{
		for (int j = 0; j < Datablocks_L2; j++)
		{
			DM[i][j].selection = 0;
			DM[i][j].valid = 0;
			DM[i][j].dirty = 0;
		}
	}

}

Decoupled::~Decoupled()
{
	for (int i = 0; i < set_no; i++)
	{
		delete[] TM[i];
	}
	delete[] TM;

	for (int j = 0; j < set_no; j++)
	{
		delete[] DM[j];
	}
	delete[] DM;

}
void Decoupled::evict()
{
	for (int i = 0; i < Datablocks_L2; i++)
	{
		if (DM[index_is][i].selection == tagsel)
		{
			if (DM[index_is][i].dirty == 1)
			{
				writeback++;
				DM[index_is][i].dirty = 0;
			}
			DM[index_is][i].selection = 0;
			DM[index_is][i].valid = 0;
			DM[index_is][i].dirty = 0;
		}

	}
}
void Decoupled::read(long int address)
{
	no_read++;
	cout << hex << address << endl;
	tag_address = address >> (Offset + Bits_of_Data + Index + Number_of_Tags);
	cout << tag_address << endl;
	index_is = address & Mask_index;
	index_is = index_is >> (Offset + Bits_of_Data);
	cout << index_is << endl;
	datasel = datasel_mask & address;
	datasel = datasel >> (Offset);
	cout << datasel << endl;
	tagsel = address & tagsel_mask;
	tagsel = tagsel >> (Offset + Bits_of_Data + Index);
	cout << tagsel << endl;
	bool flag = false;
	bool n_flag = false;
	bool n1_flag = false;

	if (DM[index_is][datasel].valid == 1
			&& TM[index_is][tagsel].tag == tag_address
			&& DM[index_is][datasel].selection == tagsel)
	{
		flag = true;

	}
	if (!flag)
	{
		read_miss++;
		for (int i = 0; i < Datablocks_L2; i++)
		{
			if (DM[index_is][i].valid == 1)
			{
				CacheBlockMiss++;
				n_flag = true;
				break;
			}
		}
		if (!n_flag)
		{
			SectorMiss++;
		}
		if (DM[index_is][datasel].valid == 0
				&& TM[index_is][tagsel].tag == tag_address)
		{
			TM[index_is][tagsel].tag = tag_address;
			DM[index_is][datasel].valid = 1;
			DM[index_is][datasel].selection = tagsel;
			n1_flag = true;
		}
		if (!n1_flag)
		{
			if (!(TM[index_is][tagsel].tag == tag_address))
			{
				evict();
			}
			if (DM[index_is][datasel].dirty == 1)
			{
				writeback++;
				DM[index_is][datasel].dirty = 0;
			}
			TM[index_is][tagsel].tag = tag_address;
			DM[index_is][datasel].valid = 1;
			DM[index_is][datasel].selection = tagsel;
		}
	}
}

void Decoupled::write(long int address)
{
	writes++;
	cout << hex << address << endl;
		tag_address = address >> (Offset + Bits_of_Data + Index + Number_of_Tags);
		cout << tag_address << endl;
		index_is = address & Mask_index;
		index_is = index_is >> (Offset + Bits_of_Data);
		cout << index_is << endl;
		datasel = datasel_mask & address;
		datasel = datasel >> (Offset);
		cout << datasel << endl;
		tagsel = address & tagsel_mask;
		tagsel = tagsel >> (Offset + Bits_of_Data + Index);
		cout << tagsel << endl;
		bool flag = false;
		bool n_flag = false;
		bool n1_flag = false;

		if (DM[index_is][datasel].valid == 1
				&& TM[index_is][tagsel].tag == tag_address
				&& DM[index_is][datasel].selection == tagsel)
		{
			flag = true;
			DM[index_is][datasel].dirty = 1;

		}
		if (!flag)
		{
			write_miss++;
			for (int i = 0; i < Datablocks_L2; i++)
			{
				if (DM[index_is][i].valid == 1)
				{
					CacheBlockMiss++;
					n_flag = true;
					break;
				}
			}
			if (!n_flag)
			{
				SectorMiss++;
			}
			if (DM[index_is][datasel].valid == 0
					&& TM[index_is][tagsel].tag == tag_address)
			{
				TM[index_is][tagsel].tag = tag_address;
				DM[index_is][datasel].valid = 1;
				DM[index_is][datasel].selection = tagsel;
				DM[index_is][datasel].dirty = 1;
				n1_flag = true;
			}
			if (!n1_flag)
			{
				if (!(TM[index_is][tagsel].tag == tag_address))
				{
					evict();
				}
				if (DM[index_is][datasel].dirty == 1)
				{
					writeback++;
					DM[index_is][datasel].dirty = 0;
				}
				TM[index_is][tagsel].tag = tag_address;
				DM[index_is][datasel].valid = 1;
				DM[index_is][datasel].selection = tagsel;
				DM[index_is][datasel].dirty = 1;
			}
		}
}

void Decoupled::printOutput2()
{
	cout << "no of read" << no_read << endl;
	cout << "number of miss" << read_miss << endl;
	cout << "no of write" << writes << endl;
	cout << "number of write miss" << write_miss << endl;
	cout << "write back is " << writeback << endl;
}

