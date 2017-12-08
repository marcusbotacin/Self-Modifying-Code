// Self Modifying Code
// Cache simulator
// Marcus Botacin 2017
// Federal University of Paraná (UFPR)

/* Include Block */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include<list>
#include "pin.H"

// define a namespace for Windows.h to avoid conflicts with pin.h
namespace WINDOWS
{
    #include <Windows.h>
}

/ Output file
ofstream OutFile;

// cache entry
typedef struct _cache
{
	VOID *tag;          // address tag
	BOOL valid;         // valid bit
	string *decoded;    // decoded instruction
	UINT64 lru;         // counter for LRU replacement policy
}Cache,*PCache;

// cache size
#define LINE_INDEX 6        // bits to index lines
#define LINES 1<<LINE_INDEX // number of lines
#define ASSOC 16            // associativity

// cache instantiation
Cache cache[LINES][ASSOC];

// LRU counter (clock)
UINT64 counter=0;

// cache access
void check_cache(VOID *PC, string *dis)
{
    // line index from PC address
	UINT64 index = ((UINT64)PC >> LINE_INDEX) && (LINES-1);

    // invalid index
	int invalid_candidate=-1;
    // lru index (default)
	int last_lru =0;
    // first lru check
	UINT64 smallest_lru = cache[index][0].lru;

	for(int j=0;j<LINES;j++)
	{
        // found an invalid entry, mark for future uses
		if(!cache[index][j].valid)
		{
			invalid_candidate=j;
		}

        // found an LRU entry, update and mark for future uses
		if(cache[index][j].lru<smallest_lru)
		{
			smallest_lru=cache[index][j].lru;
			last_lru=j;
		}

        // PC already in cache
		if(cache[index][j].valid && cache[index][j].tag==PC)
		{
            // used: update LRU
			cache[index][j].lru=counter;
            // check decoded data matches
			if(strcmp(dis->c_str(),cache[index][j].decoded->c_str())==0)
			{
                // match
                // debug print
				OutFile<<"OK"<<endl;
			}else{
                // differs
                // debug print
				OutFile<<"SMC"<<endl;
			}
            // found, do not iterate anymore
			return;
		}
	}

    // nothing found, insert into cache

	int insert_into;

    // insert into last_lru position
	insert_into = last_lru;

    // or in an invalid one
	if(invalid_candidate>=0)
	{
		insert_into = invalid_candidate;
	}

    // insert
	cache[index][insert_into].decoded = new string(*dis);
	cache[index][insert_into].lru = counter;
	cache[index][insert_into].tag = PC;
	cache[index][insert_into].valid = true;

    // debug print
	OutFile<<"Inserting"<<endl;

}

// cache clear
// in case of boot or clflush
void clear_cache()
{
    // debug print
	OutFile << "Cache Flush" << endl;
    // clear all entries
	for(int i=0;i<LINES;i++)
	{
		for(int j=0;j<ASSOC;j++)
		{
            // update LRU (0 at boot)
			cache[i][j].lru=counter;
            // mark as invalid
			cache[i][j].valid=FALSE;
		}
	}
}

// instrumentation callback
VOID check(VOID *PC,string *dis)
{
	// log
	OutFile << PC << " "<< *dis << endl;
    // check cache access
	check_cache(PC,dis);
    // clock
	counter++;
}

// Instrumentation function
VOID Instruction(INS ins, VOID *v)
{
    // instruction decode
	string dis = INS_Disassemble(ins);

    // if flush, then clear
    // i opt to clear whole cache and not just the line
	if(dis.find("clflush"))
	{
		clear_cache();
	}

    // instruction callback
	INS_InsertCall(ins,IPOINT_BEFORE,(AFUNPTR)check,IARG_INST_PTR,IARG_PTR,new string(dis),IARG_END);
}

// Parameters
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
		"o", "icache.out", "specify output file name");

/* At the end, close file */
VOID Fini(INT32 code, VOID *v)
{
	OutFile.close();
}

// Entry Point
int main(int argc, char * argv[])
{
	// Initialize pin, no args except the binary
	if (PIN_Init(argc, argv)) return -1;

	// Open log file
	OutFile.open(KnobOutputFile.Value().c_str());

    // empty cache at startup
	clear_cache();

	// instruction-level instrumentation
	INS_AddInstrumentFunction(Instruction, 0);

	// Application exit callback
	PIN_AddFiniFunction(Fini, 0);

	// Start the program
	PIN_StartProgram();

	return 0;
}
