/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

// I2C Registers
#define DEVID           0x00
#define BW_RATE         0x2C //0x0A for 100Hz
#define INT_ENABLE      0x2E //0x00 for no interrupt
#define DATA_FORMAT     0x31
#define FIFO_CTL        0x38
#define POWER_CTL       0x2D

// Values
#define RATE_100_HZ     0x0A
#define RESET_VAL       0x00
#define MEASURE_MODE    0x08
#define STANDBY_MODE    0x00

void init_accel(struct i2c_client *client)
{
        char sendBuf[2];
        sendBuf[0] = DEVID;
        int retval = i2c_master_send(client, sendBuf,1);
        retval = i2c_master_recv(client,&sendBuf, 1);
        pr_info("DevID : 0x%02X (should be 229 or 0xE5)\n", sendBuf[0]);

        sendBuf[0] = BW_RATE;
        sendBuf[1] = RATE_100_HZ;
        retval = i2c_master_send(client,sendBuf, 2);

        sendBuf[0] = INT_ENABLE;
        sendBuf[1] = RESET_VAL;
        retval = i2c_master_send(client,sendBuf, 2);

        sendBuf[0] = DATA_FORMAT;
        sendBuf[1] = RESET_VAL;
        retval = i2c_master_send(client,sendBuf, 2);

        sendBuf[0] = FIFO_CTL;
        sendBuf[1] = RESET_VAL;
        retval = i2c_master_send(client,sendBuf, 2);

        sendBuf[0] = POWER_CTL;
        sendBuf[1] = MEASURE_MODE;
        retval = i2c_master_send(client,sendBuf, 2);
}

static int accel_read(struct file *fd, char __user *usr, size_t size, loff_t * offset)
{
        return 0;
}

static int accel_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
        return 0;
}

struct accel_device {
    /* Données propres à un périphérique (exemple) */
    int used_channel;
    /* Le périphérique misc correspondant */
    struct miscdevice miscdev;
};

static int accel_probe(struct i2c_client *client,
                     const struct i2c_device_id *id)
{
        // structure de callback
        struct file_operations accel_fops = {
                .read  = accel_read ,
                .unlocked_ioctl = accel_ioctl
        };

        // MISC Framework
        struct accel_device *acceldev;
        int ret;

        /* Alloue la mémoire pour une nouvelle structure foo_device */
        acceldev = devm_kzalloc(&client->dev, sizeof(struct accel_device), GFP_KERNEL);
        if (!acceldev) return -ENOMEM;

        /* Initialise la structure foo_device, par exemple avec les
        informations issues du Device Tree */
        acceldev->used_channel = 0;


        /* Initialise la partie miscdevice de foo_device */
        acceldev->miscdev.minor = MISC_DYNAMIC_MINOR;
        acceldev->miscdev.name = "accelX";
        acceldev->miscdev.fops = &accel_fops;
        acceldev->miscdev.parent = &client->dev; /* (1) */

        i2c_set_clientdata(client,acceldev );

        /* S'enregistre auprès du framework misc */
        ret = misc_register(&acceldev->miscdev);

        // sending init i2c paquets
        init_accel(client);
        return 0;
}

static int accel_remove(struct i2c_client *client)
{
        char sendBuf[2];
        sendBuf[0] = POWER_CTL;
        sendBuf[1] = STANDBY_MODE; //measure mode;
        int retval = i2c_master_send(client,sendBuf, 2);
        pr_info("POWER_CTL, count of byte sent : %i\n", retval);
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

/* basic module code */

/*static int __init accel_init(void)
{
        pr_info("Loaded i2c accel module\n");
        return 0;
}

static void __exit accel_exit(void)
{
        pr_info("Unloaded i2c accel module\n");
}

module_init(accel_init);
module_exit(accel_exit);
*/

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My i2c accel driver");
MODULE_AUTHOR("alopez");
