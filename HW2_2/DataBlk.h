#ifndef DATABLK_H
#define DATABLK_H

#include <cstdlib>
#include <map>
using namespace std;

struct DataBlk {
	DataBlk():Size(0), SrcBlockNum(0), Buffer(NULL){}
	DataBlk(size_t Size, size_t BlkNum=0):Size(Size), Length(0), SrcBlockNum(BlkNum), DstBlockNum(BlkNum), Buffer(new unsigned char[Size]){}
	~DataBlk(){delete Buffer;}
	size_t Size;
	size_t Length;
	size_t SrcBlockNum;
	size_t DstBlockNum;
	unsigned char * Buffer;
    DataBlk(const DataBlk& other);
    DataBlk& operator=(const DataBlk& other);
};

typedef map<int, DataBlk> BlkBuffer_t;

#endif // DATABLK_H
