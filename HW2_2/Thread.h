#ifndef THREAD_H
#define THREAD_H

#include <cstdlib>
#include <pthread.h>

class Thread
{
    public:
        Thread();
        virtual ~Thread();
        void Start();
        void Join();

    protected:
        pthread_t Handle;
//        int Run();
        virtual void Execute() = 0;
        static void * EntryPoint(void*);
};

#endif // THREAD_H

