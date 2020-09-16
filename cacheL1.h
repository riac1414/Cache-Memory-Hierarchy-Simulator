/*
 * cacheL1.h
 *
 *  Created on: 23-Apr-2020
 *      Author: Ria
 */

#ifndef CACHEL1_H_
#define CACHEL1_H_
#include "Decoupled.h"
class cacheL1
{
public:
	typedef struct
	{
		long int tag;
		int valid;
		int LRU;
		int dirty;
	} cache_array;

	cacheL1(int L1blocksize, int L1associativity, int L1size);
	void seggregate(long int address, char operation);
	void read(Decoupled *p);
	void readl2(long int address);
	void writel2(long int address);
	void write(Decoupled *p);
	void readDummy(cacheL1 cacheL1);
	void printOutput();
	virtual ~cacheL1();

private:
	int set_size;
	int index_bits;
	int number_block_offset_bits;
	int number_tag_bits;
	cache_array **array1;
	long int tag_address;
	unsigned long int address1;
	unsigned long int index_addr;
	int L1associativity;
	int no_readmiss;
	int no_readhit;
	int no_writemiss;
	int no_writehit;
	int no_write;
	int no_read;
	int serialNumber;
	int write_back;
	long int publicAddress;
};
#endif /* CACHEL1_H_ */
