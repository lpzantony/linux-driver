/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

//############################################################################################//
//#######################################[ Constants ]########################################//
//############################################################################################//

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

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My i2c accel driver");
MODULE_AUTHOR("alopez");

struct accel_device {
        /* channel currently in use */
        int used_channel;
        struct i2c_client *client;
        /* Le périphérique misc correspondant */
        struct miscdevice miscdev;
};
static struct accel_device *acceldev;

static char used_channel;
module_param(used_channel, byte, S_IRUGO);
MODULE_PARM_DESC(used_channel, "The channel read by read() X=0, Y=1, Z=2");

//############# Functions declaration
char            i2c_write_byte  (struct i2c_client *client, char reg_addr,	char reg_value);
char            i2c_read_byte   (struct i2c_client *client, char reg_addr);
void            i2c_init  (struct i2c_client *client);

//############################################################################################//
//###################################[ Driver functions ]#####################################//
//############################################################################################//

static ssize_t accel_read(struct file *file, char __user *buf, size_t count, loff_t * f_pos)
{
        if(acceldev == 0) {
                pr_alert("accel device has no memory allocated! \n");
                return -1;
        }

        //###### Reading accel X, Y and Z values
        char retval8;
	switch(acceldev->used_channel){
	case X_CHAN:
                retval8 = i2c_read_byte(acceldev->client,DATAX1);
		break;
	case Y_CHAN:
                retval8 = i2c_read_byte(acceldev->client,DATAY1);
		break;
	case Z_CHAN:
                retval8 = i2c_read_byte(acceldev->client,DATAZ1);
		break;
	}

        //######  Sending value to user
        int retval = copy_to_user(buf, &retval8, 1);

        if (retval==0){    // if true then have success
                return 0;
        } else {
                dev_alert(&acceldev->client->dev, " Failed to send %d characters to the user\n", retval);
                return -1;              // Failed
        }
        return 0;
}

static long accel_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	switch(arg){
	case X_CHAN:
		acceldev->used_channel = X_CHAN;
                used_channel = X_CHAN;
		break;
	case Y_CHAN:
		acceldev->used_channel = Y_CHAN;
                used_channel = Y_CHAN;
		break;
	case Z_CHAN:
		acceldev->used_channel = Z_CHAN;
                used_channel = Z_CHAN;
		break;
	default:
		return -1;
	}
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

	//static struct accel_device *acceldev;
        // Alloue la mémoire pour une nouvelle structure accel_device
        acceldev = devm_kzalloc(&client->dev, sizeof(struct accel_device), GFP_KERNEL);
        if (!acceldev) return -ENOMEM;

        // Initialise la structure accel_device, par exemple avec les
        // informations issues du Device Tree
        used_channel = X_CHAN;
        acceldev->used_channel = X_CHAN;
        acceldev->client = client;

        // Initialise la partie miscdevice de accel_device
        acceldev->miscdev.minor = MISC_DYNAMIC_MINOR;
        acceldev->miscdev.name = "accelX";
        acceldev->miscdev.fops = accel_fops;
        acceldev->miscdev.parent = &client->dev;

        i2c_set_clientdata(client,acceldev );

        // S'enregistre auprès du framework misc
        misc_register(&acceldev->miscdev);

        // sending init i2c paquets
        i2c_init(client);
        return 0;
}

static int accel_remove(struct i2c_client *client)
{
	struct accel_device *acceldev;
	acceldev = i2c_get_clientdata(client);
	misc_deregister(&acceldev->miscdev);

	i2c_write_byte(client, POWER_CTL, STANDBY_MODE);
        return 0;
}



//############################################################################################//
//##################################[ Global structures ]#####################################//
//############################################################################################//

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

//############################################################################################//
//#####################################[ I2C FUNCTONS ]#######################################//
//############################################################################################//

char i2c_write_byte(struct i2c_client *client, char reg_addr,	char reg_value)
{
        char send_buf[2] = {reg_addr, reg_value};
        return (i2c_master_send(client,send_buf, 2)==2);
}

char i2c_read_byte(struct i2c_client *client, char reg_addr)
{
	char buf = reg_addr;
	i2c_master_send(client, &buf,1);
        i2c_master_recv(client, &buf, 1);
	return buf;
}

void i2c_init(struct i2c_client *client)
{
        dev_info(&client->dev,"DevID : 0x%02X (should be 0xE5)\n", i2c_read_byte(client, DEVID));
	i2c_write_byte(client, BW_RATE, RATE_100_HZ);
	i2c_write_byte(client, INT_ENABLE, RESET_VAL);
	i2c_write_byte(client, DATA_FORMAT, RESET_VAL);
	i2c_write_byte(client, FIFO_CTL, RESET_VAL);
	i2c_write_byte(client, POWER_CTL, MEASURE_MODE);
}
