
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/screen_info.h> 

static ssize_t read_platform_info(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    printk("%s\n", __func__);
    return 0;
}

static const struct file_operations platform_info_fops = {
    .read = read_platform_info
};

static struct miscdevice platform_info_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "platform_info",
    .fops = &platform_info_fops
};

static int __init platform_info_init(void)
{
    printk("screen_info: %d x %d x %d @ %x\n", screen_info.lfb_width, screen_info.lfb_height, screen_info.lfb_depth, screen_info.lfb_base);
    misc_register(&platform_info_dev);
    printk(KERN_INFO "platform_info registered\n");
    return 0;
}

static void __exit platform_info_exit(void)
{
    misc_deregister(&platform_info_dev);
    printk(KERN_INFO "platform_info unregistered\n");
}


module_init(platform_info_init);
module_exit(platform_info_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes Kliemann <jk@jkliemann.de");
MODULE_DESCRIPTION("Set up platform_info rom as device for Genode base-linux");
