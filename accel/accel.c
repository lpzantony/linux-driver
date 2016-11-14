/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init accel_init(void)
{

  return 0;
}

static void __exit accel_exit(void)
{
  pr_info("Ending i2c accel module\n");
}

module_init(accel_init);
module_exit(accel_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My i2c accel driver");
MODULE_AUTHOR("alopez");