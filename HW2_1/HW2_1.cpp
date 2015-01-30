//============================================================================
// Name        : HW2_1.cpp
// Author      : ken subratie
// Version     :
// Copyright   : None
// Description : Home work #2 Single threaded implementation
//============================================================================

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
using namespace std;
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



struct DataBlk {
	DataBlk():Size(0), SrcBlockNum(0), Buffer(NULL){}
	DataBlk(size_t Size, size_t BlkNum=0):Size(Size), Length(0), SrcBlockNum(BlkNum), DstBlockNum(BlkNum), Buffer(new unsigned char[Size]){}
	~DataBlk(){delete Buffer;}
	size_t Size;
	size_t Length;
	size_t SrcBlockNum;
	size_t DstBlockNum;
	unsigned char * Buffer;
};

typedef map<int, DataBlk> BlkBuffer_t;

const unsigned int BIT0 = 0x1U;
const unsigned int BIT1 = 0x1U<<1;
const unsigned int BIT2 = 0x1U<<2;
const unsigned int BIT3 = 0x1U<<3;
const unsigned int BIT4 = 0x1U<<4;
const unsigned int BIT5 = 0x1U<<5;
const unsigned int BIT6 = 0x1U<<6;
const unsigned int BIT7 = 0x1U<<7;

//-----------------------------------------------------------------------------------------------------------
class Instruction{
public:
	Instruction(const unsigned int InputBlk, const unsigned int OutBlk, const unsigned TargetBitMask);
	virtual ~Instruction();
	virtual void Op(DataBlk & Blk) = 0;
	size_t GetInputBlkNum() {return InputBlockNum;}
	size_t GetOutputBlkNum() {return OutputBlockNum;}
	unsigned GetBits() {return Bits;}
protected:
	size_t InputBlockNum;
	size_t OutputBlockNum;
	unsigned char Bits;
};

Instruction::Instruction(
		const unsigned int InputBlk,
		const unsigned int OutBlk,
		const unsigned TargetBitMask) :
			InputBlockNum(InputBlk),
			OutputBlockNum(OutBlk),
			Bits(TargetBitMask){}

Instruction::~Instruction(){}

class RevertInstruction : public virtual Instruction {
public:
	RevertInstruction(const unsigned int InputBlk, const unsigned int OutBlk, const unsigned TargetBitMask);
	~RevertInstruction();
	void Op(DataBlk & Blk);
};

RevertInstruction::RevertInstruction(
		const unsigned int InputBlk,
		const unsigned int OutBlk,
		const unsigned TargetBitMask) :
			Instruction(InputBlk, OutBlk, TargetBitMask)
{}
RevertInstruction::~RevertInstruction(){}
void RevertInstruction::Op(DataBlk & Blk)
{
	for (unsigned int i = 0; i < Blk.Size; i++){
		Blk.Buffer[i] ^= Bits;
	}
	Blk.DstBlockNum = OutputBlockNum;
	return;
}
//-----------------------------------------------------------------------------------------------------------
class ZeroInstruction : public virtual Instruction {
public:
	ZeroInstruction(const unsigned int InputBlk, const unsigned int OutBlk, const unsigned TargetBitMask);
	~ZeroInstruction();
	void Op(DataBlk & Blk);
};

ZeroInstruction::ZeroInstruction(
		const unsigned int InputBlk,
		const unsigned int OutBlk,
		const unsigned TargetBitMask) :
			Instruction(InputBlk, OutBlk, TargetBitMask)
{}
ZeroInstruction::~ZeroInstruction(){}
void ZeroInstruction::Op(DataBlk & Blk)
{
	for (unsigned int i = 0; i < Blk.Size; i++){
		Blk.Buffer[i] &= ~Bits;
	}
	Blk.DstBlockNum = OutputBlockNum;
	return;
}


