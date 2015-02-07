#include "Thread.h"
#include <cstdlib>
#include <stdexcept>
using namespace std;

Thread::Thread()
{
    //ctor
}


Thread::~Thread()
{
    //dtor
}

void Thread::Start(){
	int failed = pthread_create(&Handle, NULL,
                    (void* (*)(void*))Thread::EntryPoint,
                    static_cast<void*>(this));

    if (failed) {
      throw logic_error("thread_create failed!");
   }

	return;
}
/*
int Thread::Run(){
	return Execute();
}
*/
void * Thread::EntryPoint(void * pThis){
	Thread * pt = (Thread*)pThis;
	pt->Execute();
	pthread_exit(NULL);
	return NULL;
}


void Thread::Join(){
    int failed = pthread_join(Handle, NULL);
    if (failed)
        throw logic_error("pthread_join failed");
    return;
}
