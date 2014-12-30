/* arch/arm/mach-msm/sierra_i2c.c
 *
 * Copyright (C) 2013 Sierra Wireless, Inc
 * Author: Alex Tan <atan@sierrawireless.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/* #define DEBUG */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <linux/i2c/sierra_i2c.h>

#define I2C_BUF_SIZE 128
static LIST_HEAD(sierra_i2c_device_list);

struct sierra_i2c_device
{
  struct i2c_client *client;
  struct list_head dev_list;
};

static struct i2c_client * sierra_i2c_lookup(int addr)
{
  struct sierra_i2c_device *dev;
  list_for_each_entry(dev,&sierra_i2c_device_list,dev_list)
  {
    if(dev->client->addr == addr)
      return dev->client;
  }
  return NULL;
}

static ssize_t sierra_i2c_read(struct file *fp, char __user *buf, size_t count,
                               loff_t *posp)
{
  struct i2c_adapter *i2c_adap;
  struct i2c_client *client;
  struct i2c_msg msg;
  char data_buf[I2C_BUF_SIZE];
  int ret,data_size;

  if(list_empty(&sierra_i2c_device_list))
  {
    pr_err("%s: device list is empty. Hint: Set one up using SWI_IOCTL_I2C_ADDR_CONFIG ioctl.\n", __func__);
    return -ENODEV;
  }

  client = (struct i2c_client *)fp->private_data;
  if(!client)
  {
    pr_err("sierra i2c dev was not configured\n");
    return -EIO;
  }

  data_size = min(count,sizeof(data_buf));

  i2c_adap = client->adapter;
    
  msg.addr = client->addr;
  msg.flags = I2C_M_RD;
  msg.len = count;
  msg.buf = data_buf;
    
  ret = i2c_transfer(client->adapter, &msg, 1);
  if(ret < 0)
    return ret;

  if(copy_to_user((void __user *)buf,(void *)data_buf,data_size))
    return -EFAULT;
  
  return data_size;

}

static ssize_t sierra_i2c_write(struct file *fp, const char __user *buf,
             size_t count, loff_t *posp)
{
  struct i2c_adapter *i2c_adap;
  struct i2c_client *client;
  struct i2c_msg msg;
  char data_buf[I2C_BUF_SIZE];
  int ret;
  
  if(list_empty(&sierra_i2c_device_list))
  {
    pr_err("%s: device list is empty. Hint: Set one up using SWI_IOCTL_I2C_ADDR_CONFIG ioctl.\n", __func__);
    return -ENODEV;
  }

  client = (struct i2c_client *)fp->private_data;
  if(!client)
  {
    pr_err("sierra i2c dev was not configured\n");
    return -EIO;
  }

  if(count > sizeof(data_buf))
    return -ENOMEM;
  
  if(copy_from_user(data_buf, buf, count))
    return -EFAULT;
  
  i2c_adap = client->adapter;
  
  msg.addr = client->addr;
  msg.flags = 0;
  msg.len = count;
  msg.buf = data_buf;

  pr_debug("swi_i2c:reading %d data from 0x%x addr\n",count,client->addr);
  ret = i2c_transfer(client->adapter, &msg, 1);
  if(ret >= 0)
    return 0;
  
  return ret;
}
    
static int sierra_i2c_open(struct inode *inode, struct file *file)
{
  int ret = 0;

  return ret;
}

static int sierra_i2c_release(struct inode *inode, struct file *file)
{
  return 0;
}

static long sierra_i2c_ioctl(struct file *filp, u_int cmd, u_long arg)
{
  int addr,freq,ret;
  struct i2c_adapter *i2c_adap = NULL;
  struct i2c_client *swi_i2c_client = NULL;
  struct sierra_i2c_device *swi_i2c_dev = NULL;
  struct i2c_board_info info;
  unsigned short addr_list[] = {0xFF, I2C_CLIENT_END};
  memset(&info, 0, sizeof(struct i2c_board_info));
  i2c_adap = i2c_get_adapter(0x0);
  if(i2c_adap == NULL)
  {
    pr_err("sierra:i2c get null adapter from 0x0\n");
          return -EFAULT;
  }
  switch(cmd)
  {
    case SWI_IOCTL_I2C_ADDR_CONFIG:
      if(copy_from_user(&addr,(void __user *)arg,sizeof(int)))
        return -EFAULT;
      
      /*lookup the i2c dev list to check if this dev exist*/
      swi_i2c_client = sierra_i2c_lookup(addr);
      if(swi_i2c_client)
      {
        filp->private_data = swi_i2c_client;
        return 0;
      }
      
      addr_list[0] = addr;
      swi_i2c_client = i2c_new_probed_device(i2c_adap, &info, addr_list, NULL);
      if(!swi_i2c_client)
        return -ENODEV;
      
      swi_i2c_dev = kmalloc(sizeof(struct sierra_i2c_device),GFP_KERNEL);
      if(!swi_i2c_dev)
        return -ENOMEM;

      swi_i2c_dev->client = swi_i2c_client;
      filp->private_data = swi_i2c_client;
      list_add_tail(&swi_i2c_dev->dev_list,&sierra_i2c_device_list);
      
      pr_debug("config i2c device,addr 0x%x\n",addr);
      break;
      
    case SWI_IOCTL_I2C_FREQ_CONFIG:
      if(copy_from_user(&freq,(void __user *)arg,sizeof(int)))
        return -EFAULT;
      if((ret = swi_set_i2c_freq(i2c_adap,freq * 1000))< 0)
        return ret;
      break;
      
    default:
      break;
  }
  return 0;
}

static struct file_operations sierra_i2c_fops = {
    .owner = THIS_MODULE,
    .read = sierra_i2c_read,
    .write = sierra_i2c_write,
    .open = sierra_i2c_open,
    .unlocked_ioctl = sierra_i2c_ioctl,
    .release = sierra_i2c_release,
};

static struct miscdevice sierra_i2c_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "sierra_i2c",
    .fops = &sierra_i2c_fops,
};

static int __init sierra_i2c_init(void)
{
  return misc_register(&sierra_i2c_misc);
}

static void __exit sierra_i2c_exit(void)
{
  misc_deregister(&sierra_i2c_misc);
}

module_init(sierra_i2c_init);
module_exit(sierra_i2c_exit);

MODULE_AUTHOR("Alex Tan <atan@sierrawireless.com>");
MODULE_DESCRIPTION("Sierra I2C driver");
MODULE_LICENSE("GPL v2");
