#ifndef IOMAN_H
#define IOMAN_H

#include <cstdlib>

using namespace std;
#include "DataBlk.h"

class IOMan {
public:
	IOMan(int InFileDesc, int OutFileDesc,
			const size_t BlkSize,
			BlkBuffer_t & CachedBlks):
				InFileDesc(InFileDesc), OutFileDesc(OutFileDesc),
				BlockSz(BlkSize), CachedBlks(CachedBlks)
	{}
	unsigned int GetBlkSize() {return BlockSz;}

	DataBlk & GetBlk(
		const unsigned int BlkNumber);

	void PutBlk(
			DataBlk const & Blk);
	void SyncBlks();
private:
	int InFileDesc;
	int OutFileDesc;
	const size_t BlockSz;
	BlkBuffer_t & CachedBlks;

};


#endif // IOMAN_H
