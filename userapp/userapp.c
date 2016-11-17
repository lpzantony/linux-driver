#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define X_CHAN		0
#define Y_CHAN		1
#define Z_CHAN		2

#define READ_SIZE       55
int fd;
void signalHandler( int signum ) {
        printf("\nSIGINT: Ending userapp\n");
        close(fd);
        exit(signum);
}

int main (void)
{
        signal(SIGINT,  signalHandler);
        //opening accel driver
        fd = open("/dev/accel", O_RDWR);
        if(fd == -1){
                perror("could not open /dev/accelX");
                return -1;
        }

        // ioctl on accel driver
        char chan = Z_CHAN;
        int retval = ioctl( fd, 0, chan);
        if(retval == -1){
                perror("could not use ioctl() on /dev/accel");
                return -1;
        }

        //reading from accel driver
        char msg[READ_SIZE] = "";
        while(1){
                retval = read(fd, msg, READ_SIZE);
                if(retval == -1){
                        perror("could not read /dev/accel");
                        return -1;
                }
                if(retval != READ_SIZE){
                        perror("amount of samples read not expected ");
                        return -1;
                }
                int i=0;
                for(i=0; i<retval; i++){
                        //if(msg[i] != 0)
                        printf("sample n° %03i/%03i : %03i\n",i+1, retval, msg[i]);
                }

        }

        //closing accel driver
        close(fd);
        printf("Accel closed!\n");
        return 0;
}
