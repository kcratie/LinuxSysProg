#ifndef _HW3_H_
#define _HW3_H_

#include <linux/types.h>
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef HW3_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "hw3: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef HW3_MAJOR
#define HW3_MAJOR 0   /* dynamic major by default */
#endif

#ifndef HW3_NR_DEVS
#define HW3_NR_DEVS 3    /* scull0 through scull2 */
#endif


/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "hw3_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of HW3_QUANTUM bytes.
 *
 * The array (quantum-set) is HW3_QSET long.
 */
#ifndef HW3_QUANTUM
#define HW3_QUANTUM 4000
#endif

#ifndef HW3_QSET
#define HW3_QSET    1000
#endif

/*
 * The pipe device is a simple circular buffer. Here its default size
 */
#ifndef HW3_P_BUFFER
#define HW3_P_BUFFER 4000
#endif

/*
 * Representation of scull quantum sets.
 */
struct hw3_qset {
	void **data;
	struct hw3_qset *next;
};

typedef struct hw3_dev {
	struct hw3_qset *data;  	/* Pointer to first quantum set */
	int quantum;              	/* the current quantum size */
	int qset;                 	/* the current array size */
	unsigned long size;       	/* amount of data stored here */
	//unsigned int access_key;  	/* used by sculluid and scullpriv */
	struct semaphore sem;     	/* mutual exclusion semaphore     */
	struct cdev cdev;	  		/* Char device structure		*/
	struct list_head list_item;
	dev_t dev_no;

} hw3_device_t;

typedef struct hw3_drv {
	struct list_head devices_list;
	struct class * class;
} hw3_driver_t;

/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)			/* low  nibble */


/*
 * The different configurable parameters
 */
extern int hw3_major;     	/* main.c */
extern int hw3_nr_devs;
extern int hw3_quantum;
extern int hw3_qset;


/*
 * Prototypes for shared functions
 */


int     hw3_trim(struct hw3_dev *dev);

ssize_t hw3_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t hw3_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  hw3_llseek(struct file *filp, loff_t off, int whence);
int     hw3_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);


/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define HW3_IOC_MAGIC  'k'
/* Please use a different 8-bit number in your code */

#define HW3_IOCRESET    _IO(HW3_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */
#define HW3_IOCSQUANTUM _IOW(HW3_IOC_MAGIC,  1, int)
#define HW3_IOCSQSET    _IOW(HW3_IOC_MAGIC,  2, int)
#define HW3_IOCTQUANTUM _IO(HW3_IOC_MAGIC,   3)
#define HW3_IOCTQSET    _IO(HW3_IOC_MAGIC,   4)
#define HW3_IOCGQUANTUM _IOR(HW3_IOC_MAGIC,  5, int)
#define HW3_IOCGQSET    _IOR(HW3_IOC_MAGIC,  6, int)
#define HW3_IOCQQUANTUM _IO(HW3_IOC_MAGIC,   7)
#define HW3_IOCQQSET    _IO(HW3_IOC_MAGIC,   8)
#define HW3_IOCXQUANTUM _IOWR(HW3_IOC_MAGIC, 9, int)
#define HW3_IOCXQSET    _IOWR(HW3_IOC_MAGIC,10, int)
#define HW3_IOCHQUANTUM _IO(HW3_IOC_MAGIC,  11)
#define HW3_IOCHQSET    _IO(HW3_IOC_MAGIC,  12)

/*
 * The other entities only have "Tell" and "Query", because they're
 * not printed in the book, and there's no need to have all six.
 * (The previous stuff was only there to show different ways to do it.
 */
#define HW3_P_IOCTSIZE _IO(HW3_IOC_MAGIC,   13)
#define HW3_P_IOCQSIZE _IO(HW3_IOC_MAGIC,   14)
/* ... more to come */

#define HW3_IOC_MAXNR 14

#endif /* _HW3_H_ */
