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

void createfifo(const char *path, mode_t mode)
{
    if (mkfifo(path, mode) == -1)
    {
        perror("Error creating fifo");
    }
}

void sigusr1_handler(int sig)
{
    printf("EMERGENCY BUTTON PRESSED\n");
    sprintf(buffer, "%f", position);
    printf("STOP %.3f\n", position);
    write(fd_motorX, buffer, strlen(buffer) + 1);
    strcpy(last_input_inspection, "");
    strcpy(last_input_command, "");
    printf("inpsection messo a : %d\n", atoi(last_input_inspection));
}

void sigusr2_handler(int sig)
{
    printf("RESET RECEIVED\n");
    sprintf(last_input_inspection,"%d",'r');
    printf("atoi: %d\n",atoi(last_input_inspection));
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
    createfifo(fifo_inspection_motorX, 0666);
    createfifo(fifo_command_motorX, 0666);
    createfifo(fifo_motorX_value, 0666);
    createfifo(fifo_watchdog_pid, 0666);
    createfifo(fifo_motX_pid,0666);

    
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
    fd_motorX = open(fifo_motorX_value, O_WRONLY);
    fd_inspection = open(fifo_inspection_motorX, O_RDONLY);


    //writing own pid for inspection console
    sprintf(buffer, "%d", (int)getpid());
    write(fd_motorX, buffer, strlen(buffer) + 1);
    close(fd_motorX);

    
    //writing own pid
    int fd_motX_pid = open(fifo_motX_pid, O_WRONLY);
    sprintf(buffer, "%d", (int)getpid());
    write(fd_motX_pid, buffer, SIZE);
    close(fd_motX_pid);

    fd_motorX = open(fifo_motorX_value, O_WRONLY);
    

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
                    printf("%.3f\n", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    strcpy(last_input_command,"");
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    printf("%.3f\n", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                kill(pid_watchdog,SIGUSR1);
                break;

            case 'L':
            case 'l':
                //right

                movement = movement_distance + random_error;
                if (position + movement > max_x)
                {
                    position = max_x;
                    sprintf(buffer, "%f", position);
                    printf("%.3f\n", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    strcpy(last_input_command,"");
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    printf("%.3f\n", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    sleep(movement_time);
                }
                kill(pid_watchdog,SIGUSR1);
                break;
            case 'X':
            case 'x':
                //stop x
                printf("%.3f\n", position);
                fflush(stdout);
                write(fd_motorX, buffer, strlen(buffer) + 1);
                kill(pid_watchdog,SIGUSR1);
                strcpy(last_input_command,"");
                sleep(movement_time);
                break;
            default:
                break;
            }

            
            switch (atoi(last_input_inspection))
            {
            case 'R':
            case 'r':
                printf("entro qui dentro\n");
                //reset
                // while (position > 0)
                // {
                movement = -(5 * movement_distance) + random_error;
                if (position + movement <= 0)
                {
                    position = 0;
                    sprintf(buffer, "%f", position);
                    printf("SPRINTF %.3f\n", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    strcpy(last_input_inspection, "");
                    kill(pid_watchdog,SIGUSR1);
                    sleep(movement_time);
                }
                else
                {
                    position += movement;
                    sprintf(buffer, "%f", position);
                    printf("%.3f\n", position);
                    write(fd_motorX, buffer, strlen(buffer) + 1);
                    kill(pid_watchdog,SIGUSR1);
                    sleep(movement_time);
                }
                // sprintf(buffer, "%f", position);
                // write(fd_motorX, buffer, strlen(buffer) + 1);
                //}
                break;
            case 'S':
            case 's':
                //emergency stop
                break;
            default:
                break;
            }
            break;
        case -1: //error

            perror("Error inside motorX: ");
            fflush(stdout);
            break;
        default: //if something is ready, we read it
            if (FD_ISSET(fd_command, &readfds))
            {

                read(fd_command, last_input_command, SIZE);
            }
            if (FD_ISSET(fd_inspection, &readfds))
            {
                read(fd_inspection, last_input_inspection, SIZE);
                printf("lii: >>>%s<<<",last_input_inspection);
                strcpy(last_input_command, "");
            }
            break;
        }
    }
    close(fd_command);
    unlink(fifo_command_motorX);
    close(fd_inspection);
    unlink(fifo_inspection_motorX);
    close(fd_motorX);
    unlink(fifo_motorX_value);

    return 0;
}