#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>

#define DRIVER_NAME "globalmem"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("River Xu");
MODULE_DESCRIPTION("virtual memory global mem driver driver...");

struct globalmem_dev {
    dev_t devno;
    struct cdev cdev;
};

static struct globalmem_dev *gmem_dev;

static int major = 0;
module_param(major, int, 0644);

static void globalmem_release(struct device *dev) {
    printk(KERN_INFO "globalmem release!\n");

    return;
}

static struct platform_device globalmem_plat_dev = {
    .name = DRIVER_NAME,
    .dev = {
        .release = globalmem_release,
    },
};

static int globalmem_open(struct inode *inode, struct file *filep) {
    printk(KERN_INFO "globalmem open\n");

    return 0;
}

static int globalmem_close(struct inode *inode, struct file *filep) {
    printk(KERN_INFO "globalmem close\n");

    return 0;
}

static ssize_t globalmem_read(struct file *filep, char __user *buf, size_t size, loff_t *ppos) {
    printk(KERN_INFO "globalmem read\n");

    return 0;  /* EOF */
}

static ssize_t globalmem_write(struct file *filep, const char __user *buf, size_t size, loff_t *ppos) {
    printk(KERN_INFO "globalmem write\n");

    return size;
}

static long globalmem_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO "globalmem ioctl\n");

    return 0;
}

static struct file_operations gmem_fops = {
    .open = globalmem_open,
    .release = globalmem_close,
    .read = globalmem_read,
    .write = globalmem_write,
    .unlocked_ioctl = globalmem_ioctl,
};

static int globalmem_cdev_setup(struct globalmem_dev *dev, dev_t devno) {
    int ret = 0;

    /* cdev init */
    cdev_init(&dev->cdev, &gmem_fops);

    /* cdev add */
    ret = cdev_add(&dev->cdev, devno, 1);

    dev->devno = devno;

    return ret;
}

static int globalmem_probe(struct platform_device *dev) {
    int ret;
    dev_t devno;

    printk(KERN_INFO "globalmem probe!\n");

    devno = MKDEV(major, 0);

    if (major) {
        ret = register_chrdev_region(devno, 1, DRIVER_NAME);
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, DRIVER_NAME);
    }

    if (ret < 0) {
        printk(KERN_ERR "allocate/register chr dev failed!\n");
        goto out;
    }

    gmem_dev = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!gmem_dev) {
        printk(KERN_ERR "out of mem!\n");
        ret = -ENOMEM;
        goto malloc_fail;
    }

    ret = globalmem_cdev_setup(gmem_dev, devno);
    if (ret < 0) {
        printk(KERN_ERR "globalmem cdev setup failed\n");
        goto setup_fail;
    }

    printk(KERN_INFO "globalmem probe succeed!\n");

    return 0;

setup_fail:
    kfree(gmem_dev);
    gmem_dev = NULL;

malloc_fail:
    unregister_chrdev_region(devno, 1);

out:
    return ret;
}

static int globalmem_remove(struct platform_device *dev) {
    dev_t devno;

    printk(KERN_INFO "globalmem remove!\n");

    if (gmem_dev) {
        devno = gmem_dev->devno;

        cdev_del(&gmem_dev->cdev);

        unregister_chrdev_region(devno, 1);

        kfree(gmem_dev);
        gmem_dev = NULL;
    }

    printk(KERN_INFO "globalmem remove succeed!\n");

    return 0;
}

static struct platform_driver globalmem_plat_drv = {
    .driver = {
        .name = DRIVER_NAME,
    },
    .probe = globalmem_probe,
    .remove = globalmem_remove,
};

static int __init globalmem_init(void) {
    int ret = 0;

    ret = platform_device_register(&globalmem_plat_dev);
    if (ret < 0) {
        printk(KERN_ERR "platform device register failed\n");
        return ret;
    }

    ret = platform_driver_register(&globalmem_plat_drv);

    if (ret < 0) {
        printk(KERN_ERR "platform driver register failed\n");
        goto out;
    }

    printk(KERN_INFO "globalmem init succeed!\n");

    return 0;

out:
    platform_device_unregister(&globalmem_plat_dev);

    return ret;
}

static void __exit globalmem_exit(void) {
    platform_driver_unregister(&globalmem_plat_drv);

    platform_device_unregister(&globalmem_plat_dev);

    printk(KERN_INFO "globalmem exit succeed!\n");

    return;
}

module_init(globalmem_init);
module_exit(globalmem_exit);
