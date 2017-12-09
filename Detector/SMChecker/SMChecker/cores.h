// Self Modifying Code
// SMC detector based on PEBS counter overflow
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)
// Implementation based on VoiDbg Driver

// maximum number of processors
#define MAX_PROC 4
// first core index
#define FIRST_CORE 0
// function protorype
BOOLEAN is_first_core(int core);