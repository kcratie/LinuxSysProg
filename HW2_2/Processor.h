#ifndef PROCESSOR_H
#define PROCESSOR_H

#include"Thread.h"
#include"Instruction.h"
#include "AsyncWorkQueue.h"
#include "Writer.h"

class Processor : public virtual Thread, public virtual AsyncWorkQueue
{
    public:
        Processor(Writer & Wrt);
        virtual ~Processor();
        void Execute();
    protected:
    private:
        Writer & mWrt;
};

#endif // PROCESSOR_H
