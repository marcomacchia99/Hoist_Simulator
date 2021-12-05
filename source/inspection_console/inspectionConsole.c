#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SIZE 80

float position_x = 0;
float position_z = 0;
int fd_motX;
int fd_motZ;
int fd_motX_value;
int fd_motZ_value;
int fd_stdin;
int fd_inspection;
int pid_motX;
int pid_motZ;
int pid_watchdog;

void print_position_and_instructions()
{

    system("clear");
    printf("X position: %.3f\n", position_x);
    printf("Z position: %.3f\n", position_z);
    printf("\n\nPRESS R TO RESET THE HOIST, PRESS S FOR EMERGENCY STOP\n\n");

    fflush(stdout);
}

int main(int argc, char *argv[])
{

    // variables for select function
    struct timeval timeout;
    timeout.tv_usec = 50;
    fd_set readfds;
    fd_set read_stdinfds;

    //user input is stored here
    char input_ch[SIZE];

    //motors position received from processes
    char value_from_motor_x[SIZE];
    char value_from_motor_z[SIZE];
    
    char buffer[SIZE];


    //defining and creating fifo

    char *fifo_inspection_motorX = "/tmp/inspection_motorX";
    char *fifo_inspection_motorZ = "/tmp/inspection_motorZ";
    char *fifo_motorX_value = "/tmp/motorX_value";
    char *fifo_motorZ_value = "/tmp/motorZ_value";
    char *fifo_watchdog_pid = "/tmp/watchdog_pid_i";
    char *fifo_inspection_pid = "/tmp/pid_i";
    char *fifo_motX_pid_inspection = "/tmp/pid_x_i";
    char *fifo_motZ_pid_inspection = "/tmp/pid_z_i";

    mkfifo(fifo_inspection_motorX, 0666);
    mkfifo(fifo_inspection_motorZ, 0666);
    mkfifo(fifo_motorX_value, 0666);
    mkfifo(fifo_motorZ_value, 0666);
    mkfifo(fifo_watchdog_pid, 0666);
    mkfifo(fifo_inspection_pid, 0666);
    mkfifo(fifo_motX_pid_inspection, 0666);
    mkfifo(fifo_motZ_pid_inspection, 0666);

    //getting watchdog pid
    int fd_watchdog_pid = open(fifo_watchdog_pid, O_RDONLY);
    read(fd_watchdog_pid, buffer, SIZE);
    pid_watchdog = atoi(buffer);

    close(fd_watchdog_pid);

    //writing own pid
    int fd_inspection_pid = open(fifo_inspection_pid, O_WRONLY);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_inspection_pid, buffer, SIZE);
    close(fd_inspection_pid);

    fd_motX = open(fifo_inspection_motorX, O_WRONLY);
    fd_motZ = open(fifo_inspection_motorZ, O_WRONLY);
    fd_stdin = fileno(stdin);


    //getting motors pid
    int fd_motX_pid_i = open(fifo_motX_pid_inspection, O_RDONLY);
    int fd_motZ_pid_i = open(fifo_motZ_pid_inspection, O_RDONLY);
    read(fd_motX_pid_i, buffer, SIZE);
    pid_motX = atoi(buffer);
    read(fd_motZ_pid_i, buffer, SIZE);
    pid_motZ = atoi(buffer);
    close(fd_motX_pid_i);
    close(fd_motZ_pid_i);

    fd_motX_value = open(fifo_motorX_value, O_RDONLY);
    fd_motZ_value = open(fifo_motorZ_value, O_RDONLY);

    print_position_and_instructions();

    while (1)
    {

        //setting timout microseconds to 0
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        //initialize with an empty set the file descriptors set
        FD_ZERO(&readfds);
        FD_ZERO(&read_stdinfds);

        //add the selected file descriptor to the selected fd_set
        FD_SET(fd_motX_value, &readfds);
        FD_SET(fd_motZ_value, &readfds);
        FD_SET(fileno(stdin), &read_stdinfds);

        switch (select(FD_SETSIZE + 1, &read_stdinfds, NULL, NULL, &timeout))
        {
        case 0: //timeout reached, so nothing new

            break;

        case -1: //error
            perror("Error inside inspectionConsole read: ");
            fflush(stdout);
            break;
        default: //if something is ready, we read it
            read(fd_stdin, input_ch, SIZE);
            input_ch[strcspn(input_ch, "\n")] = 0; //trims the \n command read by user input
            fflush(stdin);

            if (strlen(input_ch) > 1)
            {
                print_position_and_instructions();
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
                case 'R': //R
                case 'r': //r
                    print_position_and_instructions();

                    //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                    printf("RESET PRESSED\n");
                    fflush(stdout);
                    write(fd_motX, out_str, strlen(out_str) + 1);
                    write(fd_motZ, out_str, strlen(out_str) + 1);
                    kill(pid_watchdog, SIGUSR1);
                    break;

                case 'S': //S
                case 's': //s
                    print_position_and_instructions();

                    //if a command is recognized, the command is sent to the relative motor and a signal sent to watchdog
                    printf("EMERGENCY STOP PRESSED\n");
                    fflush(stdout);
                    kill(pid_motX, SIGUSR1);
                    kill(pid_motZ, SIGUSR1);
                    kill(pid_watchdog, SIGUSR1);
                    break;

                default:
                    print_position_and_instructions();

                    printf("Unrecognized command, please try again: \n");
                    fflush(stdout);
                    break;
                }
            }
            break;
        }

        //check if one of the motor has sent a new position
        switch (select(FD_SETSIZE + 1, &readfds, NULL, NULL, &timeout))
        {
        case 0: //timeout reached, so nothing new

            break;

        case -1: //error
            perror("Error inside inspectionConsole");
            fflush(stdout);
            break;
        default: //if something is ready, we read it
            if (FD_ISSET(fd_motX_value, &readfds))
            {
                //update position
                read(fd_motX_value, value_from_motor_x, SIZE);
                position_x = atof(value_from_motor_x);
            }
            if (FD_ISSET(fd_motZ_value, &readfds))
            {
                //update position
                read(fd_motZ_value, value_from_motor_z, SIZE);
                position_z = atof(value_from_motor_z);
            }

            print_position_and_instructions();
            break;
        }
    }

    //close fifo
    close(fd_motX);
    unlink(fifo_inspection_motorX);
    close(fd_motZ);
    unlink(fifo_inspection_motorZ);
    close(fd_motX_value);
    unlink(fifo_motorX_value);
    close(fd_motZ_value);
    unlink(fifo_motorZ_value);

    return 0;
}