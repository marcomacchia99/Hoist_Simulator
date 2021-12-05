#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SIZE 80

int fd_motX;
int fd_motZ;

int pid_watchdog;

void print_instruction()
{
    // displays all the instructions
    system("clear");
    printf("AVAILABLE COMMANDS: \n");
    printf("I to go UP\n");
    printf("K to go DOWN\n");
    printf("J to go LEFT\n");
    printf("L to go RIGHT\n");
    printf("Z to stop z movement\n");
    printf("X to stop x movement\n\n\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{

    // pipe file path
    char *fifo_command_motorX = "/tmp/command_motorX";
    char *fifo_command_motorZ = "/tmp/command_motorZ";
    char *fifo_watchdog_pid = "/tmp/watchdog_pid_c";
    char *fifo_command_pid = "/tmp/pid_c";

    // creating fifo
    mkfifo(fifo_command_motorX, 0666);
    mkfifo(fifo_command_motorZ, 0666);
    mkfifo(fifo_watchdog_pid, 0666);
    mkfifo(fifo_watchdog_pid, 0666);

    char buffer[SIZE];


    //getting watchdog pid
    int fd_watchdog_pid = open(fifo_watchdog_pid, O_RDONLY);
    read(fd_watchdog_pid, buffer, SIZE);
    pid_watchdog = atoi(buffer);
    close(fd_watchdog_pid);

    //writing own pid
    int fd_command_pid = open(fifo_command_pid, O_WRONLY);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_command_pid, buffer, SIZE);
    close(fd_command_pid);


    //user input is stored here
    char input_ch[80];

    print_instruction();

    fd_motX = open(fifo_command_motorX, O_WRONLY);
    fd_motZ = open(fifo_command_motorZ, O_WRONLY);

    while (1)
    {

        scanf("%s", input_ch);

        print_instruction();

        if (strlen(input_ch) > 1)
        {
            printf("Unrecognized command, press only one button at a time!\n");
            fflush(stdout);
        }

        else
        {
            //the actual command is stored here
            char out_str[80];

            sprintf(out_str, "%d", input_ch[0]);

            switch (input_ch[0])
            {
            case 'I':
            case 'i':
                //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                printf("UP PRESSED\n");
                fflush(stdout);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                kill(pid_watchdog, SIGUSR1);
                break;

            case 'K':
            case 'k':
                //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                printf("DOWN PRESSED\n");
                fflush(stdout);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                kill(pid_watchdog, SIGUSR1);
                break;

            case 'J':
            case 'j':
                //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                printf("LEFT PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                kill(pid_watchdog, SIGUSR1);
                break;

            case 'L':
            case 'l':
                //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                printf("RIGHT PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                kill(pid_watchdog, SIGUSR1);
                break;

            case 'X':
            case 'x':
                //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                printf("STOP X PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                kill(pid_watchdog, SIGUSR1);
                break;

            case 'Z':
            case 'z':
                //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                printf("STOP Z PRESSED\n");
                fflush(stdout);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                kill(pid_watchdog, SIGUSR1);
                break;

            default:
                printf("Unrecognized command, please try again: \n");
                fflush(stdout);
                break;
            }
        }
    }

    //close fifo
    close(fd_motX);
    unlink(fifo_command_motorX);
    close(fd_motZ);
    unlink(fifo_command_motorZ);

    return 0;
}
