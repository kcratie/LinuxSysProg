#include "Instruction.h"
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

Instruction::Instruction(
		const unsigned int InputBlk,
		const unsigned int OutBlk,
		const unsigned TargetBitMask) :
			InputBlockNum(InputBlk),
			OutputBlockNum(OutBlk),
			Bits(TargetBitMask),
			Data(NULL){}

Instruction::~Instruction()
{
	//delete Data;
	perror("Debug:Instruction dtor");
}

void Instruction::SetDataBlk(DataBlk * Blk){
	if (Data !=NULL)
		throw logic_error("data block already assigned");
	Data = Blk;
}

const DataBlk * Instruction::GetDataBlk(){
	return Data;
}


RevertInstruction::RevertInstruction(
		const unsigned int InputBlk,
		const unsigned int OutBlk,
		const unsigned TargetBitMask) :
			Instruction(InputBlk, OutBlk, TargetBitMask)
{}
RevertInstruction::~RevertInstruction(){}
void RevertInstruction::Op(/*DataBlk & Blk*/)
{
	for (unsigned int i = 0; i < Data->Size; i++){
		Data->Buffer[i] ^= Bits;
	}
	Data->DstBlockNum = OutputBlockNum;
	return;
}




ZeroInstruction::ZeroInstruction(
		const unsigned int InputBlk,
		const unsigned int OutBlk,
		const unsigned TargetBitMask) :
			Instruction(InputBlk, OutBlk, TargetBitMask)
{}
ZeroInstruction::~ZeroInstruction(){}
void ZeroInstruction::Op(/*DataBlk & Blk*/)
{
	for (unsigned int i = 0; i < Data->Size; i++){
		Data->Buffer[i] &= ~Bits;
	}
	Data->DstBlockNum = OutputBlockNum;
	return;
}
