// Self Modifying Code (SMC) example
// Cache-flush version
// Marcus Botacin - 2017 - Federal University of Paraná (UFPR)    

#include<stdio.h>   // I/O
#include<Windows.h> // Mem
#include<intrin.h>  // cache flush asm

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

// SMC function
void foo()
{
	unsigned char *instruction;
	int i=0;
	int acc=0;
	for(i=0;i<100;i++)
	{
		if(i%2==1)
		{
			void *func_addr = (char*)foo+0x40c;
			instruction = (unsigned char*)func_addr + 0xbd;
            // flush first
			_mm_clflush(instruction);
            // then change
			*instruction=0x0;
		}else{
			acc++;
		}
	}
    // print values
	printf("%d\n",acc);
	printf("%x\n",instruction);
}

// program entry
int main()
{
	printf("CORE %u\n",GetCurrentProcessorNumber());
	printf("PID %u\n",GetCurrentProcessId());
	change_page(foo);
	foo();
	
	return 0;
}

