#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "simple_chr"
#define MEM_4K 4096
#define MEM_CLEAR 0x1   /* clear simple chr dev's global mem to 0x00 */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("River Xu");
MODULE_DESCRIPTION("simple char device driver sample...");

static int major = 0;
module_param(major, int, 0644);

struct simple_chr_dev{
    dev_t devno;
    struct cdev cdev;
    struct mutex mutex;
    char data[MEM_4K];

};

static struct simple_chr_dev* simple_dev = NULL;

static int simple_dev_open(struct inode *inode, struct file *filep) {
    // printk(KERN_INFO "open file...\n");

    filep->private_data = simple_dev;

    return 0;
}

static int simple_dev_close(struct inode *inode, struct file *filep) {
    // printk(KERN_INFO "close file...\n");

    // do nothing...

    return 0;
}

static ssize_t simple_dev_read(struct file *filep, char __user *buf, size_t size, loff_t *ppos) {
    // printk("userspace data: addr - %p, buf - %s\n", buf, buf);
    int ret = 0;
    unsigned long p = *ppos;
    unsigned int count  = size;

    struct simple_chr_dev* dev = (struct simple_chr_dev *)filep->private_data;

    if (p >= MEM_4K) {
        return count ? -ENXIO:0;
    }

    if (count > MEM_4K - p)
        count = MEM_4K - p;

    mutex_lock(&dev->mutex);

    if(copy_to_user(buf, (void *)(dev->data + p), count)) {
        ret -= -EFAULT;
    } else {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
    }

    mutex_unlock(&dev->mutex);

    return ret;
}

static ssize_t simple_dev_write(struct file *filep, const char __user *buf, size_t size, loff_t *ppos) {
    int ret = 0;
    unsigned long p = *ppos;
    unsigned int count = size;

    struct simple_chr_dev* dev = (struct simple_chr_dev *)filep->private_data;

    if (p >= MEM_4K)
        return count? -ENXIO:0;
    if (count > MEM_4K - p)
        count = MEM_4K - p;

    mutex_lock(&dev->mutex);

    if (copy_from_user(dev->data + p, buf, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
    }

    mutex_unlock(&dev->mutex);

    return ret;
}

static loff_t simple_dev_llseek(struct file *filep, loff_t offset, int orig) {
    loff_t ret = 0;

    printk(KERN_INFO "lseek case, orig=%d, offset=%lld\n", orig, offset);

    switch (orig) {
        case 0:     /* according to start position... */
            if (offset < 0) {
                ret = -EINVAL;
                break;
            }
            if ((unsigned int)offset > MEM_4K) {
                ret = -EINVAL;
                break;
            }

            filep->f_pos = (unsigned int)offset;
            ret = filep->f_pos;

            printk(KERN_INFO "llseek succeed, filep->f_pos=%lld!\n", filep->f_pos);

            break;

        case 1:     /* according to current pos */
            if ((filep->f_pos + offset) > MEM_4K) {
                ret = -EINVAL;
                break;
            }

            if ((filep->f_pos + offset) < 0) {
                ret = -EINVAL;
                break;
            }

            filep->f_pos += offset;
            ret = filep->f_pos;
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

static long simple_dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {

    struct simple_chr_dev *dev = filep->private_data;

    switch (cmd) {
        case MEM_CLEAR:
            mutex_lock(&dev->mutex);
            memset(dev->data, 0, MEM_4K);
            mutex_unlock(&dev->mutex);

            printk(KERN_INFO "globalmem is set to zero\n");
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations simple_dev_fops = {
    .open = simple_dev_open,
    .release = simple_dev_close,
    .read = simple_dev_read,
    .write = simple_dev_write,
    .llseek = simple_dev_llseek,
    .unlocked_ioctl = simple_dev_ioctl,
};

static int simple_dev_setup(struct simple_chr_dev* dev, dev_t devno) {
    int ret = 0;
    cdev_init(&dev->cdev, &simple_dev_fops);

    ret = cdev_add(&dev->cdev, devno, 1);

    dev->devno = devno;
    mutex_init(&dev->mutex);

    return ret;
}

static int __init simple_chr_init(void) {
    int ret = 0;
    dev_t devno;

    devno = MKDEV(major, 0);

    printk(KERN_INFO "devno = %d\n", devno);

    if (major) {
        ret = register_chrdev_region(devno, 1, DEVICE_NAME);
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
    }

    if (ret < 0) {
        printk(KERN_ERR "register or alloc char device region failed!\n");
        goto chrdev_error;
    }

    printk(KERN_INFO "devno = %d\n", devno);

    simple_dev = kzalloc(sizeof(struct simple_chr_dev), GFP_KERNEL);

    if (!simple_dev) {
        printk(KERN_ERR "mem allocate failed!\n");
        ret = -ENOMEM;
        goto mem_fail;
    }

    // simple_dev->devno = devno;

    ret = simple_dev_setup(simple_dev, devno);
    if (ret < 0) {
        printk(KERN_ERR "simple dev setup failed\n");
        goto setup_fail;
    }

    printk(KERN_INFO "simple chr init succeed!\n");

    return 0;

setup_fail:
mem_fail:
    unregister_chrdev_region(devno, 1);

chrdev_error:
    return ret;
}

static void __exit simple_chr_exit(void) {
    dev_t devno;

    if (simple_dev) {
        devno = simple_dev->devno;

        cdev_del(&simple_dev->cdev);

        unregister_chrdev_region(devno, 1);

        kfree(simple_dev);
    }

    printk(KERN_INFO "simple chr exit succeed!\n");

    return;
}

module_init(simple_chr_init);
module_exit(simple_chr_exit);
