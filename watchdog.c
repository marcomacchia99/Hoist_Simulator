#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define SIZE 80

#define TIMEDELTA 10

int fd_starter;
char buffer[SIZE];

int pid_motX;
int pid_motZ;
int pid_inspection;
int pid_command;

time_t time_check;

void sigusr2_handler(int sig)
{
    printf("Something happened!\n");
    time_check = time(NULL);
}

int main(int argc, char *argv[])
{

    char *fifo_starter_watchdog = "/tmp/watchdog";
    char *fifo_starter_watchdog = "/tmp/watchdog_pid";
    mkfifo(fifo_starter_watchdog, 0666);

    fd_starter = open(fifo_starter_watchdog, O_RDONLY);

    read(fd_starter, buffer, SIZE);
    pid_command = atoi(buffer);
    read(fd_starter, buffer, SIZE);
    pid_inspection = atoi(buffer);
    read(fd_starter, buffer, SIZE);
    pid_motX = atoi(buffer);
    read(fd_starter, buffer, SIZE);
    pid_motZ = atoi(buffer);

    close(fd_starter);
    unlink(fifo_starter_watchdog);

    signal(SIGUSR2, sigusr2_handler);

    time_check = time(NULL);

    while(1)
    {
        sleep(1);
        printf("difftime: %f\n", difftime(time(NULL),time_check) );
        if(difftime(time(NULL),time_check)>TIMEDELTA)
        {
            //invio reset///////////////////////////////////////////////////////////////////////////// USR2
            printf("Nothing happened for %d seconds, sending reset.\n",TIMEDELTA);
            time_check = time(NULL);
        }
    }



    return 0;
}