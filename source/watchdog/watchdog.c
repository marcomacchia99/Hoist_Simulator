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

#define TIMEDELTA 60

int fd_starter;
char buffer[SIZE];

int pid_motX;
int pid_motZ;
int pid_inspection;
int pid_command;

time_t time_check;

FILE *log_file;

void signal_handler(int sig)
{
    //writing to log file
    fprintf(log_file, "watchdog: signal received\n");
    fflush(log_file);
    //reset timer
    time_check = time(NULL);
}

int main(int argc, char *argv[])
{

    signal(SIGUSR1, signal_handler);

    //open log file
    log_file = fopen("./../logs/log.txt", "a");

    //defining and opening fifo
    char *fifo_watchdog_pid_command = "/tmp/watchdog_pid_c";
    char *fifo_watchdog_pid_inspection = "/tmp/watchdog_pid_i";
    char *fifo_watchdog_pid_motX = "/tmp/watchdog_pid_x";
    char *fifo_watchdog_pid_motZ = "/tmp/watchdog_pid_z";
    char *fifo_command_pid = "/tmp/pid_c";
    char *fifo_inspection_pid = "/tmp/pid_i";
    char *fifo_motZ_pid = "/tmp/pid_z";
    char *fifo_motX_pid = "/tmp/pid_x";

    mkfifo(fifo_watchdog_pid_command, 0666);
    mkfifo(fifo_watchdog_pid_inspection, 0666);
    mkfifo(fifo_watchdog_pid_motX, 0666);
    mkfifo(fifo_watchdog_pid_motZ, 0666);
    mkfifo(fifo_command_pid, 0666);
    mkfifo(fifo_inspection_pid, 0666);
    mkfifo(fifo_motX_pid, 0666);
    mkfifo(fifo_motZ_pid, 0666);

    //sending pid to other processes
    int fd_watchdog_pid_command = open(fifo_watchdog_pid_command, O_WRONLY);
    int fd_watchdog_pid_inspection = open(fifo_watchdog_pid_inspection, O_WRONLY);
    int fd_watchdog_pid_motX = open(fifo_watchdog_pid_motX, O_WRONLY);
    int fd_watchdog_pid_motZ = open(fifo_watchdog_pid_motZ, O_WRONLY);

    sprintf(buffer, "%d", (int)getpid());
    write(fd_watchdog_pid_command, buffer, strlen(buffer) + 1);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_watchdog_pid_inspection, buffer, strlen(buffer) + 1);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_watchdog_pid_motX, buffer, strlen(buffer) + 1);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_watchdog_pid_motZ, buffer, strlen(buffer) + 1);

    close(fd_watchdog_pid_command);
    unlink(fifo_watchdog_pid_command);
    close(fd_watchdog_pid_inspection);
    unlink(fifo_watchdog_pid_inspection);
    close(fd_watchdog_pid_motX);
    unlink(fifo_watchdog_pid_motX);
    close(fd_watchdog_pid_motZ);
    unlink(fifo_watchdog_pid_motZ);

    //writing in log file
    fprintf(log_file, "watchdog: sent pid to other processes\n");
    fflush(log_file);

    //reading other processes's pid
    int fd_pid_command = open(fifo_command_pid, O_RDONLY);
    int fd_pid_inspection = open(fifo_inspection_pid, O_RDONLY);
    int fd_pid_motX = open(fifo_motX_pid, O_RDONLY);
    int fd_pid_motZ = open(fifo_motZ_pid, O_RDONLY);
    read(fd_pid_command, buffer, SIZE);
    pid_command = atoi(buffer);
    read(fd_pid_inspection, buffer, SIZE);
    pid_inspection = atoi(buffer);
    read(fd_pid_motX, buffer, SIZE);
    pid_motX = atoi(buffer);
    read(fd_pid_motZ, buffer, SIZE);
    pid_motZ = atoi(buffer);
    close(fd_pid_command);
    close(fd_pid_inspection);
    close(fd_pid_motX);
    close(fd_pid_motZ);

    //writing in log file
    fprintf(log_file, "watchdog: sent read other processes's pid\n");
    fflush(log_file);

    //start timer
    time_check = time(NULL);

    while (1)
    {
        sleep(1);
        if (difftime(time(NULL), time_check) >= TIMEDELTA)
        {
            //if timer is more then TIMEDELTA send a reset signal to motors
            kill(pid_motX, SIGUSR2);
            kill(pid_motZ, SIGUSR2);
            //reset timer
            time_check = time(NULL);
        }
    }

    //close log file
    fclose(log_file);
    //unlink fifo
    unlink(fifo_command_pid);
    unlink(fifo_inspection_pid);
    unlink(fifo_motX_pid);
    unlink(fifo_motZ_pid);
    return 0;
}