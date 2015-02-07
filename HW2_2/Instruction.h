#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "DataBlk.h"

class Instruction{
public:
	Instruction(const unsigned int InputBlk, const unsigned int OutBlk, const unsigned TargetBitMask);
	virtual ~Instruction();
	virtual void Op(/*DataBlk & Blk*/) = 0;
	size_t GetInputBlkNum() {return InputBlockNum;}
	size_t GetOutputBlkNum() {return OutputBlockNum;}
	unsigned GetBits() {return Bits;}
	void SetDataBlk(DataBlk * Blk);
	const DataBlk * GetDataBlk();
protected:
	size_t InputBlockNum;
	size_t OutputBlockNum;
	unsigned char Bits;
	DataBlk * Data;
};


//-----------------------------------------------------------------------------------------------------------
class RevertInstruction : public virtual Instruction {
public:
	RevertInstruction(const unsigned int InputBlk, const unsigned int OutBlk, const unsigned TargetBitMask);
	~RevertInstruction();
	void Op(/*DataBlk & Blk*/);
};


//-----------------------------------------------------------------------------------------------------------
class ZeroInstruction : public virtual Instruction {
public:
	ZeroInstruction(const unsigned int InputBlk, const unsigned int OutBlk, const unsigned TargetBitMask);
	~ZeroInstruction();
	void Op(/*DataBlk & Blk*/);
};


#endif // INSTRUCTION_H
