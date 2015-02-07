//============================================================================
// Name        : HW2_2.cpp
// Author      : ken subratie
// Version     :
// Copyright   : None
// Description : Home work #2 Single threaded implementation
//============================================================================

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
using namespace std;
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include"DataBlk.h"
#include"Instruction.h"
#include"IOMan.h"
#include "Thread.h"
#include"Reader.h"
#include"Writer.h"
#include"Processor.h"




//const unsigned int BIT0 = 0x1U;
//const unsigned int BIT1 = 0x1U<<1;
//const unsigned int BIT2 = 0x1U<<2;
//const unsigned int BIT3 = 0x1U<<3;
//const unsigned int BIT4 = 0x1U<<4;
//const unsigned int BIT5 = 0x1U<<5;
//const unsigned int BIT6 = 0x1U<<6;
//const unsigned int BIT7 = 0x1U<<7;







//-----------------------------------------------------------------------------------------------------------
class HW2_2{
public:
    HW2_2();
    ~HW2_2();
    void ParseArgs(int argc, char ** argv);
    bool MoreInstructions();
    Instruction * GetNextInstruction();

    size_t OutputFD() {return Outputfd;}
    size_t InputFD() {return Inputfd;}
    size_t BlockSize() {return BlockSz;}
    size_t BufferSize() {return BuffSz;}
    BlkBuffer_t Cache;

private:
    Instruction * Decode(const string & InstrStr);
    fstream Instfs;
    int Inputfd;
	int Outputfd;
	size_t BlockSz;
    size_t BuffSz;
    const int MAX_READLINE = 256;
    char *sbuf;

};

HW2_2::HW2_2() : Inputfd(-1), Outputfd(-1), BlockSz(0), BuffSz(0), sbuf(NULL)
{
    sbuf = new char[MAX_READLINE];
}

 HW2_2::~HW2_2(){
    Instfs.close();
 	close(Inputfd);
	close(Outputfd);
	delete sbuf;
 }

Instruction * HW2_2::GetNextInstruction(){

	Instfs.getline(sbuf, MAX_READLINE);
    string strbuf(sbuf);
    strbuf.append(" ");

    if (Instfs.good()){
        return Decode(strbuf);
    }
	else{
		perror("instruction file stream bad");
	}
	return Decode(strbuf);
}

bool HW2_2::MoreInstructions(){

    Instfs.peek();
    return !Instfs.eof();
}

void
HW2_2::ParseArgs(
    int argc,
    char ** argv)
{

		if (argc < 6)
			cout<< "Usage: hw2.1 instructionFileName inputDataFileName outputDataFilename blockSize bufferSize" << endl;

		BlockSz = atoi(argv[4]);	//in bytes
		BuffSz = atoi(argv[5]);


		Instfs.open(argv[1], fstream::in);
		if (!Instfs.good()){
			throw logic_error("Failed to open input instruction file");
		}

		Inputfd = open(argv[2], O_RDONLY);
		if (Inputfd == -1){
			throw logic_error("Failed to open data file");
		}

		Outputfd = open(argv[3], O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRWXG);
		if (Outputfd == -1){
			throw logic_error("Failed to open output file");
		}

}
//HW2_2 * CreateHW2_2(){
//    return new HW2_2;
//}

Instruction* HW2_2::Decode(const string & InstrStr){
	Instruction * inst = NULL;
	int icount = 1, opcode = 0, inblk = 0, outblk = 0;
	size_t bp = 0, ep = InstrStr.find(" ");
	unsigned bits = 0;
	while (ep != string::npos){
		string op = InstrStr.substr(bp, ep);
		if (icount == 1 && op.compare("revert")==0){

			opcode = 1;
		}
		else if (icount == 1 && op.compare("zero")==0){
			opcode = 2;
		}
		else if (icount == 2){
			inblk = atoi(op.c_str());
		}
		else if (icount == 3){
			outblk = atoi(op.c_str());
		}
		else if (icount >= 4 ){
			unsigned x = atoi(op.c_str());
			if (x > 7) throw logic_error("bit value out of range");
			bits |= 1 << x;
		}
		bp = ep + 1;
		ep = InstrStr.find(" ", bp);
		icount++;
	}
	switch (opcode)
	{
	case 1:
		inst = new RevertInstruction(inblk, outblk, bits);
		break;
	case 2:
		inst = new ZeroInstruction(inblk, outblk, bits);
		break;
	case 0:
	default:
		throw logic_error("Invalid instruction encountered");
		break;
	}

	return inst;
}
//-----------------------------------------------------------------------------------------------------------


int main(int argc, char ** argv) {
	unique_ptr<Reader> readerThread;
	unique_ptr<Writer> writerThread;
	unique_ptr<Processor> processorThread;
	HW2_2 hw22;
	try{
		BlkBuffer_t RecentBlocks;

        hw22.ParseArgs(argc, argv);

        IOMan io(hw22.InputFD(), hw22.OutputFD(), hw22.BlockSize(), hw22.Cache);
        io.SyncBlks();

        writerThread.reset(new Writer(io));
        processorThread.reset(new Processor(*writerThread.get()));
        readerThread.reset(new Reader(io, *processorThread.get()));

        readerThread->Start();
        processorThread->Start();
        writerThread->Start();

        while (hw22.MoreInstructions()){
        	readerThread->AddInstruction(hw22.GetNextInstruction());
        }
        readerThread->SetExitEvent();
	}
	catch (exception & e){
		perror(e.what());
	}
	readerThread->Join();
	processorThread->Join();
	writerThread->Join();
	return 0;
}


