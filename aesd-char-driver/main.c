/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include <linux/uaccess.h> 
//#include <asm/uaccess.h> 

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Divyesh Patel"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open");
	/**
	 * TODO: handle open
	 */
	 
	struct aesd_dev *dev = NULL;
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;
	
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	/**
	 * TODO: handle release
	 */
	 //no hardware to shutdown
	 
	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = 0;
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	/**
	 * TODO: handle read
	 */
	
	 struct aesd_dev *dev;
	 dev = filp->private_data;
	 struct aesd_buffer_entry *element = NULL;
	 size_t bytesread = 0;
	 size_t read_offset = 0;
	 
	 if (mutex_lock_interruptible(&dev->aesd_dev_lock))
	 {
	 	PDEBUG("lock not acquired: read op");
		return -ERESTARTSYS;
	 }
	 // @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
  	 // NULL if this position is not available in the buffer (not enough data is written).
	 element = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->cbuf, *f_pos, &read_offset);
	 
	 if(element == NULL)		
	 {
	 	retval = 0;
	 	goto out; //undo changes
	 }
	 
	 bytesread = element->size - read_offset;
	 
/*	 if(bytesread > count)*/
/*	 {*/
/*	 	bytesread = count;*/
/*	 }*/
	 
	 if(copy_to_user(buf, element->buffptr + read_offset, bytesread))
	 {
	 	retval = -EFAULT;
	 	goto out;
	 }
	 else
	 {
	 	retval = bytesread;
	 }
	 
	 *f_pos += bytesread;
	 //retval = bytesread;
	 
out:
	 mutex_unlock(&dev->aesd_dev_lock);
 	 return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	/**
	 * TODO: handle write
	 */
	 
	struct aesd_dev *dev;
	dev = filp->private_data;
	
	if (mutex_lock_interruptible(&dev->aesd_dev_lock))
	 {
	 	PDEBUG("lock not acquired: write op");
		return -ERESTARTSYS;
	 }
	 
	if(!dev->elements.size)
	{
		dev->elements.buffptr = kmalloc(count, GFP_KERNEL);
		if(!dev->elements.buffptr)
		{
			goto out;
		}
	}
	else
	{
		dev->elements.buffptr = krealloc(dev->elements.buffptr, dev->elements.size + count, GFP_KERNEL);
		if(!dev->elements.buffptr)
		{
			goto out;
		}
	}
	
	if(copy_from_user(dev->elements.buffptr[dev->elements.size], buf, count))
	{
		retval = -EFAULT;
		goto out;
	}
	
	dev->elements.size += count;
	
	if(strchr(dev->elements.buffptr, '\n')!=0)	
	{
		const char* buffer_created = NULL;
		buffer_created = aesd_circular_buffer_add_entry(&dev->cbuf, &dev->elements);
		if(buffer_created!=NULL)
		{
			kfree(buffer_created);
		}
		
		dev->elements.buffptr = 0;
		dev->elements.size = 0;
	
	}
	retval = count;
	
out:	
	mutex_unlock(&dev->aesd_dev_lock);
	return retval;
}
struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));

	/**
	 * TODO: initialize the AESD specific portion of the device
	 */
	 
	 mutex_init(&aesd_device.aesd_dev_lock);
	 

	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{
	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);

	/**
	 * TODO: cleanup AESD specific poritions here as necessary
	 */
	 
	struct aesd_buffer_entry *element;
	uint8_t index = 0;
	AESD_CIRCULAR_BUFFER_FOREACH(element, &aesd_device.cbuf, index)
	{
		if(element->buffptr!=NULL)
		{
			kfree(element->buffptr);
		}
	
	mutex_destroy(&aesd_device.aesd_dev_lock);
	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);


