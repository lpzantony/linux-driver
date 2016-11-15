/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>

/* I2C specific code */

static char *testmsg = "Hello i2c!";
module_param(testmsg, charp, S_IRUGO);
MODULE_PARM_DESC(testmsg, "a debug message");

static int accel_probe(struct i2c_client *client,
                     const struct i2c_device_id *id)
{
    /* ... */
    pr_info("Accel prob called");
    return 0;
}

static int accel_remove(struct i2c_client *client)
{
    /* ... */
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
