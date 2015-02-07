#include "Processor.h"

Processor::Processor(Writer & Wrt) : mWrt(Wrt)
{}

Processor::~Processor()
{}

void Processor::Execute(){
	while (true){
		//pull from instruction queue
		Instruction * instr = GetNextItem();
		if (!instr){
			mWrt.SetExitEvent();
			break;
		}
		//Run the instruction on its data block
		instr->Op();
		//send the instruction and associated block to the writer thread
		mWrt.AddInstruction(instr);
	}
    return;
}
