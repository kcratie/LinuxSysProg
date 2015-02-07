#include "AsyncWorkQueue.h"

AsyncWorkQueue::AsyncWorkQueue() : mExit(false) {
    if(pthread_mutex_init(&InstrQueLock, NULL))
        throw logic_error("failed to initialize Instruction Queue mutex");

    if(pthread_cond_init(&InstrQueCond, NULL))
        throw logic_error("failed to initialize Instruction Queue conditional");

}

AsyncWorkQueue::~AsyncWorkQueue() {
	// TODO Auto-generated destructor stub
}


void AsyncWorkQueue::SetExitEvent(){
	pthread_mutex_lock(&ExitEvLock);
	mExit = true;
	pthread_mutex_unlock(&ExitEvLock);
}

bool AsyncWorkQueue::ExitEvent(){
	pthread_mutex_lock(&ExitEvLock);
	bool ex = mExit;
	pthread_mutex_unlock(&ExitEvLock);
	return ex;
}

void AsyncWorkQueue::AddInstruction(Instruction * Instr){
    pthread_mutex_lock(&InstrQueLock);
    bool mustSignal = InstrQue.empty();
    InstrQue.push(Instr);
    if (mustSignal)
    	pthread_cond_signal(&InstrQueCond);
    pthread_mutex_unlock(&InstrQueLock);
    return;
}

Instruction * AsyncWorkQueue::GetNextItem(){
    //pull from instruction queue
	pthread_mutex_lock(&InstrQueLock);
	Instruction * instr = NULL;
	while (InstrQue.empty() && !ExitEvent())
		pthread_cond_wait(&InstrQueCond, &InstrQueLock);

	if (!InstrQue.empty()){
		instr = InstrQue.front();
    	InstrQue.pop();
	}
    pthread_mutex_unlock(&InstrQueLock);
    if (!InstrQue.empty() && ExitEvent() && !instr)
    	throw logic_error("Terminating while instructions in queue");
    return instr;
}
