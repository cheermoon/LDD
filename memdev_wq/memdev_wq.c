/*  
 memdev_wq.c : ldd

	Author : moon.cheng.2014@gmail.com  
	Date   : 2014-08-12
	Version: 1.0
			
	This program is a demo program for linux device drivers created by moon. 
	It is a driver for a segment of memory.You can use the interface it supply 
	to operate the memory. Such as read,write or clear it.
				
	If you find some bugs or you have some advices. Send me a email pls!
					
	This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h> //wait_queue
#include <linux/sched.h>//wait_queue
#include <linux/semaphore.h>
#include <asm/uaccess.h>/*copy_to_user & copy_from_user*/

#define M_DEBUG 1  /*debug output infomation switch*/
#define MEM_SIZE 512  /*memory size*/
#define MAJOR_DEVNO 0 /*use 0 to use malloc_chrdev_region*/

static int major_devno = MAJOR_DEVNO;	 
static struct memdev *devp;
DECLARE_MUTEX(mem_sem);
/*device struct*/
struct memdev {
	struct cdev cdev;
	unsigned char mem[MEM_SIZE];
	struct rw_semaphore rw_sem;//read-write semphore
	struct semaphore sem_open_restrict; //how many numbers can be opened
	wait_queue_head_t mem_queue;
	int flag;
};


loff_t memdev_llseek(struct file *fp, loff_t off, int ori)
{
	int ret;

#if M_DEBUG
	printk(KERN_INFO"%s:%d off = %d ori = %d\n", __func__, __LINE__,off, ori);
#endif
	switch (ori) {
		case SEEK_SET:
			if (off < 0 || off > MEM_SIZE) {
				return -EINVAL;
			}
			fp -> f_pos = off;
			ret = fp -> f_pos;
			break;
		case SEEK_CUR:
			if ((fp->f_pos + off < 0) || (fp->f_pos + off > MEM_SIZE)) {
				return -EINVAL;
			}
			fp -> f_pos += off;	
			ret = fp -> f_pos;
			break;
		case SEEK_END:
			if (off < 0 || off > MEM_SIZE) {
				return -EINVAL;
			}
			fp -> f_pos = MEM_SIZE - off;
			ret = fp -> f_pos;
			break;
		default : 
			return -EINVAL;
	}
	return ret;
}

ssize_t memdev_read(struct file *fp, char __user *buff, size_t count, loff_t *f_pos)
{
	struct memdev *devp = fp->private_data;
	unsigned int size = count;
	unsigned long p = *f_pos;
	int ret = 0;
	
#if M_DEBUG
	printk(KERN_INFO"%s:%d *f_pos = %d size = %d\n", __func__, __LINE__, p, size);
#endif


	if ( p > MEM_SIZE) {
		p = 0;
	}
	if (p + size > MEM_SIZE) {
		size = MEM_SIZE - p;
	}

	wait_event_interruptible(devp->mem_queue, devp->flag > 0);
	down_read(&devp->rw_sem);
	if (size > devp->flag) {
		size = devp->flag;
	}
	if (copy_to_user(buff, devp->mem + p, size)) {
		up_read(&devp->rw_sem);
		ret = -EFAULT;
	} else {
		ret = size;
		devp->flag -= size;
		*f_pos += size;
		up_read(&devp->rw_sem);
#if M_DEBUG
		printk(KERN_INFO"%s:%d read %d bytes from mem!\n", __func__, __LINE__, size);
#endif
	}

	return ret;
}

