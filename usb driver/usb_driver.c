#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


#define MAX_PKT_SIZE 512 //max packet size
#define BULK_ENDPPINT_OUT 0x02 
#define BULK_ENDPPINT_IN 0x81
#define MIN(x, y) ((x) < (y) ? (x) : (y)) 

static struct usb_device *myUsbDevice;  //kernel's representation of a USB device
static struct usb_class_driver myUsbClass; //identifies a USB driver that wants to use the USB major number
static char* dev_buf; //bulk buffer


//called when the user transfer data from the USB device, 
//read from usb device at most sz bytes to the user space buffer buf
// return the number of bytes read or error 
static ssize_t my_read(struct file *f, char __user *buf, size_t sz, loff_t *off)
{
	int pipe,result,read_size,min;
	
	//allocate memory in order to read
	dev_buf=kmalloc(MAX_PKT_SIZE, GFP_KERNEL);
	
	pipe=usb_rcvbulkpipe(myUsbDevice, BULK_ENDPPINT_IN);
	
	//read from usb device using  BULK_ENDPPINT_IN
	result = usb_bulk_msg(myUsbDevice,pipe,dev_buf, MAX_PKT_SIZE, &read_size, 5000);
	if (result)
	{
		printk(KERN_ERR "Bulk ERROR %d\n", result);
		return result;
	}
	
	min=MIN(sz, read_size);
	
	//copy to the user 
	if (copy_to_user(buf, dev_buf, min))
	{
		printk(KERN_ERR "copy_to_user ERROR\n");
		return -EFAULT;
	}

	return min;
}

//called when the user transfer data from the USB device
//writes from usb device at most sz bytes 
// return the number of bytes read or error 
static ssize_t my_write(struct file *f, const char __user *buf, size_t sz,loff_t *off)
{
	int pipe,result,wrote_size,min;
	//allocate memory in order to write
	dev_buf=kmalloc(MAX_PKT_SIZE, GFP_KERNEL);
	
	wrote_size = MIN(sz, MAX_PKT_SIZE);
	min=MIN(sz, MAX_PKT_SIZE);

	//copy from the user 
	if (copy_from_user(dev_buf, buf, min))
	{
		printk(KERN_ERR "copy_from_user ERROR\n");
		return -EFAULT;
	}
	
	pipe=usb_sndbulkpipe(myUsbDevice, BULK_ENDPPINT_OUT);
	
	//Write the data to usb device using BULK_ENDPPINT_OUT.	
	result = usb_bulk_msg(myUsbDevice,pipe,dev_buf, min, &wrote_size, 5000);
	if (result)
	{
		printk(KERN_ERR "Bulk ERROR %d\n", result);
		return result;
	}

	return wrote_size;
}

//called when open
static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "my_open() is called.\n");
	return 0;
}
//called when close
static int my_close(struct inode *i, struct file *f)
{	printk(KERN_INFO "my_close() is called.\n");
	return 0;
}

//file operations
static struct file_operations fops =
{	.owner=THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.release = my_close,
};


//Called when drivers found match in struct usb_device_id, 
//Create a node in /dev/ with the name os2Ex_dev {minor number}.
// usb driver shoukd intialize the usb unterface and return 0 or negative error number on failure
static int my_probe_function(struct usb_interface *interface, const struct usb_device_id *id)
{
	
	int retval;
	myUsbDevice = interface_to_usbdev(interface);
	if(myUsbDevice == 0){
		printk(KERN_ERR "Not able to connect interface to device."); }
	
	myUsbClass.name = "usb/os2Ex_dev%d";
	myUsbClass.fops = &fops;
	
	if ((retval = usb_register_dev(interface, &myUsbClass)) < 0){
	
		/* Something prevented us from registering this driver */
		printk(KERN_ERR "Not able to get a minor for this device."); 	
	}
	else
	{
		printk(KERN_INFO "Minor obtained: %d\n", interface->minor);	
	}

	return retval;
}

//called when the usb inteface has been reoved from the system
static void my_disconnect_function(struct usb_interface *interface)
{

	/* give back our minor */
	usb_deregister_dev(interface, &myUsbClass);
}

/* table of devices that work with this driver */
static struct usb_device_id myDevice_table[] =
{
	{ USB_DEVICE(0x0483, 0x5740) },
	{} /* Terminating entry */
};

/* allow user-space tools to figure out what devices this driver can control */
MODULE_DEVICE_TABLE (usb, myDevice_table);

static struct usb_driver myDevice_driver =
{	
	.name = "usb_driver",
	.id_table = myDevice_table,
	.probe = my_probe_function,
	.disconnect = my_disconnect_function,
	
};

static int __init my_init(void)
{
	int result;

	//* register this driver with the USB subsystem */
	result = usb_register(&myDevice_driver );
	
	if (result)
		printk(KERN_ERR "usb_register failed. Error numbe %d", result);		

	return result;
}

static void __exit my_exit(void)
{
	/* deregister this driver with the USB subsystem */
	/* invokes disconnect() within usb_deregister() */
	usb_deregister(&myDevice_driver );
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Saher + esam");
MODULE_DESCRIPTION("OS2 USB Driver Exercize");
MODULE_VERSION("1.0");
