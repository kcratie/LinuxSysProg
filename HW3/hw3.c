#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include "hw3.h"

/*
 *Load time parameters
*/
int hw3_major =   HW3_MAJOR;
int hw3_minor =   0;
int hw3_nr_devs = HW3_NR_DEVS;
int hw3_quantum = HW3_QUANTUM;
int hw3_qset =    HW3_QSET;

module_param(hw3_major, int, S_IRUGO);
module_param(hw3_minor, int, S_IRUGO);
module_param(hw3_nr_devs, int, S_IRUGO);
module_param(hw3_quantum, int, S_IRUGO);
module_param(hw3_qset, int, S_IRUGO);


MODULE_AUTHOR("Ken Subratie");
MODULE_LICENSE("GPL");

static hw3_driver_t hw3_drv = {
		.devices_list = LIST_HEAD_INIT(hw3_drv.devices_list),
		.class = NULL
};


/*
 * Empty out the hw3 device; must be called with the device
 * semaphore held.
 */
int
hw3_trim(
		struct hw3_dev *dev)
{
	struct hw3_qset *next, *dptr;
	int qset = dev->qset;   /* "dev" is not-null */
	int i;

	for (dptr = dev->data; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = hw3_quantum;
	dev->qset = hw3_qset;
	dev->data = NULL;
	return 0;
}

int
hw3_open(
		struct inode *inode,
		struct file *filp)
{
	struct hw3_dev *dev;

	dev = container_of(inode->i_cdev, struct hw3_dev, cdev);
	filp->private_data = dev; /* for other methods */
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/*trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		hw3_trim(dev);
	}

	return 0;
}

int
hw3_release(
		struct inode *inode,
		struct file *filp)
{
	struct hw3_dev *dev;
	dev = container_of(inode->i_cdev, struct hw3_dev, cdev);
	up(&dev->sem);
	return 0;
}

struct hw3_qset *
hw3_follow(
		struct hw3_dev *dev,
		int n)
{
	struct hw3_qset *qs = dev->data;

        /* Allocate first qset explicitly if need be */
	if (! qs) {
		qs = dev->data = kmalloc(sizeof(struct hw3_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;  /* Never mind */
		memset(qs, 0, sizeof(struct hw3_qset));
	}

	/* Then follow the list */
	while (n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct hw3_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;  /* Never mind */
			memset(qs->next, 0, sizeof(struct hw3_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}

ssize_t
hw3_read(
		struct file *filp,
		char __user *buf,
		size_t count,
		loff_t *f_pos)
{
	struct hw3_dev *dev = filp->private_data;
	struct hw3_qset *dptr;	/* the first listitem */
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;


	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = hw3_follow(dev, item);

	if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
		goto out; /* don't fill holes */

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

  out:

	return retval;
}

ssize_t
hw3_write(
		struct file *filp,
		const char __user *buf,
		size_t count,
		loff_t *f_pos)
{
	struct hw3_dev *dev = filp->private_data;
	struct hw3_qset *dptr;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */


	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = hw3_follow(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

        /* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

  out:

	return retval;
}

int hw3_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int retval = 0;

	return retval;

}

loff_t hw3_llseek(struct file *filp, loff_t off, int whence)
{
	struct hw3_dev *dev = filp->private_data;
	loff_t newpos;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}



struct file_operations hw3_fops = {
	.owner =    THIS_MODULE,
	.llseek =   hw3_llseek,
	.read =     hw3_read,
	.write =    hw3_write,
	.open =     hw3_open,
	.release =  hw3_release,
};

/*
 * The cleanup function is used to handle initialization failures as well.
 * It must work correctly even if some of the items have not been initialized
 */
void
hw3_cleanup_module(void)
{
	struct list_head * itr, *tmp;
	dev_t devno = MKDEV(hw3_major, hw3_minor);
	hw3_device_t * adev;

	/* Get rid of the hw3 device entries */
	list_for_each_safe(itr, tmp, &hw3_drv.devices_list){
		adev = list_entry(itr, hw3_device_t, list_item);
		list_del(itr);
		device_destroy(hw3_drv.class, adev->dev_no);
		hw3_trim(adev);
		cdev_del(&adev->cdev);
		kfree(adev);
	}
	class_destroy(hw3_drv.class);
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, hw3_nr_devs);

}


/*
 * Set up the char_dev structure for this device.
 */
static void
hw3_setup_cdev(
		hw3_device_t *hw3dev,
		int index)
{
	int err;
	hw3dev->dev_no = MKDEV(hw3_major, hw3_minor + index);
    
	cdev_init(&hw3dev->cdev, &hw3_fops);
	hw3dev->cdev.owner = THIS_MODULE;
	hw3dev->cdev.ops = &hw3_fops;
	err = cdev_add (&hw3dev->cdev, hw3dev->dev_no, 1);
	if (err){
		printk(KERN_NOTICE "Error %d adding hw3%d", err, index);
		goto out;
	}
	else{
		if(NULL==device_create(hw3_drv.class, NULL, hw3dev->dev_no, NULL, "hw3%d", index))
			printk(KERN_NOTICE "Error creating device node hw3%d", index);
	}
	out:
	return;
}


int
hw3_init_module(void)
{
	int result = 0, i;
	dev_t devno = 0;
	hw3_device_t * adev;
	if (hw3_major) {	//use major/minor numbers from load input
		devno = MKDEV(hw3_major, hw3_minor);
		result = register_chrdev_region(devno, hw3_nr_devs, "hw3");
	} else {	//have the kernel allocate dev number automatically
		result = alloc_chrdev_region(&devno, hw3_minor, hw3_nr_devs, "hw3");
		hw3_major = MAJOR(devno);
	}
	if (result < 0) {
		printk(KERN_WARNING "hw3: can't get major %d\n", hw3_major);
		return result;
	}
	hw3_drv.class = class_create(THIS_MODULE, "hw3_class");

	 //allocate the devices as the number specified at load time
	for (i = 0; i < hw3_nr_devs; i++) {
		adev = kmalloc(sizeof(*adev), GFP_KERNEL);
		if (!adev) {
				result = -ENOMEM;
				goto fail;
			}
		memset(adev, 0, sizeof(*adev));
		adev->quantum = hw3_quantum;
		adev->qset = hw3_qset;
		sema_init(&adev->sem, 1);
		hw3_setup_cdev(adev, i);
		list_add_tail(&adev->list_item, &hw3_drv.devices_list);
	}
	return result;

  fail:
	hw3_cleanup_module();
	return result;
}

module_init(hw3_init_module);
module_exit(hw3_cleanup_module);
