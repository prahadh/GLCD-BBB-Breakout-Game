#include<linux/gpio.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/delay.h>
#include<linux/version.h>

//#define DEBUG
#define RS 60
#define E  48
#define CS1 49
#define CS2 68
#define RST 112
#define DATA_PIN_0 44
#define DATA_PIN_1 26
#define DATA_PIN_2 47
#define DATA_PIN_3 46
#define DATA_PIN_4 27
#define DATA_PIN_5 65
#define DATA_PIN_6 61
#define DATA_PIN_7 20

#define DISPLAY_ON 0x3F
#define DISPLAY_OFF 0x3E

int lcd_pin_array[] = {RS,E,CS1,CS2,RST,DATA_PIN_0,DATA_PIN_1,DATA_PIN_2,DATA_PIN_3,DATA_PIN_4,DATA_PIN_5,DATA_PIN_6,DATA_PIN_7};

//extern int button_pressed;
static struct task_struct *running_task = NULL;
EXPORT_SYMBOL(running_task);

MODULE_AUTHOR("Prahadheeswaran M & Arpit Rastogi");
MODULE_DESCRIPTION("LCD driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(" 0.1");

void lcd_data_pin_set(int );
void lcd_command(int );
void lcd_data(int );
void window_select(int );
void display_reset(void );

static long lcd_ioctl(struct file *f, unsigned int cmd, unsigned long arg);
static int lcd_open(struct inode *inod, struct file *fil);


//structure containing device operation
static struct file_operations fops=
{
.open=lcd_open,   //pointer to device open function
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = lcd_ioctl
#else
    .unlocked_ioctl = lcd_ioctl
#endif
};

static int __init lcd_init(void)
{
	int lcd_pin_index;
	int t=register_chrdev(95,"mylcd",&fops);
	if(t<0)
	printk(KERN_ALERT "[%d] : mylcd device registration failed.", t);
	else
	{
		// Check if all the gpio values for lcd pins are valid
		for(lcd_pin_index = 0 ; lcd_pin_index < 13; lcd_pin_index++) 
		{
			if (!gpio_is_valid(lcd_pin_array[lcd_pin_index]))
			{
			printk(KERN_INFO"GPIO_TEST: invalid button GPIO %d\n",lcd_pin_array[lcd_pin_index]);
			return -ENODEV;
			}
		}

		for(lcd_pin_index = 0 ; lcd_pin_index < 13; lcd_pin_index++)
		{
		gpio_request(lcd_pin_array[lcd_pin_index], "sysfs");      // Request for all the lcd gpio pins
		gpio_direction_output(lcd_pin_array[lcd_pin_index],0);    // Assign each lcd gpio pin direction as output and set to 0 	
		}
		printk(KERN_ALERT "mylcd device registered\n");
	}
	return 0;
}

static void __exit lcd_exit(void)
{
	int lcd_pin_index;
	for(lcd_pin_index = 0; lcd_pin_index < 5; lcd_pin_index++)
	{
   		gpio_free(lcd_pin_array[lcd_pin_index]);
	}
}

void lcd_command(int command)
{
	//printk("command : %d\n",command);
	lcd_data_pin_set(command);
	gpio_set_value(RS,0);
	gpio_set_value(E,1);
	udelay(5);
	gpio_set_value(E,0);
	udelay(5);
}

void lcd_data(int data)
{
	//printk("data : %d\n",data);
	lcd_data_pin_set(data);
	gpio_set_value(RS,1);
	gpio_set_value(E,1);
	udelay(5);
	gpio_set_value(E,0);
	udelay(5);
}

void window_select(int arg)
{
	switch(arg)
	{
		case 1: // left window
			#ifdef DEBUG
				printk(KERN_INFO "window_select:left_window arg = %d\n",arg);
			#endif

			gpio_set_value(CS1,1);
			gpio_set_value(CS2,0);

			#ifdef DEBUG
				printk(KERN_INFO "window_select:left_window data_pin_CS1 = %d, before call CS1 = 1, CS2 = 0 \n",gpio_get_value(CS1));
				printk(KERN_INFO "window_select:left_window data_pin_CS2 = %d \n",gpio_get_value(CS2));
			#endif

			break;
		case 2: // right window
			#ifdef DEBUG
				printk(KERN_INFO "window_select:right_window arg = %d, before call CS1 = 0, CS2 = 1\n",arg);
			#endif

			gpio_set_value(CS1,0);
			gpio_set_value(CS2,1);

			#ifdef DEBUG
				printk(KERN_INFO "window_select:right_window data_pin_CS1 = %d \n",gpio_get_value(CS1));
				printk(KERN_INFO "window_select:right_window data_pin_CS2 = %d \n",gpio_get_value(CS2));
			#endif

			break;
		case 3: // both window

			#ifdef DEBUG
				printk(KERN_INFO "window_select:both_window arg = %d, CS1 = 1, CS2 = 1\n",arg);
			#endif

		 	gpio_set_value(CS1,1);
			gpio_set_value(CS2,1);

			#ifdef DEBUG
				printk(KERN_INFO "window_select: both_window data_pin_CS1 = %d \n",gpio_get_value(CS1));
				printk(KERN_INFO "window_select: both_window data_pin_CS2 = %d \n",gpio_get_value(CS2));
			#endif

			break;
		default: // left window
			#ifdef DEBUG
				printk(KERN_INFO "window_select:no_window arg = %d before call CS1 = 0, CS2 = 0 \n",arg);
			#endif

			gpio_set_value(CS1,0);
			gpio_set_value(CS2,0);

			#ifdef DEBUG
				printk(KERN_INFO "window_select:no_window data_pin_CS1 = %d \n",gpio_get_value(CS1));
				printk(KERN_INFO "window_select:no_window data_pin_CS2 = %d \n",gpio_get_value(CS2));
			#endif

			break;
	}
}

void display_reset()
{
	gpio_set_value(RST,0);
	mdelay(10);
	gpio_set_value(RST,1);
	mdelay(10);
}
void lcd_data_pin_set(int value)
{
	int pin_iter;

	#ifdef DEBUG
		printk("lcd_data_pin_set entered \n\n");
	#endif

	for(pin_iter = 0; pin_iter < 8; pin_iter++)
		gpio_set_value(lcd_pin_array[pin_iter+5],((value>>pin_iter) & 0x01));

	#ifdef DEBUG
		for(pin_iter = 0; pin_iter < 8; pin_iter++)
			printk(KERN_INFO "data_pin_%d = %d \n",pin_iter,gpio_get_value(lcd_pin_array[pin_iter+2]));
	#endif
}

static int lcd_open(struct inode *inod, struct file *fil)
{
/*
	mdelay(40);
   	lcd_command(0x30);
   	mdelay(2);
   	lcd_command(0x30);
   	mdelay(2);
   	lcd_command(0x30);
   	mdelay(1);
   	lcd_command(0x38); // Data length is 8-bit, number of lines is 2.
   	lcd_command(0x08); // Display is off
   	lcd_command(0x01); // Clear display and return cursor to home
   	lcd_command(0x06); // Increment cursor on each write, do not shift display
   	// Initialization is over

   	lcd_command(0x0f); // Display is on, Cursor is on and blink cursor
   	
*/
	display_reset();
	window_select(2);
	mdelay(1);
	lcd_command(DISPLAY_ON);
	lcd_command(0xC0);
	mdelay(1);

	running_task = get_current();
    	printk("KERN_ALERT mylcd device opened");
    return 0;
}


static long lcd_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	//mdelay(5000);
	#ifdef DEBUG
		printk(KERN_INFO "lcd_ioctl: cmd = %d,arg = %ld\n", cmd,arg);
	#endif 

	if(cmd == 0)
	{
		#ifdef DEBUG
			printk(KERN_INFO "lcd_ioctl:lcd_command cmd = %d,arg = %ld\n", cmd,arg);
		#endif

		lcd_command((int ) arg);
	}
	else if(cmd == 1)
	{
		#ifdef DEBUG
			printk(KERN_INFO "lcd_ioctl:lcd_data cmd = %d,arg = %ld\n", cmd,arg);
		#endif

		lcd_data((int ) arg);
	}
	else if(cmd == 4)
	{
		#ifdef DEBUG
			printk(KERN_INFO "lcd_ioctl:window_select cmd = %d,arg = %ld\n", cmd,arg);
		#endif

		window_select((int ) arg);
	}
	else if(cmd == 3)
	{
		#ifdef DEBUG
			printk(KERN_INFO "lcd_ioctl:display_reset cmd = %d,arg = %ld\n", cmd,arg);
		#endif

		display_reset();
	}

	#ifdef DEBUG
	else
		printk("invalid operation !!\n");
	#endif

	return 0;	
}


module_init(lcd_init);
module_exit(lcd_exit);
