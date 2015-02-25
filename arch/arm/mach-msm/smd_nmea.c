/* Copyright (c) 2008-2009, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*
 * SMD NMEA Driver -- Provides GPS NMEA device to SMD port interface.
 *
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>

#include <mach/msm_smd.h>

#define MAX_BUF_SIZE 200
/*SWISTART */
#ifdef CONFIG_SIERRA
#define READ_BUF_SIZE 10240
char smd_buf[READ_BUF_SIZE];
int smd_use=0;
#endif
/* SWISTOP */

static DEFINE_MUTEX(nmea_ch_lock);
static DEFINE_MUTEX(nmea_rx_buf_lock);

static DECLARE_WAIT_QUEUE_HEAD(nmea_wait_queue);

struct nmea_device_t {
	struct miscdevice misc;

	struct smd_channel *ch;

	unsigned char rx_buf[MAX_BUF_SIZE];
	unsigned int bytes_read;
};

struct nmea_device_t *nmea_devp;

static void nmea_work_func(struct work_struct *ws)
{
	int sz;

	for (;;) {
		sz = smd_cur_packet_size(nmea_devp->ch);
		if (sz == 0)
			break;
		if (sz > smd_read_avail(nmea_devp->ch))
			break;
		if (sz > MAX_BUF_SIZE) {
			smd_read(nmea_devp->ch, 0, sz);
			continue;
		}

		mutex_lock(&nmea_rx_buf_lock);
		if (smd_read(nmea_devp->ch, nmea_devp->rx_buf, sz) != sz) {
			mutex_unlock(&nmea_rx_buf_lock);
			printk(KERN_ERR "nmea: not enough data?!\n");
			continue;
		}
/* SWISTART */
#ifdef CONFIG_SIERRA
        if((smd_use+sz)<READ_BUF_SIZE)
        {

  		  nmea_devp->bytes_read = sz;
           memcpy((char *)(&smd_buf)+smd_use,nmea_devp->rx_buf,sz);
           smd_use+=sz;
           smd_buf[smd_use]=0;
        }
        else
        {
          printk(KERN_ERR "NMEA read driver miss interrupt, abandon current buff\n");
          smd_use=0;

        }
		mutex_unlock(&nmea_rx_buf_lock);
    
          if(smd_use>512)
          {
		    wake_up_interruptible(&nmea_wait_queue);
          }
	}

    if(smd_use!=0)
    {
	   wake_up_interruptible(&nmea_wait_queue);
     }
#else
		nmea_devp->bytes_read = sz;
		mutex_unlock(&nmea_rx_buf_lock);
		wake_up_interruptible(&nmea_wait_queue);
	}
#endif
/* SWISTOP */
}

struct workqueue_struct *nmea_wq;
static DECLARE_WORK(nmea_work, nmea_work_func);

static void nmea_notify(void *priv, unsigned event)
{
	switch (event) {
	case SMD_EVENT_DATA: {
		int sz;
		sz = smd_cur_packet_size(nmea_devp->ch);
		if ((sz > 0) && (sz <= smd_read_avail(nmea_devp->ch)))
			queue_work(nmea_wq, &nmea_work);
		break;
	}
	case SMD_EVENT_OPEN:
		printk(KERN_INFO "nmea: smd opened\n");
		break;
	case SMD_EVENT_CLOSE:
		printk(KERN_INFO "nmea: smd closed\n");
		break;
	}
}

static ssize_t nmea_read(struct file *fp, char __user *buf,
			 size_t count, loff_t *pos)
{
	int r;
	int bytes_read;

/* SWISTART */
#ifdef CONFIG_SIERRA
	r = wait_event_interruptible(nmea_wait_queue,
				smd_use);
#else
	r = wait_event_interruptible(nmea_wait_queue,
				nmea_devp->bytes_read);
#endif
/* SWISTOP */
	if (r < 0) {
		/* qualify error message */
		if (r != -ERESTARTSYS) {
			/* we get this anytime a signal comes in */
			printk(KERN_ERR "ERROR:%s:%i:%s: "
				"wait_event_interruptible ret %i\n",
				__FILE__,
				__LINE__,
				__func__,
				r
				);
		}
		return r;
	}

	mutex_lock(&nmea_rx_buf_lock);
/*SWISTART */
#ifdef CONFIG_SIERRA
	bytes_read = smd_use;
    smd_use=0;
    
	r = copy_to_user(buf, &smd_buf, bytes_read);
#else
	bytes_read = nmea_devp->bytes_read;
	nmea_devp->bytes_read = 0;
	r = copy_to_user(buf, nmea_devp->rx_buf, bytes_read);
#endif
/* SWISTOP */
	mutex_unlock(&nmea_rx_buf_lock);

	if (r > 0) {
		printk(KERN_ERR "ERROR:%s:%i:%s: "
			"copy_to_user could not copy %i bytes.\n",
			__FILE__,
			__LINE__,
			__func__,
			r);
		return r;
	}

	return bytes_read;
}

/* SWISTART */
#ifdef CONFIG_SIERRA
/* code reference: smd_pkt_write */
static int nmea_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	int r;
	unsigned char cmd[64];

	if (count > sizeof(cmd))
		return -EINVAL;

	if (smd_write_avail(nmea_devp->ch) < count) {
        printk(KERN_ERR "nmea_write - Not enough space to write\n");
		return -ENOMEM;
	}

	r = copy_from_user(cmd, buf, count);
	if (r) {
		printk(KERN_ERR "nmea_write - copy_from_user failed %d\n", r);
		return -EFAULT;
	}

	r = smd_write(nmea_devp->ch, cmd, count);
	if (r != count) {
		printk(KERN_ERR "nmea_write failed to write %d bytes: %d.\n", count, r);
		return -EIO;
	}

	return count;
}
#endif /* CONFIG_SIERRA */
/* SWISTOP */
static int nmea_open(struct inode *ip, struct file *fp)
{
	int r = 0;

	mutex_lock(&nmea_ch_lock);
	if (nmea_devp->ch == 0)
		r = smd_open("GPSNMEA", &nmea_devp->ch, nmea_devp, nmea_notify);
	mutex_unlock(&nmea_ch_lock);

	return r;
}

static int nmea_release(struct inode *ip, struct file *fp)
{
	int r = 0;

	mutex_lock(&nmea_ch_lock);
	if (nmea_devp->ch != 0) {
		r = smd_close(nmea_devp->ch);
		nmea_devp->ch = 0;
	}
	mutex_unlock(&nmea_ch_lock);

	return r;
}

static const struct file_operations nmea_fops = {
	.owner = THIS_MODULE,
	.read = nmea_read,
/* SWISTART */
#ifdef CONFIG_SIERRA
	.write = nmea_write,
#endif /* CONFIG_SIERRA */
/* SWISTOP */
	.open = nmea_open,
	.release = nmea_release,
};

static struct nmea_device_t nmea_device = {
	.misc = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "nmea",
		.fops = &nmea_fops,
	}
};

static void __exit nmea_exit(void)
{
	destroy_workqueue(nmea_wq);
	misc_deregister(&nmea_device.misc);
}

static int __init nmea_init(void)
{
	int ret;

	nmea_device.bytes_read = 0;
	nmea_devp = &nmea_device;

	nmea_wq = create_singlethread_workqueue("nmea");
	if (nmea_wq == 0)
		return -ENOMEM;

	ret = misc_register(&nmea_device.misc);
	return ret;
}

module_init(nmea_init);
module_exit(nmea_exit);

MODULE_DESCRIPTION("MSM Shared Memory NMEA Driver");
MODULE_LICENSE("GPL v2");
