/* accel.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/string.h>

#define INPUT_FRAMEWORK

// For input device
#ifdef INPUT_FRAMEWORK
#include <linux/input.h>
// For misc device
#else
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#endif


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
#define FIFO_STATUS     0x39

// Values
#define RATE_100_HZ     0x0A
#define RESET_VAL       0x00
#define MEASURE_MODE    0x08
#define STANDBY_MODE    0x00
#define FIFO_STREAM_20  0x94
#define INT_WATERMARK   0x02

#define X_CHAN		0
#define Y_CHAN		1
#define Z_CHAN		2

#define DATA_BUF_SIZE   32


// 20 = 0x14  = 10100
// stream, 20 samples = 0x94 = 1001 0100

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My i2c accel driver");
MODULE_AUTHOR("alopez");

struct accel_device {
        /* channel currently in use */
#ifdef INPUT_FRAMEWORK
        struct input_dev * input;
#else
        int used_channel;
        struct miscdevice miscdev;
        struct mutex lock;
        wait_queue_head_t queue;
#endif
        char data[3][DATA_BUF_SIZE];
        char data_size;

};

#ifndef INPUT_FRAMEWORK
static char used_channel;
module_param(used_channel, byte, S_IRUGO);
MODULE_PARM_DESC(used_channel, "The channel read by read() X=0, Y=1, Z=2");
#endif

//############# Functions declaration
char            i2c_write_byte  (struct i2c_client *client, char reg_addr, char reg_value);
char            i2c_read_byte   (struct i2c_client *client, char reg_addr);
void            i2c_init        (struct i2c_client *client);

#ifndef INPUT_FRAMEWORK
//############################################################################################//
//###################################[ Misc functions ]#######################################//
//############################################################################################//

static ssize_t accel_read(struct file *file, char __user *buf, size_t count, loff_t * f_pos)
{
        struct i2c_client *client;
        struct miscdevice *miscdev;
        struct accel_device *acceldev;
        miscdev = file->private_data;
        client = container_of(miscdev->parent, struct i2c_client, dev);
        acceldev = i2c_get_clientdata(client);

        int retval = 0;
        // If no need to use queue because we know we will not block
        mutex_lock(&acceldev->lock);
        if(count < acceldev->data_size){

                switch(acceldev->used_channel){
                case X_CHAN:
                        //retval8 = acceldev->data[X_CHAN][i];
                        retval = copy_to_user(buf, acceldev->data[X_CHAN], count);
                        break;
                case Y_CHAN:
                        //retval8 = acceldev->data[Y_CHAN][i];
                        retval = copy_to_user(buf, acceldev->data[Y_CHAN], count);
                        break;
                case Z_CHAN:
                        //retval8 = acceldev->data[Z_CHAN][i];
                        retval = copy_to_user(buf, acceldev->data[Z_CHAN], count);
                        break;
                }
                acceldev->data_size = 0;
                if (retval!=0){    // if true then have success
                        dev_alert(&client->dev,
                                " Failed to send %d characters to the user\n",
                                retval);
                        mutex_unlock(&acceldev->lock);
                        return -1;              // Failed
                }
                mutex_unlock(&acceldev->lock);
                return count;
        }
        mutex_unlock(&acceldev->lock);
        //more thant data_size to read, we will block
        int samples_read = 0;
        int to_read = 0;
        while (samples_read < count){
                mutex_lock(&acceldev->lock);
                //determine how many samples we will write to userspace
                if(count - samples_read < acceldev->data_size)
                        to_read = count - samples_read;
                else to_read = acceldev->data_size;

        	switch(acceldev->used_channel){
        	case X_CHAN:
                        retval = copy_to_user(buf + samples_read, acceldev->data[X_CHAN], to_read);
        		break;
        	case Y_CHAN:
                        retval = copy_to_user(buf+ samples_read, acceldev->data[Y_CHAN], to_read);
        		break;
        	case Z_CHAN:
                        retval = copy_to_user(buf + samples_read, acceldev->data[Z_CHAN], to_read);
        		break;
        	}
                if (retval!=0){    // if true then have success
                        dev_alert(&client->dev,
                                " Failed to send %d characters to the user\n",
                                retval);
                        mutex_unlock(&acceldev->lock);
                        return -1;              // Failed
                }
                mutex_unlock(&acceldev->lock);
                samples_read += to_read;
                acceldev->data_size = 0;
                //block
                retval = 1;
                while(retval != 0){
                        retval =  wait_event_interruptible(acceldev->queue,
                                        acceldev->data_size >0);
                        if(retval == -ERESTARTSYS){
                                return -1;
                        }
                }
        }
        return count;

}

