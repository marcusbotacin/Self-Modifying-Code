// Self Modifying Code (SMC) example
// Shows the relation between SMC and anti-analysis
// Marcus Botacin - 2017 - Federal University of Paraná (UFPR)

#include<stdio.h>   // I/O
#include<Windows.h> // Mem

// supposed malicious function
void malicious(void) {
    // i is always zero
    int i=0;
    // thus this check is always fail
    // unless SMC changes this value
	if(i)
        // this is the malicious behavior
		printf("I'm a malware\n");
}

// simple anti analysis function
BOOL anti_analysis()
{
	return IsDebuggerPresent();
}

// page permission, as in the previous examples
void change_page(void *addr)
{
	DWORD old;
	SYSTEM_INFO si;
	UINT64 _addr = (UINT64)addr;
	GetSystemInfo(&si);
	int page_size = si.dwPageSize;
	_addr-= (UINT64)addr % page_size;
	
	if(!VirtualProtect((PVOID)_addr,page_size,PAGE_EXECUTE_READWRITE,&old))
		printf("Error: %x\n",GetLastError());

	return;
}

#define FUNC_ADJUST_OFFSET 0x26 // Function entry offset
#define INSTR_OFFSET 0xF        // instruction offset within the function
#define INSTR_DATA 0x2          // new instruction bytes

// Program entry
int main()
{
    // try first time
    // will be non-malicious
	malicious();

    //debug print
	printf("PID %u\n",GetCurrentProcessId());

    // anti-analysis trick
	if(!anti_analysis())
	{
        // SMC, as in the previous case
		void *func_addr = (char*)malicious+FUNC_ADJUST_OFFSET;
		change_page(malicious);
		unsigned char *instruction = (unsigned char*)func_addr + INSTR_OFFSET;
		*instruction = INSTR_DATA;
	}
	
    // try second time
    // this time will be malicious
    malicious();

	return 0;
}