ssize_t memdev_write(struct file *fp, const char __user *buff, size_t count, loff_t *f_pos)
{
	struct memdev *devp = fp->private_data;
	unsigned int size = count;
	unsigned long p = *f_pos;
	int ret = 0;
#if M_DEBUG
	printk(KERN_INFO"%s:%d *f_pos = %d size = %d\n", __func__, __LINE__, p, size);
#endif

	if ( p > MEM_SIZE) {
		return -EINVAL;
	}
	if (p + size > MEM_SIZE) {
		size = MEM_SIZE - p;
	}

	down_write(&devp->rw_sem);
	if (copy_from_user(devp->mem + p, buff, size)){
		up_write(&devp->rw_sem);
		ret = -EFAULT;
	} else {
		ret = size;
		devp->flag += size;
		*f_pos += size;
		up_write(&devp->rw_sem);
#if M_DEBUG
		printk(KERN_INFO"%s:%d write %d bytes to mem!\n", __func__, __LINE__, size);
#endif
	}

	wake_up_interruptible(&devp->mem_queue);
	
	return ret;
}

int memdev_ioctl(struct inode *nodep, struct file *fp, unsigned int cmd, unsigned long arg)
{
	int ret;
	struct memdev *devp;

#if M_DEBUG
	printk(KERN_INFO"%s:%d cmd = %d \n", __func__, __LINE__, cmd);
#endif
	switch (cmd) {
		case 0 : 
			devp = fp -> private_data;
			 memset(devp->mem, 0, MEM_SIZE);
			break;
		default : ret = -EINVAL;
	}
	return 0;
}

int memdev_open(struct inode *nodep, struct file *fp)
{
	struct memdev *devp;
/*
	if (down_trylock(&sem_open_restrict)) {
		return -EBUSY;
	}
*/
	
	devp = container_of(nodep->i_cdev, struct memdev, cdev);

	if (!!down_interruptible(&devp->sem_open_restrict)) {
			printk(KERN_INFO"YOU ARE BE INTERRUPTED!\n");
			return 0;
	}

	fp->private_data = devp;
#if M_DEBUG
	printk(KERN_INFO"%s:%d you have open the device\n", __func__, __LINE__);
#endif
	return 0;
}


int memdev_release(struct inode *nodep, struct file *fp)
{
#if M_DEBUG
	printk(KERN_INFO"%s:%d you have release the device\n", __func__, __LINE__);
#endif

	up(&devp->sem_open_restrict);

	return 0;
}


/*file_operations */
static struct file_operations memdev_fops = {
	.owner = THIS_MODULE,
	.llseek = memdev_llseek,
	.read = memdev_read,
	.write = memdev_write,
	.ioctl = memdev_ioctl,
	.open = memdev_open,
	.release = memdev_release,
};

static int __init memdev_init(void)
{
	int ret;
	dev_t devno;

/*apply for device no*/
	if (major_devno) {
	   devno=MKDEV(major_devno, 0);
	   ret = register_chrdev_region(devno, 0, "memdev_wq");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "memdev_wq");
		major_devno = MAJOR(devno);
	}
	if (!!ret) {
		printk(KERN_ERR"arrage device no erro!\n");
		return ret;
	}

/*apply for device struct*/
	devp = kmalloc(sizeof(struct memdev), GFP_KERNEL);
	if (!devp) {
		printk(KERN_ERR"malloc memory to devp erro!\n");
		unregister_chrdev_region(devno, 1);
		return -ENOMEM;
	}
	memset(devp, 0, sizeof(struct memdev));

/*Init our device struct*/	
	cdev_init(&devp->cdev, &memdev_fops);
	devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&devp->cdev, devno, 1);
	if (!!ret) {
		printk(KERN_ERR"register the device to system erro!\n");
		kfree(devp);
		unregister_chrdev_region(devno, 1);
		return ret;
	}

	sema_init(&devp->sem_open_restrict, 3);
	init_rwsem(&devp->rw_sem);
	init_waitqueue_head(&devp->mem_queue);

	return 0;

}

static void __exit memdev_exit(void)
{
	cdev_del(&devp->cdev);
	kfree(devp);
	unregister_chrdev_region(MKDEV(major_devno, 0), 1);
}

module_init(memdev_init);
module_exit(memdev_exit);

MODULE_AUTHOR("moon.cheng");
MODULE_VERSION("v0.1");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("This is a charactor device demo driver");

