#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int fd_motX;
int fd_motZ;

void print_instruction()
{

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

    mkfifo(fifo_command_motorX, 0666);
    mkfifo(fifo_command_motorZ, 0666);


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
            char out_str[80];
            sprintf(out_str, "%d", input_ch[0]);

            switch (input_ch[0])
            {
            case 73:
            case 105:
                printf("UP PRESSED\n");
                fflush(stdout);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                break;

            case 75:
            case 107:
                printf("DOWN PRESSED\n");
                fflush(stdout);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                break;

            case 74:
            case 106:
                printf("LEFT PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                break;

            case 76:
            case 108:
                printf("RIGHT PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                break;

            case 88:
            case 120:
                printf("STOP X PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                break;

            case 90:
            case 122:
                printf("STOP Z PRESSED\n");
                fflush(stdout);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                break;

            default:
                printf("Unrecognized command, please try again: \n");
                fflush(stdout);
                break;
            }
        }
    }
    close(fd_motX);
    unlink(fifo_command_motorX);
    close(fd_motZ);
    unlink(fifo_command_motorZ);


    return 0;
}
