/*
 * pixxgen.c
 *
 *  Created on: 2021. 10. 21.
 *      Author: k2h
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/property.h>


#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/spi/spi.h>

#include "../ioctl.h"

#define DEVICE_NAME 		"pixxgen_adc"
#define US_TO_NS(x) 		(x * 1000L)
#define MMAP_SIZE			4096		//128 * 4 byte 항상 4096배수 사용
#define MMAP_CNT			5			5
#define MMAP_OFFSET		5			//1 x 5 channel
#define ADC_CHANNEL_NUM	5

#define MCP320X_VOLTAGE_CHANNEL_DIFF(chan1, chan2)		\
	{							\
		.type = IIO_VOLTAGE,				\
		.indexed = 1,					\
		.channel = (chan1),				\
		.channel2 = (chan2),				\
		.address = (chan1),				\
		.differential = 1,				\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) \
	}

enum {
	mcp3202,
	mcp3204,
	mcp3208,
};

struct pixxgen_adc{
	struct spi_device *spi;
	struct miscdevice pixxgen_miscdevice;
	struct hrtimer pixxgen_hr_timer;
	struct task_struct *pixxgen_thread;
	struct fasync_struct *async_queue; /* asynchronous readers */
	struct completion pixxgen_complete_ok;
	int timer_period;
	char *mmap_buf;
	unsigned int cnt;
	int adc_channel;
	char thread_done;

	u8 tx_buf ____cacheline_aligned;
	u8 rx_buf[4];

	struct spi_message msg;
	struct spi_transfer transfer[2];
};

static int mcp320x_channel_to_tx_data( const unsigned int channel, bool differential)
{
	int start_bit = 1;


	return ((start_bit << 6) | (!differential << 5) | (channel << 2));

}

int pixxgen_thread(void *priv)
{
	struct pixxgen_adc * pixxgen_private = (struct pixxgen_adc *)priv;
	struct device *dev = &pixxgen_private->spi->dev;
	int status;
	bool differential = false;

	dev_info(dev, "pixxgen_thread() called\n");

	pixxgen_private->thread_done = 0;

	while(!kthread_should_stop()) {
		memset(&pixxgen_private->rx_buf, 0, sizeof(pixxgen_private->rx_buf));
		pixxgen_private->tx_buf = mcp320x_channel_to_tx_data(pixxgen_private->adc_channel,
									 differential);
		wait_for_completion_interruptible(&pixxgen_private->pixxgen_complete_ok);
		if(pixxgen_private->thread_done){
			return 0;
		}
		status = spi_sync(pixxgen_private->spi, &pixxgen_private->msg);
		if(status < 0){
			dev_info(dev, "error reading ADC\n");
			return status;
		}
		reinit_completion(&pixxgen_private->pixxgen_complete_ok);

		*((int *)(pixxgen_private->mmap_buf)+pixxgen_private->adc_channel) = (pixxgen_private->rx_buf[0] << 4 | pixxgen_private->rx_buf[1] >> 4); //big endian
		//dev_info(dev, "TX %X ", pixxgen_private->tx_buf);
		pixxgen_private->adc_channel++;
		pixxgen_private->adc_channel = pixxgen_private->adc_channel % ADC_CHANNEL_NUM;

		if(!pixxgen_private->adc_channel){				//모든 채널 데이터 저정
			if (pixxgen_private->async_queue){
				kill_fasync(&pixxgen_private->async_queue, SIGIO, POLL_OUT);
			}
		}
    }
    return 0;
}

enum hrtimer_restart pixxgen_hr_timer(struct hrtimer *t)
{
	struct pixxgen_adc *pixxgen_private = from_timer(pixxgen_private, t, pixxgen_hr_timer);
	struct device *dev;
	ktime_t ktime;

	dev = &pixxgen_private->spi->dev;
	complete(&pixxgen_private->pixxgen_complete_ok);
	ktime = ktime_set(0, US_TO_NS(pixxgen_private->timer_period));
	hrtimer_forward(&pixxgen_private->pixxgen_hr_timer,
			hrtimer_cb_get_time(&pixxgen_private->pixxgen_hr_timer), ktime);

	return HRTIMER_RESTART;
//	return HRTIMER_NORESTART;
}

static int pixxgen_fasync(int fd, struct file *file, int mode)
{
	struct pixxgen_adc * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_adc, pixxgen_miscdevice);
	dev = &pixxgen_private->spi->dev;

	dev_info(dev, "pixxgen_fasync() is called.\n");
	return fasync_helper(fd, file, mode, &pixxgen_private->async_queue);
}