//-----------------------------------------------------------------------------------------------------------
Instruction* DecodeInstrString(const string & InstrStr){
	Instruction * inst = NULL;
	int icount = 1, opcode = 0, inblk = 0, outblk = 0;
	size_t bp = 0, ep = InstrStr.find(" ");
	unsigned bits = 0;
	while (ep != string::npos){
		string op = InstrStr.substr(bp, ep);
		if (icount == 1 && op.compare("revert")==0){

			opcode = 1;
		}
		else if (icount == 1 && op.compare("zero")==0){
			opcode = 2;
		}
		else if (icount == 2){
			inblk = atoi(op.c_str());
		}
		else if (icount == 3){
			outblk = atoi(op.c_str());
		}
		else if (icount >= 4 ){
			unsigned x = atoi(op.c_str());
			if (x > 7) throw logic_error("bit value out of range");
			bits |= 1 << x;
		}
		bp = ep + 1;
		ep = InstrStr.find(" ", bp);
		icount++;
	}
	switch (opcode)
	{
	case 1:
		inst = new RevertInstruction(inblk, outblk, bits);
		break;
	case 2:
		inst = new ZeroInstruction(inblk, outblk, bits);
		break;
	case 0:
	default:
		throw logic_error("Invalid instruction encountered");
		break;
	}

	return inst;
}
//-----------------------------------------------------------------------------------------------------------

class IOMan {
public:
	IOMan(int InFileDesc, int OutFileDesc,
			const unsigned int BlkSize,
			BlkBuffer_t & CachedBlks):
				InFileDesc(InFileDesc), OutFileDesc(OutFileDesc),
				BlockSz(BlkSize), CachedBlks(CachedBlks)
	{}
	unsigned int GetBlkSize() {return BlockSz;}

	DataBlk & GetBlk(
		const unsigned int BlkNumber);

	void PutBlk(
			DataBlk & Blk);
	void SyncBlks();
private:
	int InFileDesc;
	int OutFileDesc;
	const unsigned int BlockSz;
	BlkBuffer_t & CachedBlks;

};

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

void IOMan::PutBlk(DataBlk & Blk)
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
//-----------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------
int main(int argc, char ** argv) {
	int inputfd = -1;
	int outputfd = -1;
	try{
		BlkBuffer_t RecentBlocks;

		if (argc < 6)
			cout<< "Usage: hw2.1 instructionFileName inputDataFileName outputDataFilename blockSize bufferSize" << endl;
		int BlockSize = atoi(argv[4]);	//in bytes
		int BufferSize = atoi(argv[5]);

		fstream instfs;
		instfs.open(argv[1], fstream::in);
		if (!instfs.good()){
			throw logic_error("Failed to open input data file");
		}

		inputfd = open(argv[2], O_RDONLY);
		if (inputfd == -1){
			throw logic_error("Failed to open instruction file");
		}

		outputfd = open(argv[3], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG);
		if (outputfd == -1){
			throw logic_error("Failed to open output file");
		}
		IOMan io(inputfd, outputfd, BlockSize, RecentBlocks);
		io.SyncBlks();
		const int MAX_READLINE = 256;
		char sbuf[MAX_READLINE];
		string strbuf;
		unique_ptr <Instruction> instr;
		while (!instfs.eof()){

			instfs.getline(sbuf, MAX_READLINE);
			strbuf.assign(sbuf).append(" ");
			if (instfs.good())
				instr.reset(DecodeInstrString(strbuf));
			else {
				continue;
			}
			DataBlk & blk =  io.GetBlk(instr->GetInputBlkNum());
			instr->Op(blk);
			io.PutBlk(blk);
		}


	}
	catch (exception & e){
		perror(e.what());
	}

	close(inputfd);
	close(outputfd);
	return 0;
}

/*
Setting a bit
number |= 1 << x;

Clearing a bit
number &= ~(1 << x);

Toggling a bit
number ^= 1 << x;

Checking a bit
bit = number & (1 << x);
or
bit = (number >> x) & 1
*/
