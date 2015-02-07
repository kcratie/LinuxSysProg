#ifndef READER_H
#define READER_H

#include <queue>
using namespace std;
#include"Thread.h"
#include"DataBlk.h"
#include"Instruction.h"
#include "AsyncWorkQueue.h"
#include "Processor.h"

class Reader : public virtual Thread, public virtual AsyncWorkQueue
{
    public:
        Reader(IOMan & IO, Processor & Pcr);
        virtual ~Reader();
        void Execute();
        //void AddInstruction(Instruction * Instr);
    protected:
    private:
    IOMan & mIO;
    Processor & mPcr;
    /*
    queue<Instruction*> InstrQue;
    pthread_mutex_t InstrQueLock;
    pthread_cond_t InstrQueCond;*/

};


#endif // READER_H