static int pixxgen_mmap(struct file *file, struct vm_area_struct *vma) {

	struct pixxgen_adc * pixxgen_private;
	struct device *dev;

	pixxgen_private = container_of(file->private_data,  struct pixxgen_adc,
			pixxgen_miscdevice);
	dev = &pixxgen_private->spi->dev;

	dev_info(dev, "pixxgen_mmap() called\n");
	if(remap_pfn_range(vma, vma->vm_start, virt_to_phys(pixxgen_private->mmap_buf) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start, vma->vm_page_prot)){
		return -EAGAIN;
	}
	return 0;
}

static int pixxgen_open(struct inode *inode, struct file *file)
{
	struct pixxgen_adc * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_adc, pixxgen_miscdevice);
	//dev = &pixxgen_private->pdev->dev;
	dev = &pixxgen_private->spi->dev;
	dev_info(dev, "pixxgen_dev_open() is called.\n");
	return 0;
}

static int pixxgen_close(struct inode *inode, struct file *file)
{
	struct pixxgen_adc * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_adc, pixxgen_miscdevice);
	dev = &pixxgen_private->spi->dev;

	dev_info(dev, "pixxgen_dev_close() is called.\n");
	return 0;
}

static int pixxgen_read(struct file *file, char __user *buff,
	               size_t count, loff_t *off)
{
	struct pixxgen_adc * pixxgen_private;
	struct device *dev;

	pixxgen_private = container_of(file->private_data, struct pixxgen_adc,
			pixxgen_miscdevice);
	dev = &pixxgen_private->spi->dev;
	dev_info(dev, "pixxgen_read entered\n");

	*off+=count;

	return count;
}

/* declare ioctl_function */
static long pixxgen_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
//	static struct ioctl_info info;
	struct pixxgen_adc *pixxgen_private;
	struct device *dev;
	ktime_t ktime;
	int ret;

	pixxgen_private = container_of(file->private_data, struct pixxgen_adc, pixxgen_miscdevice);
	dev = &pixxgen_private->spi->dev;

	switch (cmd) {
		case ADC_START:
			dev_info(dev, "ADC_START\n");
			pixxgen_private->cnt = 0;
			ktime = ktime_set( 0, US_TO_NS(pixxgen_private->timer_period));
			hrtimer_start(&pixxgen_private->pixxgen_hr_timer, ktime, HRTIMER_MODE_REL );
			break;
		case ADC_STOP:
			dev_info(dev, "ADC_STOP\n");
			ret = hrtimer_cancel(&pixxgen_private->pixxgen_hr_timer);
			if(ret){
				//dev_info(dev, "The timer was still in use.");
			}
			break;
		default:
			dev_info(dev, "invalid command %d\n", cmd);
		return -EFAULT;
	}
	return 0;
}

static const struct file_operations pixxgen_fops = {
	.owner = THIS_MODULE,
	.open = pixxgen_open,
	.read = pixxgen_read,
	.release = pixxgen_close,
	.mmap = pixxgen_mmap,
	.fasync = pixxgen_fasync,
	.unlocked_ioctl 	= pixxgen_ioctl,
};

static ssize_t notify_write(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int period;
	struct pixxgen_adc *pixxgen_private = dev_get_drvdata(dev);

	sscanf(buf, "%d", &period);

	if(period > 0 && period < 10000){
		pixxgen_private-> timer_period = period;
	}
	//sysfs_notify(&dev->kobj, "recipe_sys", "notify");
	return count;
}

static ssize_t notify_read(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct pixxgen_adc *pixxgen_private = dev_get_drvdata(dev);

	dev_info(dev, "notify_read is called\n");

	return scnprintf(buf, sizeof(int)+1, "%d\n", pixxgen_private-> timer_period);
}

static DEVICE_ATTR(notify, 0664, notify_read, notify_write);

static struct attribute *pixxgenadc_attrs[] = {
	&dev_attr_notify.attr,
	NULL,
};

static struct attribute_group pixxgenadc_sys_group = {
	.name = "pixxgenadc_sys",
	.attrs = pixxgenadc_attrs,
};

