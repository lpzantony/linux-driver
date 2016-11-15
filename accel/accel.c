/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>

/* I2C specific code */
#define DEVID           0x00
#define BW_RATE         0x2C //0x0A for 100Hz
#define INT_ENABLE      0x2E //0x00 for no interrupt
#define DATA_FORMAT     0x31
#define FIFO_CTL        0x38
#define POWER_CTL       0x2D

#define RATE_100_HZ     0x0A
#define RESET_VAL       0x00
#define MEASURE_MODE    0x08
#define STANDBY_MODE    0x00

static char *testmsg = "Hello i2c!";
module_param(testmsg, charp, S_IRUGO);
MODULE_PARM_DESC(testmsg, "a debug message");

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
static int accel_probe(struct i2c_client *client,
                     const struct i2c_device_id *id)
{
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
