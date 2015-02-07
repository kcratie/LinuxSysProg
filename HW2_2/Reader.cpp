#include "Reader.h"
#include "Processor.h"

Reader::Reader(IOMan & IO, Processor & Pcr) : mIO(IO), mPcr(Pcr)
{}

Reader::~Reader()
{}

/*
void Reader::AddInstruction(Instruction * Instr){
    pthread_mutex_lock(&InstrQueLock);
    InstrQue.push(Instr);
    pthread_cond_signal(&InstrQueCond);
    pthread_mutex_unlock(&InstrQueLock);
    return;
}
*/
void Reader::Execute(){
	while (true){
		//pull from instruction queue
		Instruction * instr = GetNextItem();
		if (!instr){
			mPcr.SetExitEvent();
			break;
		}
		//Get the associate data block
		instr->SetDataBlk( &mIO.GetBlk(instr->GetInputBlkNum()));

		//send the instruction and associated blk to the processor thread
		mPcr.AddInstruction(instr);
	}
	return;

}



