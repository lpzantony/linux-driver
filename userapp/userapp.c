#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>

#define INPUT_FRAMEWORK

#ifdef INPUT_FRAMEWORK
char *events[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
	[EV_SYN] = "Sync",			[EV_KEY] = "Key",
	[EV_REL] = "Relative",			[EV_ABS] = "Absolute",
	[EV_MSC] = "Misc",			[EV_LED] = "LED",
	[EV_SND] = "Sound",			[EV_REP] = "Repeat",
	[EV_FF] = "ForceFeedback",		[EV_PWR] = "Power",
	[EV_FF_STATUS] = "ForceFeedbackStatus",
};

char *keys[KEY_MAX + 1] = {};
char *absval[5] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat " };
char *relatives[REL_MAX + 1] = {};
char *absolutes[ABS_MAX + 1] = {
	[0 ... ABS_MAX] = NULL,
	[ABS_X] = "X",			[ABS_Y] = "Y",
	[ABS_Z] = "Z",			[ABS_RX] = "Rx",
	[ABS_RY] = "Ry",		[ABS_RZ] = "Rz",
	[ABS_THROTTLE] = "Throttle",	[ABS_RUDDER] = "Rudder",
	[ABS_WHEEL] = "Wheel",		[ABS_GAS] = "Gas",
	[ABS_BRAKE] = "Brake",		[ABS_HAT0X] = "Hat0X",
	[ABS_HAT0Y] = "Hat0Y",		[ABS_HAT1X] = "Hat1X",
	[ABS_HAT1Y] = "Hat1Y",		[ABS_HAT2X] = "Hat2X",
	[ABS_HAT2Y] = "Hat2Y",		[ABS_HAT3X] = "Hat3X",
	[ABS_HAT3Y] = "Hat 3Y",		[ABS_PRESSURE] = "Pressure",
	[ABS_DISTANCE] = "Distance",	[ABS_TILT_X] = "XTilt",
	[ABS_TILT_Y] = "YTilt",		[ABS_TOOL_WIDTH] = "Tool Width",
	[ABS_VOLUME] = "Volume",	[ABS_MISC] = "Misc",
};

char *misc[MSC_MAX + 1] = {};
char *leds[LED_MAX + 1] = {};
char *repeats[REP_MAX + 1] = {};
char *sounds[SND_MAX + 1] = {};
char **names[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
	[EV_SYN] = events,     [EV_KEY] = keys,        [EV_REL] = relatives,			[EV_ABS] = absolutes,  [EV_MSC] = misc,        [EV_LED] = leds,
	[EV_SND] = sounds,     [EV_REP] = repeats,
};
#else
#define X_CHAN		0
#define Y_CHAN		1
#define Z_CHAN		2
#define READ_SIZE       55
#endif

int fd;
void signalHandler( int signum ) {
        printf("\nSIGINT: Ending userapp\n");
        close(fd);
        exit(signum);
}

int main (void)
{
        signal(SIGINT,  signalHandler);

#ifdef INPUT_FRAMEWORK
        //opening accel driver
        fd = open("/dev/input/event0", O_RDWR);
        if(fd == -1){
                perror("could not open /dev/input/event0");
                return -1;
        }

        //reading from accel input driver
        struct input_event ev[3];
        int rd;
        int i = 0;
        while (1) {
                rd = read(fd, ev, sizeof(struct input_event) * 3);

                if (rd < (int) sizeof(struct input_event)) {
                        perror("\nError reading");
                        return 1;
                }

                for (i = 0; i < rd / sizeof(struct input_event); i++)

                        if (ev[i].type == EV_SYN) {
                                printf("Event: time %ld.%06ld, -------------- %s ------------\n",
                                        ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].code ? "Config Sync" : "Report Sync" );
                        } else if (ev[i].type == EV_MSC && (ev[i].code == MSC_RAW || ev[i].code == MSC_SCAN)) {
                                printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %02x\n",
                                        ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                                        events[ev[i].type] ? events[ev[i].type] : "?",
                                        ev[i].code,
                                        names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
                                        ev[i].value);
                        } else {
                                printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
                                        ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                                        events[ev[i].type] ? events[ev[i].type] : "?",
                                        ev[i].code,
                                        names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
                                        ev[i].value);
                        }

        }
#else
        //opening accel driver
        fd = open("/dev/accel", O_RDWR);
        if(fd == -1){
                perror("could not open /dev/accel");
                return -1;
        }
        // ioctl on accel driver
        char chan = Z_CHAN;
        int retval = ioctl( fd, 0, chan);
        if(retval == -1){
                perror("could not use ioctl() on /dev/accel");
                return -1;
        }


        //reading from accel misc driver
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
                        if(msg[i] != 0)
                        printf("sample n° %03i/%03i : %03i\n",i+1, retval, msg[i]);
                }

        }

#endif

        //closing accel driver
        close(fd);
        printf("Accel closed!\n");
        return 0;
}
