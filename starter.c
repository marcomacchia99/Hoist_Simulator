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

int main(int argc, char *argv[])
{
    char *fifo_watchdog = "/tmp/watchdog";
    mkfifo(fifo_watchdog, 0666);

    char program[] = "/usr/bin/konsole";
    char *arg_list_1[] = {"/usr/bin/konsole", "-e", "./commandConsole","", (char*)NULL };
    char *arg_list_2[] = {"/usr/bin/konsole", "-e", "./inspectionConsole","", (char*)NULL };
    char *arg_list_3[] = {"/usr/bin/konsole", "-e", "./motorX","", (char*)NULL };
    char *arg_list_4[] = {"/usr/bin/konsole", "-e", "./motorZ","", (char*)NULL };
    char *arg_list_5[] = {"/usr/bin/konsole", "-e", "./watchdog","", (char*)NULL };

    pid_command = spawn(program, arg_list_1);
    pid_inspection = spawn(program, arg_list_2);
    pid_motorX = spawn(program, arg_list_3);
    pid_motorZ = spawn(program, arg_list_4);
    pid_watchdog = spawn(program, arg_list_5);

    /* fd_watchdog = open(fifo_watchdog, O_WRONLY);
    sprintf(buffer, "%d,%d,%d,%d,%d", pid_command, pid_inspection, pid_motorX, pid_motorZ, pid_watchdog);
    write(fd_watchdog, buffer, strlen(buffer) + 1);

    close(fd_watchdog);
    unlink(fifo_watchdog); */

    return 0;
}
