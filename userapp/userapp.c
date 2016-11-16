#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#define X_CHAN		0
#define Y_CHAN		1
#define Z_CHAN		2

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
        fd = open("/dev/accelX", O_RDWR);
        if(fd == -1){
                perror("could not open /dev/accelX");
                return -1;
        }
        printf("AccelX opened!\n");

        // ioctl on accel driver
        int retval = ioctl( fd, 0, Z_CHAN);
        if(retval == -1){
                perror("could not use ioctl() on /dev/accelX");
                return -1;
        }
        printf("ioctl called! \n");

        //reading from accel driver
        char msg;
        while(1){
                retval = read(fd, &msg, 1);
                if(retval == -1){
                        perror("could not read /dev/accelX");
                        return -1;
                }
                printf("retval = %i, 0x%02X\n", retval, msg);
        }

        //closing accel driver
        close(fd);
        printf("AccelX closed!\n");
        return 0;
}
