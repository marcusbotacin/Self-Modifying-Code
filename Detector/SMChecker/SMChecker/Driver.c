// Self Modifying Code
// SMC detector based on PEBS counter overflow
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)
// Implementation based on VoiDbg Driver

// Include Block

#include "Driver.h"			/*	WDF defs				*/
#include "SMChecker.h"		/*	Driver defs				*/
#include "PEBS.h"			/*	PEBS data types defs	*/
#include "debug.h"			/*	Driver debug defs		*/
#include "IO.h"				/*	I/O defs				*/
#include "messages.h"		/*	Default prints defs		*/
#include "cores.h"			/*  Multi-core issues		*/

/* global variables */

/* thread handler */
HANDLE thandle;

/* declare unmapped APIC pointer */
UINT32* APIC=NULL;

// PID which executed SMC
LONG intPID;

/* Interrupt Request Queue (copy) */
WDFQUEUE Queue2;

// PEBS Buffers
// One for each core
PTDS_BASE DS_BASE[MAX_PROC];
PTPEBS_BUFFER PEBS_BUFFER[MAX_PROC];

// Buffers Setup Routine
// Changed from BTS to PEBS
void FILL_DS_WITH_BUFFER(PTDS_BASE DS_BASE,PTPEBS_BUFFER PEBS_BUFFER)
{
	// Ignoring BTS data here
	/* PEBS BUFFER BASE */
	DS_BASE->PEBS_BASE=PEBS_BUFFER;
	/* PEBS INDEX */
	DS_BASE->PEBS_INDEX=PEBS_BUFFER;
	/* PEBS MAX */
	DS_BASE->PEBS_MAXIMUM=PEBS_BUFFER+1;
	/* PEBS Threshold */
	DS_BASE->PEBS_THRESHOLD=PEBS_BUFFER;
}

/* Driver Entry Point */
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	WDF_DRIVER_CONFIG config;
	NTSTATUS status;

	// debug message
	dbg_print(ENTRY_MESSAGE);

	WDF_DRIVER_CONFIG_INIT(&config, InvertedEvtDeviceAdd);

	status = WdfDriverCreate(DriverObject,RegistryPath,WDF_NO_OBJECT_ATTRIBUTES,&config,WDF_NO_HANDLE);

	/* error case */
	if (!NT_SUCCESS(status)) {
		dbg_print(DCFAIL_MESSAGE);
	}

	// just setup things
	// start/stop is performed by threads
	// called through IOCTLs

	return status;
}

/* attach thread to a given core */
void thread_attach_to_core(unsigned id)
{
	KAFFINITY mask;
#pragma warning( disable : 4305 )
#pragma warning( disable : 4334 )
	mask=1<<id;
	KeSetSystemAffinityThread(mask);
}

/* delay execution for a while */
void delay_exec(INT64 interval)
{
	LARGE_INTEGER _interval;
	_interval.QuadPart=interval;
	KeDelayExecutionThread(KernelMode,FALSE,&_interval);
}



// Interrupt Routine
void PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext) 
{
	PINVERTED_DEVICE_CONTEXT devContext;
	LARGE_INTEGER pa;
	UINT32* APIC;
	UNREFERENCED_PARAMETER(Interrupt);
	UNREFERENCED_PARAMETER(ServiceContext);

	// disable PEBS
	__writemsr(MSR_IA32_PEBS_ENABLE, DISABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);

	// Get PID which triggered detection
	intPID = (LONG)PsGetCurrentProcessId();

	// user notification
	dbg_print_var(SMC_MESSAGE,intPID);

	// clear APIC flag
	pa.QuadPart=PERF_COUNTER_APIC;
	APIC=(UINT32*)MmMapIoSpace(pa,sizeof(UINT32),MmNonCached);
	*APIC=ORIGINAL_APIC_VALUE;
	MmUnmapIoSpace(APIC,sizeof(UINT32));		

	// fires inverted notification
	devContext = InvertedGetContextFromDevice(WdfIoQueueGetDevice(Queue2));
	InvertedNotify(devContext);

	// reenables PEBS here if you want

} 

// PMI handlers
void *perfmon_hook = PMI;
void *restore_hook = NULL;

// Uninstall PMI ISR
void unhook_handler()
{
	HalSetSystemInformation(HalProfileSourceInterruptHandler,sizeof(PVOID*),&restore_hook);
}

// Install PMI ISR
void hook_handler()
{
	NTSTATUS status;
	status = HalSetSystemInformation(HalProfileSourceInterruptHandler,sizeof(PVOID*),&perfmon_hook);
}

