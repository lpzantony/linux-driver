/* uart.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/types.h>


#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/types.h>

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mutex.h>


//############################################################################################//
//#######################################[ Constants ]########################################//
//############################################################################################//

#define X_CHAN		0
#define Y_CHAN		1
#define Z_CHAN		2
#define DATA_BUF_SIZE   32
#define LWHPS2FPGA_SIZE 0x1f
#define LWHPS2FPGA_START_ADDR 0xFF200000

// 20 = 0x14  = 10100
// stream, 20 samples = 0x94 = 1001 0100

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My i2c uart driver");
MODULE_AUTHOR("alopez");

struct uart_device {
        struct miscdevice miscdev;
        struct mutex lock;
        char data[3][DATA_BUF_SIZE];
        char data_size;
        wait_queue_head_t queue;
        void __iomem *ptr;
};



#ifndef INPUT_FRAMEWORK
//############################################################################################//
//###################################[ Misc functions ]#######################################//
//############################################################################################//

static ssize_t uart_read(struct file *file, char __user *buf, size_t count, loff_t * f_pos)
{
        struct platform_device *pdev;
        struct miscdevice *miscdev;
        struct uart_device *uartdev;
        miscdev = file->private_data;
        pdev = container_of(miscdev->parent, struct platform_device, dev);
        uartdev = platform_get_drvdata(pdev);

        int retval = 0;
        // If no need to use queue because we know we will not block
        mutex_lock(&uartdev->lock);
        if(count < uartdev->data_size){
                retval = copy_to_user(buf, uartdev->data[X_CHAN], count);
                uartdev->data_size = 0;
                if (retval!=0){    // if true then have success
                        dev_alert(&pdev->dev,
                                " Failed to send %d characters to the user\n",
                                retval);
                        mutex_unlock(&uartdev->lock);
                        return -1;              // Failed
                }
                mutex_unlock(&uartdev->lock);
                return count;
        }
        mutex_unlock(&uartdev->lock);
        //more thant data_size to read, we will block
        int samples_read = 0;
        int to_read = 0;
        while (samples_read < count){
                mutex_lock(&uartdev->lock);
                //determine how many samples we will write to userspace
                if(count - samples_read < uartdev->data_size)
                        to_read = count - samples_read;
                else to_read = uartdev->data_size;

                retval = copy_to_user(buf + samples_read, uartdev->data[X_CHAN], to_read);

                if (retval!=0){    // if true then have success
                        dev_alert(&pdev->dev,
                                " Failed to send %d characters to the user\n",
                                retval);
                        mutex_unlock(&uartdev->lock);
                        return -1;              // Failed
                }
                mutex_unlock(&uartdev->lock);
                samples_read += to_read;
                uartdev->data_size = 0;
                //block
                retval = 1;
                /*====================== pour pouvoir compiler///
                while(retval != 0){
                        retval =  wait_event_interruptible(uartdev->queue,
                                        uartdev->data_size >0);
                        if(retval == -ERESTARTSYS){
                                return -1;
                        }
                }
                */
        }
        return count;

}


#endif

//############################################################################################//
//#################################[ interrupt functions ]####################################//
//############################################################################################//
irqreturn_t uart_handler(int irq, void *dev_id){
        struct platform_device *pdev = (struct platform_device *) dev_id;

        if(irq != platform_get_irq(pdev, 0)){
                dev_alert(&pdev->dev, "Bad irq number!\n");
                return IRQ_NONE;
        }
        // reveiller les taches
        return IRQ_WAKE_THREAD;
}

irqreturn_t uart_thread_handler(int irq, void *dev_id){
        struct platform_device *pdev = (struct platform_device *) dev_id;
        struct uart_device *uartdev = platform_get_drvdata(pdev);
        // Getting sample count in FIFO
        char send_buf = 0;
        unsigned char nb_samples = 1;
        mutex_lock(&uartdev->lock);
        int retval;
        nb_samples = nb_samples & 0x3F;

        if(nb_samples > 32) {
                dev_emerg(&pdev->dev, "ERROR : wrong value read from FIFO_STATUS\n");
#ifndef INPUT_FRAMEWORK
                mutex_unlock(&uartdev->lock);
#endif
                return -1;
        }
        unsigned int i = 0;
        for(i=0; i <nb_samples; i++){
                send_buf = 0;
                char recv_buf[6] = {1,1,1,1,1,1};
                uartdev->data[X_CHAN][i] = recv_buf[1];
                uartdev->data[Y_CHAN][i] = recv_buf[3];
                uartdev->data[Z_CHAN][i] = recv_buf[5];
         }
         uartdev->data_size = nb_samples;
         mutex_unlock(&uartdev->lock);
         /*====================== pour pouvoir compiler///
         wake_up_interruptible(&uartdev->queue);
         */
         return IRQ_HANDLED;
}

