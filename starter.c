#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SIZE 80

int spawn(const char *program, char **arg_list)
{
    pid_t child_pid = fork();
    if (child_pid != 0)
        return child_pid;
    else
    {
        execvp(program, arg_list);

        perror("exec failed");
        return 1;
    }
}

int pid_command;
int pid_inspection;
int pid_motorX;
int pid_motorZ;
int pid_watchdog;
char buffer[SIZE];

int fd_watchdog;
int fd_inspection;

int main(int argc, char *argv[])
{
    char *fifo_starter_watchdog = "/tmp/watchdog";
    char *fifo_starter_inspection = "/tmp/inspection";
    mkfifo(fifo_starter_watchdog, 0666);
    mkfifo(fifo_starter_inspection, 0666);

    char *fifo_watchdog_pid_command = "/tmp/watchdog_pid_c";
    char *fifo_watchdog_pid_inspection = "/tmp/watchdog_pid_i";
    char *fifo_watchdog_pid_motX = "/tmp/watchdog_pid_x";
    char *fifo_watchdog_pid_motZ = "/tmp/watchdog_pid_z";

    mkfifo(fifo_watchdog_pid_command, 0666);
    mkfifo(fifo_watchdog_pid_inspection, 0666);
    mkfifo(fifo_watchdog_pid_motX, 0666);
    mkfifo(fifo_watchdog_pid_motZ, 0666);

    char program[] = "/usr/bin/konsole";
    char *arg_list_1[] = {"/usr/bin/konsole", "-e", "./commandConsole", "", (char *)NULL};
    char *arg_list_2[] = {"/usr/bin/konsole", "-e", "./inspectionConsole", "", (char *)NULL};
    char *arg_list_3[] = {"/usr/bin/konsole", "-e", "./motorX", "", (char *)NULL};
    char *arg_list_4[] = {"/usr/bin/konsole", "-e", "./motorZ", "", (char *)NULL};
    char *arg_list_5[] = {"/usr/bin/konsole", "-e", "./watchdog", "", (char *)NULL};

    pid_motorX = spawn(program, arg_list_3);
    pid_motorZ = spawn(program, arg_list_4);
    pid_command = spawn(program, arg_list_1);
    pid_inspection = spawn(program, arg_list_2);
    pid_watchdog = spawn(program, arg_list_5);



    return 0;
}
