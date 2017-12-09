// Self Modifying Code
// SMC detector based on PEBS counter overflow
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)
// Implementation based on VoiDbg Driver

/* avoid multiple includes */
#pragma once

/* Default user messages */

#define GET_OUT_BUFFER_ERROR "Cannot get output buffer"
#define GET_IN_BUFFER_ERROR "Cannot get input buffer"
#define NO_CACHED_IO "No cached I/O request"
#define INVALID_IOCTL "Received an invalid IOCTL"
#define IOCTL_DISPATCH "Entering IOCTL Dispatch"
#define SEND_PID_MSG "Called SEND_PID IOCTL"
#define PID_OK_MSG "PID Received"
#define GET_REG_MSG "Called GET_REG IOCTL"
#define BUFFER_SIZE_ERROR "Buffer has not enough size"
#define GET_DATA_MSG "Called GET_DATA IOCTL"
#define BRANCH_DATA_MSG "Returning Branch Data"
#define CACHE_REQUEST "Asked to cache i/o request"
#define CACHE_REQUEST_ERROR "Cannot cache i/o request"
#define ENTRY_MESSAGE "Entry Point"
#define DCFAIL_MESSAGE "WdfDriverCreate failed"
#define PMI_HOOK_MESSAGE "Hooking PMI"
#define PMI_HOOK_ERROR_MESSAGE "Failed to hook PMI"
#define PMI_UNHOOK_MESSAGE "Unhooking PMI"
#define PMI_UNHOOK_ERROR_MESSAGE "Failed to unhook PMI"
#define ASSIGN_FAIL_MESSAGE "WdfDeviceInitAssignName failed"
#define INIT_FAIL_MESSAGE "WdfDeviceInitAssignSDDLString failed"
#define DC_FAIL_MESSAGE "WdfDeviceCreate failed"
#define SYM_FAIL_MESSAGE "WdfDeviceCreateSymbolicLink failed"
#define IOC_FAIL_MESSAGE "WdfIoQueueCreate for default queue failed"
#define IOQ_FAIL_MESSAGE "WdfIoQueueCreate for manual queue failed"
#define IOCTL_START_MESSAGE "Starting SMC monitoring"
#define IOCTL_STOP_MESSAGE "Stopping SMC monitoring"
#define SMC_MESSAGE "Detected SMC on PID"