OUTNAME=userapp

all:
	arm-linux-gnueabihf-gcc userapp.c -o $(OUTNAME)

mount:
	sudo insmod ../accel/accel.ko

unmount:
	sudo rmmod accel

list:
	lsmod

clean:
	rm -rf *.o $(OUTNAME)
