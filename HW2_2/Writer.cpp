#include "Writer.h"

Writer::Writer(IOMan & IO) : mIO(IO)
{}

Writer::~Writer()
{}

void Writer::Execute(){
	while (true){
		//pull from instruction queue
		Instruction * instr = GetNextItem();
		if (!instr)
			break;
		//Write bock to output file
		mIO.PutBlk(*instr->GetDataBlk());
	}
    return;
}