static long accel_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct i2c_client *client;
        struct miscdevice *miscdev;
        struct accel_device *acceldev;
        miscdev = file->private_data;
        client = container_of(miscdev->parent, struct i2c_client, dev);
        acceldev = i2c_get_clientdata(client);

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

#endif

//############################################################################################//
//#################################[ interrupt functions ]####################################//
//############################################################################################//
irqreturn_t accel_handler(int irq, void *dev_id){
        struct i2c_client *client = (struct i2c_client *) dev_id;

        if(irq != client->irq){
                dev_alert(&client->dev, "Bad irq number!\n");
                return IRQ_NONE;
        }
        // reveiller les taches
        return IRQ_WAKE_THREAD;
}

irqreturn_t accel_thread_handler(int irq, void *dev_id){
        struct i2c_client *client = (struct i2c_client *) dev_id;
        struct accel_device *acceldev = i2c_get_clientdata(client);
        // Getting sample count in FIFO
        char send_buf = FIFO_STATUS;
        unsigned char nb_samples;
#ifndef INPUT_FRAMEWORK
        mutex_lock(&acceldev->lock);
#endif
        int retval = i2c_master_send(client, &send_buf,1);
        retval = i2c_master_recv(client,&nb_samples, 1);
        if(retval != 1) return -1;
        nb_samples = nb_samples & 0x3F;

        if(nb_samples > 32) {
                dev_emerg(&client->dev, "ERROR : wrong value read from FIFO_STATUS\n");
#ifndef INPUT_FRAMEWORK
                mutex_unlock(&acceldev->lock);
#endif
                return -1;
        }
        unsigned int i = 0;
        for(i=0; i <nb_samples; i++){
                send_buf = DATAX0;
                char recv_buf[6];
                retval = i2c_master_send(client, &send_buf,1);
                retval = i2c_master_recv(client,recv_buf, 6);
                if(retval != 6) {
                        dev_emerg(&client->dev, "ERROR : Could not read accel data\n");
                        acceldev->data_size = 0;
#ifndef INPUT_FRAMEWORK
                        mutex_unlock(&acceldev->lock);
#endif
                        return -1;
                }
                acceldev->data[X_CHAN][i] = recv_buf[1];
                acceldev->data[Y_CHAN][i] = recv_buf[3];
                acceldev->data[Z_CHAN][i] = recv_buf[5];
         }
         acceldev->data_size = nb_samples;
#ifdef INPUT_FRAMEWORK
         input_report_abs(acceldev->input, ABS_X, acceldev->data[X_CHAN][i-1]);
         input_report_abs(acceldev->input, ABS_Y, acceldev->data[Y_CHAN][i-1]);
         input_report_abs(acceldev->input, ABS_Z, acceldev->data[Z_CHAN][i-1]);
         input_sync(acceldev->input);
#else
         mutex_unlock(&acceldev->lock);
         wake_up_interruptible(&acceldev->queue);
 #endif
         return IRQ_HANDLED;
}

//############################################################################################//
//###################################[ Driver functions ]#####################################//
//############################################################################################//

