
#include<linux/gpio.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/signal.h>
#include<linux/ioctl.h>
#include<linux/syscalls.h>

// #define DEBUG
#define BUTTON_0 66		// GPIO_P8_7
#define BUTTON_1 67 	// GPIO_P8_8
#define BUTTON_2 69 	// GPIO_P8_9

#define SIG_BUTTON0 20
#define SIG_BUTTON1 21
#define SIG_BUTTON2 22
//#define SIG_BUTTON3 23
//#define SIG_BUTTON4 24


MODULE_AUTHOR("Prahadheeswaran M & Arpit Rastogi");
MODULE_DESCRIPTION("GPIO driver for buttons");
MODULE_LICENSE("GPL");
MODULE_VERSION(" 0.1");

static irq_handler_t button_handler_0(unsigned int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t button_handler_1(unsigned int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_t button_handler_2(unsigned int irq, void *dev_id, struct pt_regs *regs);

//Declaring array of button irq handlers for the 5 buttons
static irq_handler_t (*button_handler_pointer[])(unsigned int irq, void *dev_id, struct pt_regs *regs) = {					
																										  button_handler_0,
																										  button_handler_1,
																										  button_handler_2
																									  };
int button_pressed = -1;
EXPORT_SYMBOL(button_pressed);
extern struct task_struct *running_task;
struct kernel_siginfo info;

unsigned int irq_number_array[5];									  // Declare 3 irq numbers for each of the 5 buttons			
int button_array[3] = {BUTTON_0,BUTTON_1,BUTTON_2}; // Store the gpio numbers of each button into an array	

static int __init button_init(void)
{
   int result = 0;
   int button_index;
   printk(KERN_INFO"Initialize the button module!\n");

   // Check if all the gpio values of the buttons are valid
   for(button_index = 0 ; button_index < 3; button_index++) 
   {
   		if (!gpio_is_valid(button_array[button_index]))
   		{
      		printk(KERN_INFO"GPIO_TEST: invalid button GPIO %d\n",button_array[button_index]);
      		return -ENODEV;
   		}
   }

   for(button_index = 0 ; button_index < 3; button_index++)
   {
   		gpio_request(button_array[button_index], "sysfs");      // Request for all the 3 gpio buttons
   		gpio_direction_input(button_array[button_index]);       // Assign each gpio button direction as input 
   		gpio_set_debounce(button_array[button_index],50);		// Debounce each button for 50ms
   		irq_number_array[button_index] = gpio_to_irq(button_array[button_index]); // obtain the IRQ number number associated with the particular gpio
        printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irq_number_array[button_index]);
        result = request_irq(irq_number_array[button_index],    // Add IRQ handler to the particular gpio and get triggered at rising edge
        	                 (irq_handler_t) (*button_handler_pointer[button_index]),
        	                 IRQF_TRIGGER_RISING,"button_handler",
        	                 NULL);
        printk(KERN_INFO"The interrupt request result is: %d\n",result);                 
   }
   memset(&info, 0, sizeof(struct kernel_siginfo ));
    	
   return result;
}
 
 
static void __exit button_exit(void)
{
	int button_index;

	for(button_index = 0; button_index < 3; button_index++)
	{
   		free_irq(irq_number_array[button_index],NULL);
   		gpio_free(button_array[button_index]);
	}
	
	printk(KERN_INFO"Exit button module!\n");
}
 
static irq_handler_t button_handler_0(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
   #ifdef DEBUG
      printk(KERN_INFO"Button 0 pressed\n");
   #endif

   button_pressed = 0;
   //Sending signal to app
    info.si_signo = SIG_BUTTON0;
    info.si_code = SI_QUEUE;
    info.si_int = 0;
    if (running_task != NULL) {
        #ifdef DEBUG
            printk(KERN_INFO "Sending signal to app\n");
        #endif
        if(send_sig_info(SIG_BUTTON0, &info, running_task) < 0) {
            #ifdef DEBUG
              printk(KERN_INFO "Unable to send signal\n");
            #endif
        }
    }
   return (irq_handler_t) IRQ_HANDLED;      
}

static irq_handler_t button_handler_1(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
   #ifdef DEBUG
      printk(KERN_INFO"Button 1 pressed\n");
   #endif

   button_pressed = 1;
    //Sending signal to app
    info.si_signo = SIG_BUTTON1;
    info.si_code = SI_QUEUE;
    info.si_int = 1;
    if (running_task != NULL) {
        #ifdef DEBUG
            printk(KERN_INFO "Sending signal to app\n");
        #endif
        if(send_sig_info(SIG_BUTTON1, &info, running_task) < 0) {
          #ifdef DEBUG
            printk(KERN_INFO "Unable to send signal\n");
          #endif
        }
    }
   return (irq_handler_t) IRQ_HANDLED;      
}

static irq_handler_t button_handler_2(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
   #ifdef DEBUG
      printk(KERN_INFO"Button 2 pressed\n");
   #endif
      
   button_pressed = 2;
   //Sending signal to app
    info.si_signo = SIG_BUTTON2;
    info.si_code = SI_QUEUE;
    info.si_int = 2;
    if (running_task != NULL) {
        #ifdef DEBUG
          printk(KERN_INFO "Sending signal to app\n");
        #endif
        if(send_sig_info(SIG_BUTTON2, &info, running_task) < 0) {
          #ifdef DEBUG
            printk(KERN_INFO "Unable to send signal\n");
          #endif
        }
    }
   return (irq_handler_t) IRQ_HANDLED;      
}

module_init(button_init);
module_exit(button_exit);
