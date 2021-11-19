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



  

void print_position_and_instructions()
{

    system("clear");
    printf("X position: %.3f\n", position_x);
    printf("Z position: %.3f\n", position_z);
    printf("\n\nPRESS R TO RESET THE HOIST, PRESS S FOR EMERGENCY STOP\n\n");

    fflush(stdout);
}

void createfifo(const char *path, mode_t mode)
{
    if (mkfifo(path, mode) == -1)
    {
        perror("Error creating fifo:");
    }
}

int main(int argc, char *argv[])
{

    char *fifo_inspection_motorX = "/tmp/inspection_motorX";
    char *fifo_inspection_motorZ = "/tmp/inspection_motorZ";
    char *fifo_motorX_value = "/tmp/motorX_value";
    char *fifo_motorZ_value = "/tmp/motorZ_value";

    createfifo(fifo_inspection_motorX, 0666);
    createfifo(fifo_inspection_motorZ, 0666);
    createfifo(fifo_motorX_value, 0666);
    createfifo(fifo_motorZ_value, 0666);

    fd_motX_value = open(fifo_motorX_value, O_RDONLY);
    printf("1\n");

    fd_motZ_value = open(fifo_motorZ_value, O_RDONLY);
    printf("2\n");

    fd_motX = open(fifo_inspection_motorX, O_WRONLY);
    printf("3\n");

    fd_motZ = open(fifo_inspection_motorZ, O_WRONLY);

    fd_stdin = fileno(stdin);

    printf("ci sono\n");

    struct timeval timeout;
    fd_set readfds;
    fd_set read_stdinfds;
    char input_ch[80];
    char value_from_motor_x[SIZE];
    char value_from_motor_z[SIZE];

    print_position_and_instructions();

    while (1)
    {

        //setting timout microseconds to 0
        timeout.tv_sec=0;
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
            char out_str[80];
            sprintf(out_str, "%d", input_ch[0]);

            switch (input_ch[0])
            {
            case 'R': //R
            case 'r': //r
                print_position_and_instructions();

                printf("RESET PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                write(fd_motZ, out_str, strlen(out_str) + 1);
                break;

            case 'S': //S
            case 's': //s
                print_position_and_instructions();

                printf("EMERGENCY STOP PRESSED\n");
                fflush(stdout);
                write(fd_motX, out_str, strlen(out_str) + 1);
                write(fd_motZ, out_str, strlen(out_str) + 1);
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

        

        switch (select(FD_SETSIZE + 1, &readfds, NULL, NULL, &timeout))
        {
        case 0: //timeout reached, so nothing new
            
            break;

        case -1: //error
            perror("Error inside inspectionConsole");
            fflush(stdout);
            break;
        default: //if something is ready, we read it
            printf("ho letto\n");
            if (FD_ISSET(fd_motX_value, &readfds))
            {
                read(fd_motX_value, value_from_motor_x, SIZE);
                position_x = atof(value_from_motor_x);
            }
            if (FD_ISSET(fd_motZ_value, &readfds))
            {
                read(fd_motZ_value, value_from_motor_z, SIZE);
                position_z = atof(value_from_motor_z);
            }

            print_position_and_instructions();
            break;
        }
    }

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