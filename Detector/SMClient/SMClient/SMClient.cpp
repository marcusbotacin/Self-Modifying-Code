// Self Modifying Code
// SMC Detector client
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)

// Include block
#include<stdio.h>
#include<Windows.h>
#include<Psapi.h>
#include<signal.h>

// constants
#define MAX_STRING 4096

// Inverted I/O message
#define NOTIFY_EVENT(msg) printf("%s\n",msg);

// Inverted I/O structure
typedef struct _OVL_WRAPPER {
	OVERLAPPED  Overlapped;
	LONG        ReturnedSequence;
} OVL_WRAPPER, *POVL_WRAPPER;

// Inverted I/O Control
#define FILE_DEVICE_INVERTED 0xCF54
#define IOCTL_SMC_INVERT_NOTIFICATION CTL_CODE(FILE_DEVICE_INVERTED, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMC_INVERT_EVENT_OCCURRED CTL_CODE(FILE_DEVICE_INVERTED, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMC_START CTL_CODE(FILE_DEVICE_INVERTED, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMC_STOP CTL_CODE(FILE_DEVICE_INVERTED, 2052, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Driver Handle
HANDLE hFile;

// Ask for an alert
DWORD SendTheIoctl(POVL_WRAPPER Wrap, HANDLE driverHandle)
{
	DWORD code;
	DeviceIoControl(driverHandle,(DWORD)IOCTL_SMC_INVERT_NOTIFICATION,NULL,0,&Wrap->ReturnedSequence,sizeof(LONG),NULL,&Wrap->Overlapped);
	code = GetLastError();
	return code;
}

// Ask driver to start monitoring
void start(HANDLE driverHandle)
{
	DeviceIoControl(driverHandle,(DWORD)IOCTL_SMC_START,NULL,0,NULL,0,NULL,NULL);
}

// Ask driver to stop monitoring
void stop(HANDLE driverHandle)
{
	DeviceIoControl(driverHandle,(DWORD)IOCTL_SMC_STOP,NULL,0,NULL,0,NULL,NULL);
}

// Signal handler - finish monitoring
void terminator(int sig)
{
	// debug message
	printf("Stopping\n");
	// ask driver to stop
	stop(hFile);
	// terminate own execution
	exit(0);
}

// Entry
int main()
{
	POVL_WRAPPER wrap;
	DWORD code;
	HANDLE completionPortHandle;

	// register signal handler
	signal(SIGINT,terminator);

	// driver handle
	hFile=CreateFile(L"\\\\.\\SMCheckerDriver",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
	// error case
	if(hFile==INVALID_HANDLE_VALUE) /* Check */
	{
		// error message
		printf("Driver handler error\n");
		// exit
		return 0;
	}

	// start monitoring
	start(hFile);

	// port to receive the inverted I/O
	completionPortHandle = CreateIoCompletionPort(hFile,NULL,0,0);
	// error case
	if(completionPortHandle == NULL) {
		// debug message
		printf("Completion error\n");
		// exit
		return 0;
	}

	// allocate inverted wrapper
	wrap = (POVL_WRAPPER)malloc(sizeof(OVL_WRAPPER));

	// infinite loop
	while(1){

		// clear wrapper
		memset(wrap, 0, sizeof(OVL_WRAPPER));
		// ask for a notification
		code = SendTheIoctl(wrap,hFile);

		// debug print
		printf("IOCTL sent\n");

		// identify what happened when returning
		code = GetLastError();

		if(code != ERROR_IO_PENDING)  {
			// debug print
			printf("DeviceIoControl failed: %x\n", code);
			// exit
			return 0;
		}

		DWORD byteCount = 0;
		ULONG_PTR compKey = 0;
		OVERLAPPED* overlapped = NULL;
		wrap = NULL;
		// Wait for a completion notification.
		overlapped = NULL;
		// debug print
		printf("waiting for event\n");

		// complete request
		BOOL worked = GetQueuedCompletionStatus(completionPortHandle,&byteCount,&compKey,&overlapped,INFINITE);

		// check what was received
		if (byteCount == 0) {
			// debug print
			printf("Error: Zero\n");
		}

		// check for errors
		if (overlapped == NULL) {
			//debug print
			printf("Error: NULL\n");
		}

		wrap = (POVL_WRAPPER)overlapped;
		code = GetLastError();
		char buff[MAX_STRING];

		// Here, returned sequence is the process which triggered SMC detection
		// Open it for inspection
		HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,wrap->ReturnedSequence);
		if(GetProcessImageFileNameA(proc,buff,4096)!=0)
		{
			// opened
			printf("SMC at %d: %s\n",wrap->ReturnedSequence,buff);
		}else{
			// failed to open
			printf("SMC at %d: Unknown Process\n",wrap->ReturnedSequence);
		}
	}

	// finish
	return 0;
}