static int accel_probe(struct i2c_client *client,
                     const struct i2c_device_id *id)
{
#ifndef INPUT_FRAMEWORK
        // structure de callback
        struct file_operations *accel_fops;
        // Alloue la mémoire pour une nouvelle structure file_operations
        accel_fops = devm_kzalloc(&client->dev, sizeof(struct file_operations), GFP_KERNEL);
        if (!accel_fops) return -ENOMEM;
        accel_fops->read  = accel_read;
        accel_fops->unlocked_ioctl = accel_ioctl;
        accel_fops->compat_ioctl = accel_ioctl;
#endif
        // Alloue la mémoire pour une nouvelle structure accel_device
        static struct accel_device *acceldev;
        acceldev = devm_kzalloc(&client->dev, sizeof(struct accel_device), GFP_KERNEL);
        if (!acceldev) return -ENOMEM;

        // Initialise la structure accel_device, par exemple avec les
        // informations issues du Device Tree
#ifdef INPUT_FRAMEWORK
        // Initialise la partie input device de accel_device
        acceldev->input = devm_input_allocate_device(&client->dev);
        if (!acceldev->input) {
                 dev_emerg(&client->dev, "Not enough memory to allocate input_dev\n");
                 return -ENOMEM;
        }

        acceldev->input->name = "accel";
        acceldev->input->id.bustype = BUS_I2C;
        acceldev->input->evbit[0] = BIT_MASK(EV_ABS);
        acceldev->input->absbit[BIT_WORD(ABS_X)] |= BIT_MASK(ABS_X);
        acceldev->input->absbit[BIT_WORD(ABS_Y)] |= BIT_MASK(ABS_Y);
        acceldev->input->absbit[BIT_WORD(ABS_Z)] |= BIT_MASK(ABS_Z);
        input_set_abs_params(acceldev->input, ABS_X, 0, 255, 0, 0);
        input_set_abs_params(acceldev->input, ABS_Y, 0, 255, 0, 0);
        input_set_abs_params(acceldev->input, ABS_Z, 0, 255, 0, 0);
#else
        // Initialise la partie miscdevice de accel_device
        used_channel = X_CHAN;
        acceldev->used_channel = X_CHAN;
        acceldev->miscdev.minor = MISC_DYNAMIC_MINOR;
        acceldev->miscdev.name = "accel";
        acceldev->miscdev.fops = accel_fops;
        acceldev->miscdev.parent = &client->dev;
        mutex_init(&acceldev->lock);
        init_waitqueue_head(&acceldev->queue);
#endif
        acceldev->data_size = 0;

        i2c_set_clientdata(client, acceldev);


#ifdef INPUT_FRAMEWORK
        if (input_register_device(acceldev->input)) {
                 dev_emerg(&client->dev, "Failed to register input device\n");
                 input_free_device(acceldev->input);
                 return ENOMEM;
         }
 #else
         // S'enregistre auprès du framework misc
         misc_register(&acceldev->miscdev);
 #endif

        int retval = devm_request_threaded_irq(&client->dev,
                        client->irq,
                        accel_handler,
                        accel_thread_handler,
                        0,
                        "accel_INT",
                        client);
        if(retval !=0){
               dev_emerg(&client->dev,"Could not register interrupt!\n");
               return -1;
        }

        // sending init i2c paquets
        i2c_init(client);
        return 0;
}

static int accel_remove(struct i2c_client *client)
{
	struct accel_device *acceldev;
	acceldev = i2c_get_clientdata(client);
#ifdef INPUT_FRAMEWORK
        input_unregister_device(acceldev->input);
#else
	misc_deregister(&acceldev->miscdev);
#endif
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

void i2c_init(struct i2c_client *client)
{
        dev_info(&client->dev,"DevID : 0x%02X (should be 0xE5)\n", i2c_read_byte(client, DEVID));
	i2c_write_byte(client, BW_RATE, RATE_100_HZ);
	i2c_write_byte(client, INT_ENABLE, INT_WATERMARK);
	i2c_write_byte(client, DATA_FORMAT, RESET_VAL);
	i2c_write_byte(client, FIFO_CTL, FIFO_STREAM_20);
	i2c_write_byte(client, POWER_CTL, MEASURE_MODE);
        // Update IRQ, just in case
        i2c_read_byte(client, DATAX0);
}

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
