#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/types.h>
#include<linux/slab.h>
#include<linux/device.h>

MODULE_LICENSE("GPL");

int globalvar_open(struct inode *,struct file *);
int globalvar_release(struct inode *,struct file *);
ssize_t globalvar_read(struct file *,char *,size_t,loff_t *);
ssize_t globalvar_write(struct file *,const char *,size_t,loff_t *);


static int dev_major=101;
int dev_minor=0;
#define DEVICE_NAME "globalvar"
static struct class *globalvar_class;
int temp;

struct file_operations globalvar_fops={
.owner=THIS_MODULE,
.open=globalvar_open,
.release=globalvar_release,
.read=globalvar_read,
.write=globalvar_write,
};

struct globalvar_dev
{
  int global_var;
  struct cdev cdev;
};

struct globalvar_dev *my_dev;

static void __exit globalvar_exit(void)
{
   dev_t devno=MKDEV(dev_major,dev_minor);
   cdev_del(&my_dev->cdev);
   kfree(my_dev);
   unregister_chrdev_region(devno,1);
   device_destroy(globalvar_class,MKDEV(dev_major,0));
   class_destroy(globalvar_class);
   printk("globalvar unregister success!\n");
}

static int __init globalvar_init(void)
{
  int ret,err;
  dev_t devno;
  if(dev_major)
  { 
    devno=MKDEV(dev_major,dev_minor);
    ret=register_chrdev_region(devno,1,"globalvar");
  }
  else
  {
    ret=alloc_chrdev_region(&devno,dev_minor,1,"globalvar");
    dev_major=MAJOR(devno);
  }
  if(ret<0)
  {
    printk("globalvar register failure!\n");
    globalvar_exit();
    return ret;
  }
  else
  {
    printk("globalvar register success!\n");
  }
  my_dev=kmalloc(sizeof(struct globalvar_dev),GFP_KERNEL);
  if(!my_dev)
  {
    ret=-ENOMEM;
    printk("create device failed!\n");
  }
  else
  {
    my_dev->global_var=0;
    cdev_init(&my_dev->cdev,&globalvar_fops);
    my_dev->cdev.owner=THIS_MODULE;
    err=cdev_add(&my_dev->cdev,devno,1);
    if(err<0)
    {
      printk("add device failure!\n");
    }
    else
    {
      globalvar_class=class_create(THIS_MODULE,DEVICE_NAME);
      if (IS_ERR(globalvar_class))
      {
         printk("Err:failed in creating class.\n");
         return -1;
      }
      device_create(globalvar_class,NULL,MKDEV(dev_major,0),NULL,"%s",DEVICE_NAME);
      printk("add device success!\n");
    }
  }
  return ret;
}

int globalvar_open(struct inode *inode,struct file *filp)
{
   struct globalvar_dev *dev;
   dev=container_of(inode->i_cdev,struct globalvar_dev,cdev);
   filp->private_data=dev;
   return 0;
}

int globalvar_release(struct inode *inode,struct file *filp)
{
   return 0;
}

ssize_t globalvar_read(struct file *filp, char *buf,size_t len,loff_t *off)
{
  struct globalvar_dev *dev=filp->private_data;
  if(copy_to_user(buf,&dev->global_var,sizeof(int)))
  {
    return -EFAULT;
  }
  return *buf;
}

ssize_t globalvar_write(struct file *filp,const char *buf,size_t len,loff_t *off)
{
  struct globalvar_dev *dev=filp->private_data;
  if(copy_from_user(&dev->global_var,buf,sizeof(int)))
  {
    return -EFAULT;
  }
  return sizeof(int);
}

static ssize_t globalvar_show(struct device *dev,struct device_attribute *attr,char *buf,size_t size)
{
  sprintf(buf,"read,demo temp is %d/n",temp);
  return 0;
}

static ssize_t globalvar_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t size)
{
   sscanf(buf,"%d",&temp);
   printk(KERN_NOTICE"write,demo temp is %d/n",temp);
   return size;
}

static DEVICE_ATTR(globalvar,(S_IRUGO | S_IWUSR | S_IWGRP),globalvar_show,globalvar_store);


module_init(globalvar_init);
module_exit(globalvar_exit);




