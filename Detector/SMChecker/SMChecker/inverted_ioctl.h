// Self Modifying Code
// SMC detector based on PEBS counter overflow
// Marcus Botacin - 2017
// Federal University of Paraná (UFPR)
// Implementation based on VoiDbg Driver

/* avoid multiple includes */
#pragma once

#define FILE_DEVICE_INVERTED 0xCF54

/* Operations - control codes */
#define IOCTL_SMC_INVERT_NOTIFICATION CTL_CODE(FILE_DEVICE_INVERTED, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMC_INVERT_EVENT_OCCURRED CTL_CODE(FILE_DEVICE_INVERTED, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMC_START CTL_CODE(FILE_DEVICE_INVERTED, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMC_STOP CTL_CODE(FILE_DEVICE_INVERTED, 2052, METHOD_BUFFERED, FILE_ANY_ACCESS)