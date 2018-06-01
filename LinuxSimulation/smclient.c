/* SMC Alert - Performance Tests
 * Marcus Botacin - UFPR - 2018
 */

/* Include Block */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <time.h>
#include <unistd.h>

/* Device Path */
#define DEVICE "/dev/smcmod"

/* PID SIZE */
#define PID_SIZE 5

/* FLAG SIZE */
#define FLAG_SIZE 3

int main(int argc, char *argv[])
{
    int i, fd;

    /* Device Open */
    fd = open(DEVICE, O_RDWR);

    if(fd == -1){
        printf("Open Device Error");
        return 0;
    }

    char pid[PID_SIZE];

    struct timespec ts_initial, ts_final;

    /* Clocks before starting I/O */
    clock_gettime(CLOCK_MONOTONIC,&ts_initial);

    /* Get SMC PID */
    read(fd, pid, sizeof(pid));

    /* Write Decision */
    char decision[FLAG_SIZE];
    strcpy(decision,"1");

    /* Write Whitelist Information */
    write(fd, decision, sizeof(decision));

    /* Clocks after I/O */
    clock_gettime(CLOCK_MONOTONIC,&ts_final);

    /* Elapsed ticks */
    printf("%lu\n",ts_final.tv_nsec - ts_initial.tv_nsec); 

    /* Close device */
    close(fd);

    /* Finish */
    return 0;
}