// Start to monitor SMC execution
VOID StarterThread(_In_ PVOID StartContext)
{
	// avoid warnings
	int core;
	core = (int)StartContext;

	
	// Hook PMI handler
	// only one core is enough
	if(is_first_core(core))
	{
		dbg_print(PMI_HOOK_MESSAGE);
		hook_handler();
	}else{
		dbg_print(PMI_HOOK_ERROR_MESSAGE);
	}

	// delay execution for a while
	delay_exec(DELAY_INTERVAL);

	// Attach thread to core
	thread_attach_to_core(core);

	// Allocate buffers
	DS_BASE[core] = (PTDS_BASE)ExAllocatePool(NonPagedPool,sizeof(TDS_BASE));
	PEBS_BUFFER[core]=(PTPEBS_BUFFER)ExAllocatePool(NonPagedPool,sizeof(TPEBS_BUFFER));
	FILL_DS_WITH_BUFFER(DS_BASE[core],PEBS_BUFFER[core]);

	// enable mechanism through MSRs

	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);
	__writemsr(MSR_IA32_PERFCTR0, -PERIOD);
	__writemsr(MSR_IA32_EVNTSEL0,pebs_event | EVTSEL_EN | EVTSEL_USR | EVTSEL_INT);
	__writemsr(MSR_IA32_PEBS_ENABLE, ENABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, ENABLE_PEBS);
}

// Thread used to terminate SMC execution monitoring
VOID StopperThread(_In_ PVOID StartContext)
{
	/* avoid warnings */
	int core;
	core= (int)StartContext;
	
	// attach to a given core
	thread_attach_to_core(core);

	/* Disable PEBS for counter 0 */
	__writemsr(MSR_IA32_PEBS_ENABLE, DISABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);

	// delay execution for a while
	delay_exec(DELAY_INTERVAL);

	// Unhook PMI
	// only one core is enough
	if(is_first_core(core))
	{
		dbg_print(PMI_UNHOOK_MESSAGE);
		unhook_handler();
	}else{
		dbg_print(PMI_UNHOOK_ERROR_MESSAGE);
	}

	// Free allocated memory
	ExFreePool(PEBS_BUFFER[core]);
	ExFreePool(DS_BASE[core]);
}

/* Add device routine - setup */
NTSTATUS InvertedEvtDeviceAdd(WDFDRIVER Driver, PWDFDEVICE_INIT DeviceInit)
{
	NTSTATUS status;
	WDF_OBJECT_ATTRIBUTES objAttributes;
	WDFDEVICE device;
	WDF_IO_QUEUE_CONFIG queueConfig;
	PINVERTED_DEVICE_CONTEXT devContext;

	DECLARE_CONST_UNICODE_STRING(nativeDeviceName,NATIVE_NAME);
	DECLARE_CONST_UNICODE_STRING(userDeviceName,USER_NAME);

	UNREFERENCED_PARAMETER(Driver);

	WDF_OBJECT_ATTRIBUTES_INIT(&objAttributes);

	WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&objAttributes,INVERTED_DEVICE_CONTEXT);

	status = WdfDeviceInitAssignName(DeviceInit, &nativeDeviceName);

	/* error case */
	if (!NT_SUCCESS(status)) {
		dbg_print(ASSIGN_FAIL_MESSAGE);
		return status;
	}

	status = WdfDeviceInitAssignSDDLString(DeviceInit,&INVERTED_DEVICE_PROTECTION);

	/* error case */
	if (!NT_SUCCESS(status)) {
		dbg_print(INIT_FAIL_MESSAGE);
		return status;
	}

	status = WdfDeviceCreate(&DeviceInit,&objAttributes,&device);

	/* error case */
	if (!NT_SUCCESS(status)) {
		dbg_print(DC_FAIL_MESSAGE);
		return status;
	}

	devContext = InvertedGetContextFromDevice(device);    

	status = WdfDeviceCreateSymbolicLink(device, &userDeviceName);

	if (!NT_SUCCESS(status)) {
		dbg_print(SYM_FAIL_MESSAGE);
		return status;
	}

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,WdfIoQueueDispatchParallel);

	queueConfig.EvtIoDeviceControl = InvertedEvtIoDeviceControl;
	queueConfig.PowerManaged = WdfFalse;

	status = WdfIoQueueCreate(device,&queueConfig,WDF_NO_OBJECT_ATTRIBUTES,WDF_NO_HANDLE);

	if (!NT_SUCCESS(status)) {
		dbg_print(IOC_FAIL_MESSAGE);
		return status;
	}

	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig,WdfIoQueueDispatchManual);

	queueConfig.PowerManaged = WdfFalse;

	status = WdfIoQueueCreate(device,&queueConfig,WDF_NO_OBJECT_ATTRIBUTES,&devContext->NotificationQueue);

	if (!NT_SUCCESS(status)) {
		dbg_print(IOQ_FAIL_MESSAGE);
		return status;
	}

	return status;
}