static int pixxgen_probe(struct spi_device *spi)
{
	int ret;
	struct device *dev = &spi->dev;
	struct pixxgen_adc *pixxgen_private;
	ktime_t ktime;

	dev_info(dev, "pixxgen_probe() function is called. \n");

	pixxgen_private = devm_kzalloc(&spi->dev, sizeof(struct pixxgen_adc), GFP_KERNEL);
	if(!pixxgen_private){
		dev_err(dev, "failed memory allocation");
		return -ENOMEM;
	}
	pixxgen_private->spi = spi;
	spi_set_drvdata(spi, pixxgen_private);

	pixxgen_private->timer_period = 100;				//100uS timer
	ktime = ktime_set( 0, US_TO_NS(pixxgen_private->timer_period));
	hrtimer_init(&pixxgen_private->pixxgen_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	pixxgen_private->pixxgen_hr_timer.function = &pixxgen_hr_timer;

	init_completion(&pixxgen_private->pixxgen_complete_ok);
	pixxgen_private->pixxgen_thread = kthread_run(pixxgen_thread,pixxgen_private,"pixxgen Thread");
	if(pixxgen_private->pixxgen_thread){
		dev_info(dev, "Kthread Created Successfully.\n");
	} else {
		dev_err(dev, "Cannot create kthread\n");
		return -ENOMEM;
	}
	pixxgen_private->mmap_buf = kzalloc(MMAP_SIZE, GFP_KERNEL);
	if(!pixxgen_private->mmap_buf){
		dev_err(dev, "failed memory allocation\n");
		return -ENOMEM;
	}

	pixxgen_private->transfer[0].tx_buf = &pixxgen_private->tx_buf;
	pixxgen_private->transfer[0].len = sizeof(pixxgen_private->tx_buf);
	pixxgen_private->transfer[1].rx_buf = pixxgen_private->rx_buf;
	pixxgen_private->transfer[1].len = DIV_ROUND_UP(12, 8);		//12 --> 12bit
	spi_message_init_with_transfers(&pixxgen_private->msg, pixxgen_private->transfer,
							ARRAY_SIZE(pixxgen_private->transfer));

	/* Attach the SPI device to the private structure */
	pixxgen_private->pixxgen_miscdevice.name = DEVICE_NAME;
	pixxgen_private->pixxgen_miscdevice.minor = MISC_DYNAMIC_MINOR;
	pixxgen_private->pixxgen_miscdevice.fops = &pixxgen_fops;
	pixxgen_private->pixxgen_miscdevice.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	/* Register sysfs call back */
	ret = sysfs_create_group(&dev->kobj, &pixxgenadc_sys_group);
	if (ret < 0) {
		dev_err(dev, "could not register sysfs group\n");
		return ret;
	}

	ret = misc_register(&pixxgen_private->pixxgen_miscdevice);
	if (ret){
		return ret; /* misc_register returns 0 if success */
	}
	dev_info(dev, "pixxgen_probe() function is completed.\n");
	return 0;
}

/* Add remove() function */
static int __exit pixxgen_remove(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct pixxgen_adc *pixxgen_private = spi_get_drvdata(spi);
	int ret;

	pixxgen_private->thread_done = 1;
	complete(&pixxgen_private->pixxgen_complete_ok);
	dev_info(dev, "pixxgen_remove() complete is called.\n");

	kthread_stop(pixxgen_private->pixxgen_thread);
	dev_info(dev, "pixxgen_remove() kthread_stop is called.\n");

	kfree(pixxgen_private->mmap_buf);
	dev_info(dev, "pixxgen_remove() kfree function is called.\n");

	ret = hrtimer_cancel(&pixxgen_private->pixxgen_hr_timer);
	if(ret){
		dev_info(dev, "The timer was still in use.");
	}
	sysfs_remove_group(&dev->kobj, &pixxgenadc_sys_group);
	dev_info(dev, "pixxgen_remove() hrtimer_cancel is called.\n");
	misc_deregister(&pixxgen_private->pixxgen_miscdevice);
	dev_info(dev, "pixxgen_remove() misc_deregister is called.\n");
	return 0;
}

static const struct of_device_id pixxgen_of_ids[] = {
	{ .compatible = "brcm,pixggen_adc", },
	{ }
};
MODULE_DEVICE_TABLE(of, pixxgen_of_ids);

static const struct spi_device_id pixxgen_id[] = {
	{ .name = "MCP3208", },
	{ }
};
MODULE_DEVICE_TABLE(spi, pixxgen_id);

static struct spi_driver pixxgen_platform_driver = {
	.driver = {
		.name = "pixxgen_adc",
		.owner = THIS_MODULE,
		.of_match_table = pixxgen_of_ids,
	},
	.probe   = pixxgen_probe,
	.remove  = pixxgen_remove,
	.id_table	= pixxgen_id,
};
/* Register our platform driver */
module_spi_driver(pixxgen_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kwang Hyuk Ko");
MODULE_DESCRIPTION("This is a pixxgen ADC SPI Device Driver ");
