#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main (void)
{
        //test msg
        printf("HELLL YEAHHHH!\n");

        //opening accel driver
        int fd = open("/dev/accelX", O_RDWR);
        if(fd == -1){
                perror("could not open /dev/accelX");
                return -1;
        }
        printf("AccelX opened!\n");


        //reading from accel driver
        ssize_t msglen = 46;
        char msg[46]="";
        int retval = read(fd, msg, 46);
        if(retval == -1){
                perror("could not open /dev/accelX");
                return -1;
        }
        printf("retval = %i, %s\n", retval, msg);


        // ioctl on accel driver
        retval = ioctl( fd, 0, 0);
        if(retval == -1){
                perror("could not use ioctl() on /dev/accelX");
                return -1;
        }
        printf("ioctl called! \n");

        //closing accel driver
        close(fd);
        printf("AccelX closed!\n");
        return 0;
}
