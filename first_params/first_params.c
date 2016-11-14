/* first_params.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static char *message = "Hello world!";
module_param(message, charp, S_IRUGO);
MODULE_PARM_DESC(message, "The message to print");

static char the_number = 42;
module_param(the_number, byte, S_IRUGO);
MODULE_PARM_DESC(the_number, "the answer to life the universe and everything");

static char is_true = 1;
module_param(is_true, bool, S_IRUGO);
MODULE_PARM_DESC(is_true, "tells if true is true");

static int __init first_params_init(void)
{
	pr_info("The message : %s\n", message);
	pr_info("The number  : %i\n", the_number);
	pr_info("The truth   : %i\n", is_true);
  return 0;
}

static void __exit first_params_exit(void)
{
  pr_info("Bye\n");
}

module_init(first_params_init);
module_exit(first_params_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My first module with parameters");
MODULE_AUTHOR("The Doctor");