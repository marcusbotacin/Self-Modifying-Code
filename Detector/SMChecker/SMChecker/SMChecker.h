// Self Modifying Code
// SMC detector based on PEBS counter overflow
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)
// Implementation based on VoiDbg Driver

/* avoid multiple includes */
#pragma once

/* MSR defs*/
#include<intrin.h>

/* Driver defs */
#define NATIVE_NAME L"\\Device\\SMCheckerDriver"
#define USER_NAME L"\\Global??\\SMCheckerDriver"

/* Thread defs */
#define DELAY_INTERVAL -10000000*1

/* Interrupt defs */
#define APIC_INT_VALUE 0x400
#define BTS_INTERRUPT_FLAGS 0x3C0

/* Struct definitions */

// BTS struct
typedef struct st_BTSBUFFER
{
	UINT64 FROM,TO,MISC;
}TBTS_BUFFER,*PTBTS_BUFFER;

// PEBS struct
typedef struct st_PEBSBUFFER
{
	UINT64 RFLAGS,RIP,RAX,RBX,RCX,RDX,RSI,RDI,RBP,RSP,R8,R9,R10,R11,R12,R13,R14,R15;
}TPEBS_BUFFER,*PTPEBS_BUFFER;

// DSBASE
typedef struct st_DSBASE
{
	// Pointer to BTS struct
	PTBTS_BUFFER BUFFER_BASE,BTS_INDEX,MAXIMUM,THRESHOLD;
	// Pointer to PEBS struct
	PTPEBS_BUFFER PEBS_BASE,PEBS_INDEX,PEBS_MAXIMUM,PEBS_THRESHOLD;
}TDS_BASE,*PTDS_BASE;