//############################################################################################//
//###################################[ Driver functions ]#####################################//
//############################################################################################//

static int request_and_map(struct platform_device *pdev,
                resource_size_t start,
                resource_size_t n ,
                void __iomem *ptr)
{
        struct resource *region;
        struct device *device = &pdev->dev;
        region = devm_request_mem_region(device, start, n, dev_name(device));

        if (region == NULL) {
                dev_emerg(device, "unable to request %s\n", dev_name(device));
                return -EBUSY;
        }
        ptr = devm_ioremap(device, region->start, resource_size(region));

        if (ptr == NULL) {
                dev_emerg(device, "ioremap_nocache of %s failed!",dev_name(device));
                return -ENOMEM;
        }
        return 0;
}

static int uart_probe(struct platform_device *pdev)
{

        // structure de callback
        struct file_operations *uart_fops;
        // Alloue la mémoire pour une nouvelle structure file_operations
        uart_fops = devm_kzalloc(&pdev->dev, sizeof(struct file_operations), GFP_KERNEL);
        if (!uart_fops) return -ENOMEM;
        uart_fops->read  = uart_read;


        // Alloue la mémoire pour une nouvelle structure uart_device
        static struct uart_device *uartdev;
        uartdev = devm_kzalloc(&pdev->dev, sizeof(struct uart_device), GFP_KERNEL);
        if (!uartdev) return -ENOMEM;

        // Initialise la structure uart_device, par exemple avec les
        // informations issues du Device Tree
        // Initialise la partie miscdevice de uart_device
        uartdev->miscdev.minor = MISC_DYNAMIC_MINOR;
        uartdev->miscdev.name = "uart";
        uartdev->miscdev.fops = uart_fops;
        uartdev->miscdev.parent = &pdev->dev;
        mutex_init(&uartdev->lock);

        uartdev->data_size = 0;
        init_waitqueue_head(&uartdev->queue);
        platform_set_drvdata(pdev, uartdev);

         // S'enregistre auprès du framework misc
         misc_register(&uartdev->miscdev);

        int retval = devm_request_threaded_irq(&pdev->dev,
                        platform_get_irq(pdev, 0),
                        uart_handler,
                        uart_thread_handler,
                        0,
                        "uart_INT",
                        pdev);
        if(retval !=0){
               dev_emerg(&pdev->dev,"Could not register interrupt!\n");
               return -1;
        }

        // Taking care of UART lwhps2fpga
        //get_resource not functionnal for the device tree overlay
        //Thus, we will put start address and size as constants from the dts
        /*struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if(res == 0){
                pr_emerg("could not get ressource\n");
                return -1;
        }*/

        retval = request_and_map(pdev, LWHPS2FPGA_START_ADDR, LWHPS2FPGA_SIZE, uartdev->ptr);
        if(retval !=0){
                return -1;
        }
        return 0;
}

static int uart_remove(struct platform_device *pdev)
{
	struct uart_device *uartdev;
	uartdev = platform_get_drvdata(pdev);
	misc_deregister(&uartdev->miscdev);
        return 0;
}

//############################################################################################//
//##################################[ Global structures ]#####################################//
//############################################################################################//
static struct platform_device_id uart_idtable[] = {
    { "uart", 0 },
    { }
};
MODULE_DEVICE_TABLE(platform, uart_idtable);

#ifdef CONFIG_OF
static const struct of_device_id uart_of_match[] = {
    { .compatible = "foo,myuart", },
    {}
};

MODULE_DEVICE_TABLE(of, uart_of_match);
#endif


static struct platform_driver uart_driver = {
        .driver         = {
            .name           = "uart",
            .of_match_table = uart_of_match,
        },
        .id_table       = uart_idtable,
        .probe          = uart_probe,
        .remove         = uart_remove,
};

module_platform_driver(uart_driver);
