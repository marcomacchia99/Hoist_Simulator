#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SIZE 80

float position = 0; //position of the X motor
float max_x = 1; //maximum position of X motor
float movement_distance = 0.20; //the amount of movement made after receiving a command

int fd_inspection;
int fd_command;
int fd_motorX;

int main (int argc, char * argv[])
{
    //randomizing seed for random error generator
    srand(time(NULL));

    //pipe file path
    char *fifo_command_motorX = "/tmp/command_motorX";
    //char *fifo_motorX_inspection = "/tmp/motorX_inspection";
    char *fifo_inspection_motorX = "/tmp/inspection_motorX";
    mkfifo(fifo_command_motorX,0666);
    //mkfifo(fifo_motorX_inspection,0666);
    mkfifo(fifo_inspection_motorX,0666);


    char last_input_command[SIZE];
    char last_input_inspection[SIZE];


    struct timeval timeout;
    fd_set readfds;
    char buffer[80];

    while(1){
        fd_inspection = open(fifo_inspection_motorX,O_RDWR);
        fd_command = open(fifo_command_motorX,O_RDWR);
        //fd_motorX = open(fifo_motorX_inspection,O_WRONLY);



        //setting timout microseconds to 0
        timeout.tv_usec=0;
        
        //initialize with an empty set the file descriptors set
        FD_ZERO(&readfds);

        //add the selected file descriptor to the selected fd_set
        FD_SET(fd_command,&readfds);

        /////////////////////////////////////////////dobbiamo mettere fd set anche per inspection?

        //generating a small random error between -0.02 and 0.02
        int random_error = (-2 + rand() % 4)/100; 

        //select return -1 in case of error, 0 if timeout reached, or the number of ready descriptors
        switch (select(FD_SETSIZE+1,&readfds,NULL,NULL,&timeout))
        {
        default: //if something is ready, we read it
            read(fd_command,last_input_command,SIZE);
            read(fd_inspection,last_input_inspection,SIZE);
            break;

        case 0: //timeout reached, so nothing new    
            switch (atoi(last_input_command))
            {
            case 74: 
            case 106:
            // left  

            float movement = - movement_distance + random_error;
            if(position+movement<0){
                sprintf(buffer,"%s","no left");
                write(fd_inspection,buffer,strlen(buffer)+1);
            }
            else {
                position+=movement;
                sprintf(buffer,"%f",position);
                write(fd_command,buffer,strlen(buffer)+1);
            }
                break;

            case 76:
            case 108:
            //right
                
            float movement = movement_distance + random_error;
            if(position+movement<max_x){
                sprintf(buffer,"%s","no right");
                write(fd_command,buffer,strlen(buffer)+1);
            }
            else {
                position+=movement;
                sprintf(buffer,"%f",position);
                write(fd_inspection,buffer,strlen(buffer)+1);
            }    
                break;
            default:
                break;
            }
            break;

        case -1: //error
            perror("Error inside motorX: ");
            fflush(stdout);
            break;

        
        }

    }

    return 0;
}