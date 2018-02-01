
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/screen_info.h> 
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define PLATFORM_INFO_SIZE 4095UL

static char *platform_info;
static size_t pi_size;

static size_t pi_cat(char *string)
{
    size_t const len = strlen(string);
    size_t write = 0;

    if(PLATFORM_INFO_SIZE - pi_size > len){
        write = len;
    }else{
        write = PLATFORM_INFO_SIZE - pi_size;
    }

    if(write > 0){
        pi_size += write;
        strncat(platform_info, string, write);
    }

    if(write != len){
        printk("platform_info: warning: platform_info too small, failed to cat %zu bytes\n", len - write);
    }

    return write;
}

static int open_platform_info(struct inode *inode, struct file *file)
{
    printk("%s\n", __func__);
    
    if(!capable(CAP_SYS_RAWIO))
        return -EPERM;

    inode->i_size = strlen(platform_info);
    
    return 0;
}

static ssize_t read_platform_info(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    printk("%s\n", __func__);
    return 0;
}

static int mmap_platform_info(struct file *file, struct vm_area_struct *vma)
{
    unsigned long page;
    size_t size = vma->vm_end - vma->vm_start;
    printk("%s\n", __func__);

    printk("%lu <-> %lu\n", size, PLATFORM_INFO_SIZE + 1);
    if(size > PLATFORM_INFO_SIZE + 1){
        return -EINVAL;
    }

    page = virt_to_phys((void*)platform_info) >> PAGE_SHIFT;
    if(remap_pfn_range(vma, vma->vm_start, page, size, vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}

static const struct file_operations platform_info_fops = {
    .open = open_platform_info,
    .read = read_platform_info,
    .mmap = mmap_platform_info
};

static struct miscdevice platform_info_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "platform_info",
    .fops = &platform_info_fops
};

void generate_boot_fb_info(void)
{
    char framebuffer[128];
    memset(framebuffer, 0, 128);

    if(screen_info.lfb_base){
        pi_cat("    <boot>\n");
        sprintf(framebuffer,
                "        <framebuffer phys=\"%#x\" width=\"%u\" height=\"%u\" bpp=\"%u\" type=\"1\" pitch=\"%u\"/>\n",
                screen_info.lfb_base, screen_info.lfb_width, screen_info.lfb_height,
                screen_info.lfb_depth, screen_info.lfb_linelength);
        pi_cat(framebuffer);
        pi_cat("    </boot>\n");
    }
}

void generate_platform_info(void)
{
    memset(platform_info, 0, PLATFORM_INFO_SIZE + 1);
    pi_size = 0;
    
    pi_cat("<platform_info>\n");
    generate_boot_fb_info();
    //generate_acpi_info();
    pi_cat("</platform_info>\n");
}

static int __init platform_info_init(void)
{
    platform_info = (char*)kmalloc(PLATFORM_INFO_SIZE + 1, GFP_KERNEL);
    generate_platform_info();
    printk("platform_info:\n%s", platform_info);
    misc_register(&platform_info_dev);
    printk(KERN_INFO "platform_info registered\n");
    return 0;
}

static void __exit platform_info_exit(void)
{
    misc_deregister(&platform_info_dev);
    kfree(platform_info);
    printk(KERN_INFO "platform_info unregistered\n");
}


module_init(platform_info_init);
module_exit(platform_info_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes Kliemann <jk@jkliemann.de");
MODULE_DESCRIPTION("Set up platform_info rom as device for Genode base-linux");

