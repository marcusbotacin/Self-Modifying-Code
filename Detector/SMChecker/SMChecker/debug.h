// Self Modifying Code
// SMC detector based on PEBS counter overflow
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)
// Implementation based on VoiDbg Driver

/* avoid multiple includes */
#pragma once

/* Print definitions */
#define DBG_PRINTS
#define PRINT_NAME "SMChecker"

/* Function definitions */
void dbg_print(char *msg);
void dbg_print_var(char *msg, long var);