/*
 * AsyncWorkQueue.h
 *
 *  Created on: Feb 6, 2015
 *      Author: kcratie
 */

#ifndef ASYNCWORKQUEUE_H_
#define ASYNCWORKQUEUE_H_

#include <cstdlib>
#include <queue>
using namespace std;
#include"DataBlk.h"
#include"Instruction.h"

class AsyncWorkQueue {
public:
	AsyncWorkQueue();
	virtual ~AsyncWorkQueue();
	void AddInstruction(Instruction * Instr);
	void SetExitEvent();
protected:
    queue<Instruction*> InstrQue;
    pthread_mutex_t InstrQueLock;
    pthread_mutex_t ExitEvLock;
    pthread_cond_t InstrQueCond;
    Instruction * GetNextItem();
    bool mExit;
    bool ExitEvent();
};

#endif /* ASYNCWORKQUEUE_H_ */
