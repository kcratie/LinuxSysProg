#include "IOMan.h"
#include <unistd.h>

DataBlk & IOMan::GetBlk(const unsigned int BlkNumber)
{
	DataBlk & db = CachedBlks[BlkNumber];
	if (db.Size == 0) {	// its not in our buffer so get it from the input data file
		db.Size = BlockSz;
		db.Buffer = new unsigned char[BlockSz];
		ssize_t rcnt = pread(InFileDesc, db.Buffer, BlockSz, BlkNumber*BlockSz);
		if (rcnt == -1){
			string emsg ("Failed to read data block from input file");
			perror(emsg.c_str());
			throw logic_error(emsg);
		}
		else {
			db.Length = rcnt;
			db.SrcBlockNum = BlkNumber;
		}

	}

	return db;
}

void IOMan::PutBlk(DataBlk const & Blk)
{
	int wcnt = pwrite(OutFileDesc, Blk.Buffer, Blk.Length, Blk.DstBlockNum*Blk.Size);
	if (wcnt == -1) throw logic_error("Failed to write data block to output file");
}

/*
 * Copies the Input data to the output file
 */
void IOMan::SyncBlks()
{
	DataBlk blk(BlockSz);
	ssize_t rcnt = pread(InFileDesc, blk.Buffer, BlockSz, blk.SrcBlockNum*BlockSz);
	while (rcnt > 0){
		blk.Length = rcnt;
		int wcnt = pwrite(OutFileDesc, blk.Buffer, blk.Length, blk.SrcBlockNum*BlockSz);
		if (wcnt == -1) throw logic_error("Failed to write data block to output file");
		blk.SrcBlockNum++;
		rcnt = pread(InFileDesc, blk.Buffer, BlockSz, blk.SrcBlockNum*BlockSz);
	}

	return;
}
