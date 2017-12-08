// Self Modifying Code (SMC) example
// Marcus Botacin - 2017 - Federal University of Paraná (UFPR)

#include<stdio.h>   // I/O
#include<Windows.h> // Memory

// Change page permission to allow writing to code pages
void change_page(void *addr)
{
	DWORD old;
	SYSTEM_INFO si;
	UINT64 _addr = (UINT64)addr;
    // get system page size for the system
	GetSystemInfo(&si);
	int page_size = si.dwPageSize;
    // calculate page base address from code address
	_addr-= (UINT64)addr % page_size;
	
    // give page write AND exec permission
	if(!VirtualProtect((PVOID)_addr,page_size,PAGE_EXECUTE_READWRITE,&old))
		printf("Error: %x\n",GetLastError());

	return;
}

// SMC function
void foo()
{
	unsigned char *instruction; // instruction pointer
	int i=0;                    // loop iterator
	int acc=0;                  // accumulator

    // loop some times
	for(i=0;i<100;i++)
	{
        // even
		if(i%2==1)
		{
            // adjust pointer to function entry
			void *func_addr = (char*)foo+0x268;
            // adjust pointer to given instruction
			instruction = (unsigned char*)func_addr + 0x251;
            // debug print
            // instruction address
			printf("%x\n",instruction);
            // modify instruction
            // change increment from 1 to i
			*instruction=i;
			acc++;
		}else{
            // odd
            // just sum
			acc++;
		}
	}

    // print accumulated values
	printf("%d\n",acc);
}

// Entry Point
int main()
{
    // Debug prints
    // Display some information to externally track this process
	printf("CORE %u\n",GetCurrentProcessorNumber());
	printf("PID %u\n",GetCurrentProcessId());
    // allow mem writes
	change_page(foo);
    // call SMC function
	foo();
	return 0;
}

