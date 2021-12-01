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

float position = 0;             //position of the X motor
float max_x = 1;                //maximum position of X motor
float movement_distance = 0.10; //the amount of movement made after receiving a command
float movement_time = 1;        //the amount of seconds needed to do the movement

int fd_inspection;
int fd_command;
int fd_motorX;

char buffer[SIZE];
char last_input_command[SIZE];
char last_input_inspection[SIZE];

int pid_watchdog;

FILE* log_file;

void sigusr1_handler(int sig)
{
    fprintf(log_file,"motorX: emergency stop received\n");
    sprintf(buffer, "%f", position);
    write(fd_motorX, buffer, strlen(buffer) + 1);
    strcpy(last_input_inspection, "");
    strcpy(last_input_command, "");
}

void sigusr2_handler(int sig)
{
    fprintf(log_file,"motorXs: reset received from watchdog\n");
    sprintf(last_input_inspection, "%d", 'r');
}

int main(int argc, char *argv[])
{

    //randomizing seed for random error generator
    srand(time(NULL));
    fflush(stdout);

    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);

    //pipe file path
    char *fifo_command_motorX = "/tmp/command_motorX";
    char *fifo_inspection_motorX = "/tmp/inspection_motorX";
    char *fifo_motorX_value = "/tmp/motorX_value";
    char *fifo_watchdog_pid = "/tmp/watchdog_pid_x";
    char *fifo_motX_pid = "/tmp/pid_x";
    char *fifo_motX_pid_inspection = "/tmp/pid_x_i";
    mkfifo(fifo_inspection_motorX, 0666);
    mkfifo(fifo_command_motorX, 0666);
    mkfifo(fifo_motorX_value, 0666);
    mkfifo(fifo_watchdog_pid, 0666);
    mkfifo(fifo_motX_pid, 0666);
    mkfifo(fifo_motX_pid_inspection, 0666);

    //getting watchdog pid
    int fd_watchdog_pid = open(fifo_watchdog_pid, O_RDONLY);
    read(fd_watchdog_pid, buffer, SIZE);
    pid_watchdog = atoi(buffer);
    close(fd_watchdog_pid);

    struct timeval timeout;
    fd_set readfds;

    float random_error;
    float movement;
    fd_command = open(fifo_command_motorX, O_RDONLY);
    fd_inspection = open(fifo_inspection_motorX, O_RDONLY);

    //writing own pid for inspection console
    int fd_motX_pid_i = open(fifo_motX_pid_inspection, O_WRONLY);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_motX_pid_i, buffer, SIZE);
    close(fd_motX_pid_i);

    //writing own pid
    int fd_motX_pid = open(fifo_motX_pid, O_WRONLY);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_motX_pid, buffer, SIZE);
    close(fd_motX_pid);

    fd_motorX = open(fifo_motorX_value, O_WRONLY);

    log_file = fopen("./../logs/log.txt", "a");

    system("clear");
    while (1)
    {

        //setting timout microseconds to 0
        timeout.tv_usec = 100000;
        //initialize with an empty set the file descriptors set
        FD_ZERO(&readfds);

        //add the selected file descriptor to the selected fd_set
        FD_SET(fd_command, &readfds);
        FD_SET(fd_inspection, &readfds);

        //generating a small random error between -0.02 and 0.02
        random_error = (float)(-20 + rand() % 40) / 1000;
        //select return -1 in case of error, 0 if timeout reached, or the number of ready descriptors
        switch (select(FD_SETSIZE + 1, &readfds, NULL, NULL, &timeout))
        {
        case 0: //timeout reached, so nothing new
            switch (atoi(last_input_command))
            {
            case 'J':
            case 'j':
                // left

                movement = -movement_distance + random_error;
                if (position + movement < 0)
                {
                    position = 0;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    strcpy(last_input_command, "");
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                fprintf(log_file, "X = %f\n", position);
                kill(pid_watchdog, SIGUSR1);
                break;

            case 'L':
            case 'l':
                //right

                movement = movement_distance + random_error;
                if (position + movement > max_x)
                {
                    position = max_x;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    strcpy(last_input_command, "");
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                fprintf(log_file, "X = %f\n", position);
                kill(pid_watchdog, SIGUSR1);
                break;
            case 'X':
            case 'x':
                //stop x
                write(fd_motorX, buffer, strlen(buffer) + 1);
                fprintf(log_file, "X = %f\n", position);
                kill(pid_watchdog, SIGUSR1);
                strcpy(last_input_command, "");
                sleep(movement_time);
                break;
            default:
                break;
            }

            if (atoi(last_input_inspection) == 'r' || atoi(last_input_inspection) == 'R')
            {
                movement = -(5 * movement_distance) + random_error;
                if (position + movement <= 0)
                {
                    position = 0;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    strcpy(last_input_inspection, "");
                    kill(pid_watchdog, SIGUSR1);
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    kill(pid_watchdog, SIGUSR1);
                    sleep(movement_time);
                }
                fprintf(log_file, "X = %f\n", position);
            }
            break;
        case -1: //error
            fprintf(log_file, "Error inside motorX");
            break;
        default: //if something is ready, we read it
            if (FD_ISSET(fd_command, &readfds))
            {

                read(fd_command, last_input_command, SIZE);
                fprintf(log_file, "motorX: instruction from command console\n");

            }
            if (FD_ISSET(fd_inspection, &readfds))
            {
                read(fd_inspection, last_input_inspection, SIZE);
                fprintf(log_file, "motorX: instruction from inpsection console\n");
                strcpy(last_input_command, "");
            }
            break;
        }
    }
    fclose(log_file);
    close(fd_command);
    unlink(fifo_command_motorX);
    close(fd_inspection);
    unlink(fifo_inspection_motorX);
    close(fd_motorX);
    unlink(fifo_motorX_value);

    return 0;
}