/* I/O Dispatch Routine */
VOID InvertedEvtIoDeviceControl(WDFQUEUE Queue,
								WDFREQUEST Request,
								size_t OutputBufferLength,
								size_t InputBufferLength,
								ULONG IoControlCode)
{
	/* general vars */
	int i;
	PINVERTED_DEVICE_CONTEXT devContext;
	NTSTATUS status;
	ULONG_PTR info;	
	/* Avoid some warnings */
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	/* Initial Setup */
	Queue2=Queue;

	/* Get device context */
	devContext = InvertedGetContextFromDevice(WdfIoQueueGetDevice(Queue));

	/* empty status to start */
	status = STATUS_INVALID_PARAMETER;
	info = NO_DATA_RETURNED;

	/* notify user */
	dbg_print(IOCTL_DISPATCH);

	/* Check Operation Code */
	switch(IoControlCode) {
		/* Asked to cache an I/O request */
	case IOCTL_SMC_INVERT_NOTIFICATION: {

		/* User notification */
		dbg_print(CACHE_REQUEST);

		/* buffer size check */
		if(OutputBufferLength < sizeof(LONG)) {
			dbg_print(GET_OUT_BUFFER_ERROR);
			break;
		}

		/* try to cache request */
		status = WdfRequestForwardToIoQueue(Request,devContext->NotificationQueue);

		/* check if request was cached */
		if(!NT_SUCCESS(status)) {
			dbg_print(CACHE_REQUEST_ERROR);
			break;
		}
		return;
										}

										/* case an interrupt occured */										
	case IOCTL_SMC_INVERT_EVENT_OCCURRED: {
		/* fires inverted I/O call */
		InvertedNotify(devContext);
		status = STATUS_SUCCESS;
		break;
										  }

										  // required to start SMC detection
	case IOCTL_SMC_START:
		{
			// debug print
			dbg_print(IOCTL_START_MESSAGE);
			// for each core
			for(i=0;i<MAX_PROC;i++)
			{
				// create a starting thread
				PsCreateSystemThread(&thandle,GENERIC_ALL,NULL,NULL,NULL,StarterThread,(VOID*)i);
			}
			status = STATUS_SUCCESS;
			break;
		}
		// required to stop SMC detection
	case IOCTL_SMC_STOP:
		{
			// debug message
			dbg_print(IOCTL_STOP_MESSAGE);
			// for each core
			for(i=0;i<MAX_PROC;i++)
			{
				// create a killer thread
				PsCreateSystemThread(&thandle,GENERIC_ALL,NULL,NULL,NULL,StopperThread,(VOID*)i);
			}
			status = STATUS_SUCCESS;
			break;
		}

		/* Any other case is considered an error */
	default: {
		dbg_print(INVALID_IOCTL);
		break;
			 }
	}

	WdfRequestCompleteWithInformation(Request,status,info);    
}

/* Inverted I/O Implementation
* This function implements the interrupt notification
*/
VOID InvertedNotify(PINVERTED_DEVICE_CONTEXT DevContext)
{
	NTSTATUS status;
	ULONG_PTR info;
	WDFREQUEST notifyRequest;
	PULONG  bufferPointer;
	LONG valueToReturn;

	/* Get cached I/O Request */
	status = WdfIoQueueRetrieveNextRequest(DevContext->NotificationQueue,&notifyRequest);

	/* Case no request available */
	if(!NT_SUCCESS(status)) {    
		dbg_print(NO_CACHED_IO);
		return;
	}

	/* get the output buffer */
	status = WdfRequestRetrieveOutputBuffer(notifyRequest,sizeof(LONG),(PVOID*)&bufferPointer,NULL); 

	/* case cannot get buffer */
	if(!NT_SUCCESS(status)) {
		dbg_print(GET_OUT_BUFFER_ERROR);
		status = STATUS_SUCCESS;
		/* No Data Returned */
		info = NO_DATA_RETURNED;
	} else {
		// Data to Return
		// returning PID
		valueToReturn = intPID;
		*bufferPointer = valueToReturn;
		status = STATUS_SUCCESS;
		/* return data size */
		info = sizeof(valueToReturn);
	}

	/* Complete I/O */
	WdfRequestCompleteWithInformation(notifyRequest, status, info);
}

/* Implements the debug print */
void dbg_print(char *msg)
{
#ifdef DBG_PRINTS
	DbgPrint("[%s] %s",PRINT_NAME,msg);
#else
	return;
#endif
}

/* dbg_print version which also displays variable values */
void dbg_print_var(char *msg, long var)
{
#ifdef DBG_PRINTS
	DbgPrint("[%s] %s: %x",PRINT_NAME,msg,var);
#else
	return;
#endif
}

// check if current core is the first one
BOOLEAN is_first_core(int core)
{
	return core == FIRST_CORE;
}