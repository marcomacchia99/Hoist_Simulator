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

int main(int argc, char *argv[])
{
    char *fifo_command_motorX = "/tmp/command_motorX";

  char * arg_list_1[] = { "/usr/bin/hoist",  "-e", "./commandconsole"};
  char * arg_list_2[] = { "/usr/bin/hoist",  "-e", "./inspectionconsole"}; 
  char * arg_list_3[] = { "/usr/bin/hoist",  "-e", "./motorX"};
  char * arg_list_4[] = { "/usr/bin/hoist",  "-e", "./motorZ"};
  char * arg_list_5[] = { "/usr/bin/hoist",  "-e", "./watchdog"};
}
