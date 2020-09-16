/*
 * Decoupled.h
 *
 *  Created on: 22-Jun-2020
 *      Author: Ria
 */

#ifndef DECOUPLED_H_
#define DECOUPLED_H_
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include <math.h>

class Decoupled
{
public:
	typedef struct
	{
		int selection;
		int valid;
		int dirty;
	} data_array;
	typedef struct
		{
			int tag;
		} tag_array;
	Decoupled(int in_BlockSize, int in_L2size,int L2_assoc, int L2_datablocks, int addresstags);
	void read( long int address);
	void write( long int address);
	void printOutput2();
	void evict();
	virtual ~Decoupled();
	int BlockSize ;
	int Size_L2 ;
	int Associativity;
	int Datablocks_L2 ;
	int Addresstags_L2 ;


private:
	int set_no;
	int Offset;
	int Index;
	int Number_of_Tags;
	int Bits_of_Data;
	int Tag;
	int Mask_index;
	int datasel_mask;
	int tagsel_mask;
	tag_array **TM;
	data_array **DM;
	int tag_address;
    int tagsel, datasel;
    int index_is;
    int read_miss;
    int write_miss;
    int no_read;
    int writes;
    int CacheBlockMiss;
    int SectorMiss;
    int writeback;


};

#endif /* DECOUPLED_H_ */
