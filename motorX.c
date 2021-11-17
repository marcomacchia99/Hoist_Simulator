#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SIZE 80

float position = 0;             //position of the X motor
float max_x = 1;                //maximum position of X motor
float movement_distance = 0.10; //the amount of movement made after receiving a command
float movement_time = 1;        //the amount of seconds needed to do the movement

int fd_inspection;
int fd_command;
int fd_motorX;

int main(int argc, char *argv[])
{
    //randomizing seed for random error generator
    srand(time(NULL));

    //pipe file path
    char *fifo_command_motorX = "/tmp/command_motorX";
    char *fifo_inspection_motorX = "/tmp/inspection_motorX";
    char *fifo_motorX_value = "/tmp/motorX_value";
    //mkfifo(fifo_motorX_inspection,0666);
    mkfifo(fifo_command_motorX, 0666);
    mkfifo(fifo_inspection_motorX, 0666);

    //communicate pid to other processes
    char pid[SIZE];
    fd_command = open(fifo_command_motorX, O_WRONLY);
    fd_inspection = open(fifo_inspection_motorX, O_WRONLY);
    sprintf(pid, "%d", (int)getpid());
    write(fd_command, pid, strlen(pid) + 1);
    write(fd_inspection, pid, strlen(pid) + 1);
    close(fd_command);
    close(fd_inspection);

    char last_input_command[SIZE];
    char last_input_inspection[SIZE];

    struct timeval timeout;
    fd_set readfds;
    char buffer[80];

    while (1)
    {
        ////////////fuori dal while?
        fd_inspection = open(fifo_inspection_motorX, O_RDONLY);
        fd_command = open(fifo_command_motorX, O_RDONLY);
        fd_motorX = open(fifo_motorX_value, O_WRONLY);

        //setting timout microseconds to 0
        timeout.tv_usec = 0;

        //initialize with an empty set the file descriptors set
        FD_ZERO(&readfds);

        //add the selected file descriptor to the selected fd_set
        FD_SET(fd_command, &readfds);
        FD_SET(fd_inspection, &readfds);

        //generating a small random error between -0.02 and 0.02
        int random_error = (-20 + rand() % 40) / 1000;

        //select return -1 in case of error, 0 if timeout reached, or the number of ready descriptors
        switch (select(FD_SETSIZE + 1, &readfds, NULL, NULL, &timeout))
        {
        case 0: //timeout reached, so nothing new
            switch (atoi(last_input_command))
            {
            case 74:
            case 106:
                // left

                float movement = -movement_distance + random_error;
                if (position + movement < 0)
                {
                    position = 0;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                break;

            case 76:
            case 108:
                //right

                float movement = movement_distance + random_error;
                if (position + movement < max_x)
                {
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                break;
            default:
                break;
            }
            break;

            switch (atoi(last_input_inspection))
            {
            case 82:
            case 114:
                //reset
                while (position == 0)
                {
                    float movement = -(5 * movement_distance) + random_error;
                    if (position + movement <= 0)
                    {
                        position = 0;
                        printf(buffer, "%f", position);
                        write(fd_motorX, buffer, strlen(buffer) + 1);
                        sleep(movement_time);
                    }
                    else
                    {
                        position += movement;
                        sprintf(buffer, "%f", position);
                        write(fd_motorX, buffer, strlen(buffer) + 1);
                        sleep(movement_time);
                    }
                }
                break;
            case 76:
            case 108:
                //emergency stop

                sprintf(buffer, "%f", position);
                write(fd_motorX, buffer, strlen(buffer) + 1);
                sleep(movement_time);
                break;
            default:
                break;
            }

        case -1: //error
            perror("Error inside motorX: ");
            fflush(stdout);
            break;
        default: //if something is ready, we read it
            if (FD_ISSET(fd_command, &readfds))
                read(fd_command, last_input_command, SIZE);
            if (FD_ISSET(fd_inspection, &readfds))
                read(fd_inspection, last_input_inspection, SIZE);
            break;
        }

        close(fd_command);
        close(fd_inspection);
        close(fd_motorX);
    }

    return 0;
}