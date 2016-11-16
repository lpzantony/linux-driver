/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// I2C Registers
#define DEVID           0x00
#define BW_RATE         0x2C //0x0A for 100Hz
#define INT_ENABLE      0x2E //0x00 for no interrupt
#define DATA_FORMAT     0x31
#define FIFO_CTL        0x38
#define POWER_CTL       0x2D
#define DATAX0		0x32
#define DATAX1		0x33
#define DATAY0		0x34
#define DATAY1		0x35
#define DATAZ0		0x36
#define DATAZ1		0x37

// Values
#define RATE_100_HZ     0x0A
#define RESET_VAL       0x00
#define MEASURE_MODE    0x08
#define STANDBY_MODE    0x00

#define X_CHAN		0
#define Y_CHAN		1
#define Z_CHAN		2

struct accel_device {
        /* Données propres à un périphérique (exemple) */
        int used_channel;
        /* Le périphérique misc correspondant */
        struct miscdevice miscdev;
};

char accel_write_byte(struct i2c_client *client, char reg_addr,	char reg_value)
{
        char send_buf[2] = {reg_addr, reg_value};
        return (i2c_master_send(client,send_buf, 2)==2);
}

char accel_read_byte(struct i2c_client *client, char reg_addr)
{
	char buf = reg_addr;
	i2c_master_send(client, &buf,1);
        i2c_master_recv(client, &buf, 1);
	return buf;
}

void accel_init(struct i2c_client *client)
{
        dev_info(&client->dev,"DevID : 0x%02X (should be 0xE5)\n", accel_read_byte(client, DEVID));
	accel_write_byte(client, BW_RATE, RATE_100_HZ);
	accel_write_byte(client, INT_ENABLE, RESET_VAL);
	accel_write_byte(client, DATA_FORMAT, RESET_VAL);
	accel_write_byte(client, FIFO_CTL, RESET_VAL);
	accel_write_byte(client, POWER_CTL, MEASURE_MODE);
}

static ssize_t accel_read(struct file *file, char __user *buf, size_t count, loff_t * f_pos)
{
        pr_alert("accel_read() called\n");
	/*
        char send_buf = DATAX0;
	char recv_buf[6];
        int retval = i2c_master_send(client, &send_buf,1);
        retval = i2c_master_recv(client,recv_buf, 6);
        if(retval != 6) return -1;
	switch(acceldev->used_channel){
	case X_CHAN:
		retval = recv_buf[1]<<8 + recv_buf[0];
		break;
	case Y_CHAN:
		retval = recv_buf[3]<<8 + recv_buf[2];
		break;
	case Z_CHAN:
		retval = recv_buf[5]<<8 + recv_buf[4];
		break;
	}
	*/


        int retval = 0;
        char msglen = 46;
        char msg[46] = "This msg has been written by the accel_reag()";
        // copy_to_user has the format ( * to, *from, size) and returns 0 on success
        retval = copy_to_user(buf, msg, 46);

        if (retval==0){            // if true then have success
                pr_alert(KERN_INFO "EBBChar: Sent %d characters to the user\n", 46);
                return 0;  // clear the position to the start and return 0
        } else {
                pr_alert(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", retval);
                return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
        }

        return 0;
}

static long accel_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	/*
	switch(arg){
	case X_CHAN:
		acceldev->used_channel = X_CHAN;
		break;
	case Y_CHAN:
		acceldev->used_channel = Y_CHAN;
		break;
	case Z_CHAN:
		acceldev->used_channel = Z_CHAN;
		break;
	default:
		acceldev->used_channel = X_CHAN;
		break;
	}
	*/
        //pr_alert("accel_ioctl() called\n");
        return 0;
}



static int accel_probe(struct i2c_client *client,
                     const struct i2c_device_id *id)
{
        // structure de callback
        struct file_operations *accel_fops;
        // Alloue la mémoire pour une nouvelle structure file_operations
        accel_fops = devm_kzalloc(&client->dev, sizeof(struct file_operations), GFP_KERNEL);
        if (!accel_fops) return -ENOMEM;
        accel_fops->read  = accel_read;
        accel_fops->unlocked_ioctl = accel_ioctl;
        accel_fops->compat_ioctl = accel_ioctl;




	static struct accel_device *acceldev;
        // Alloue la mémoire pour une nouvelle structure accel_device
        acceldev = devm_kzalloc(&client->dev, sizeof(struct accel_device), GFP_KERNEL);
        if (!acceldev) return -ENOMEM;

        // Initialise la structure accel_device, par exemple avec les
        // informations issues du Device Tree
        acceldev->used_channel = X_CHAN;

        // Initialise la partie miscdevice de accel_device
        acceldev->miscdev.minor = MISC_DYNAMIC_MINOR;
        acceldev->miscdev.name = "accelX";
        acceldev->miscdev.fops = accel_fops;
        acceldev->miscdev.parent = &client->dev;

        i2c_set_clientdata(client,acceldev );

        // S'enregistre auprès du framework misc
        misc_register(&acceldev->miscdev);

        // sending init i2c paquets
        accel_init(client);
        return 0;
}

static int accel_remove(struct i2c_client *client)
{
	struct accel_device *acceldev;
	acceldev = i2c_get_clientdata(client);
	misc_deregister(&acceldev->miscdev);

	accel_write_byte(client, POWER_CTL, STANDBY_MODE);
        return 0;
}

static struct i2c_device_id accel_idtable[] = {
    { "accel", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, accel_idtable);

#ifdef CONFIG_OF
static const struct of_device_id accel_of_match[] = {
    { .compatible = "alopez,accel", },
    {}
};

MODULE_DEVICE_TABLE(of, accel_of_match);
#endif

static struct i2c_driver accel_driver = {
    .driver = {
        .name   = "accel",
        .of_match_table = of_match_ptr(accel_of_match),
    },

    .id_table       = accel_idtable,
    .probe          = accel_probe,
    .remove         = accel_remove,
};

module_i2c_driver(accel_driver);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My i2c accel driver");
MODULE_AUTHOR("alopez");
