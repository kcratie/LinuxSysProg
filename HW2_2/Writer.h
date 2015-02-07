#ifndef WRITER_H
#define WRITER_H

#include"Thread.h"
#include"IOMan.h"
#include "AsyncWorkQueue.h"

class Writer : public virtual Thread, public virtual AsyncWorkQueue
{
    public:
        Writer(IOMan & IO);
        virtual ~Writer();
        void Execute();
    protected:
    private:
    IOMan & mIO;

};

#endif // WRITER_H
