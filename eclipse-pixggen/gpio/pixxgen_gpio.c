/*
 * pixxgen.c
 *
 *  Created on: 2021. 6. 15.
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
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/delay.h>

#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include "../ioctl.h"

#define DEVICE_NAME 		"pixxgen_gpio"
#define US_TO_NS(x) 		(x * 1000L)

struct pixxgen_gpio{
	struct platform_device *pdev;
	struct miscdevice pixxgen_miscdevice;
	struct gpio_desc *exout16,  *exout17, *exout18, *exout19,
						*exout20, *exout21, *exout22;
	struct gpio_desc	*exin6, *exin7, *exin23;
	struct hrtimer st_motor_hr_timer1, st_motor_hr_timer2, st_motor_hr_timer3, st_motor_hr_timer4;
	int st_motor_period1, st_motor_period2, st_motor_period3, st_motor_period4;
	int st_motor_cnt1, st_motor_cnt2, st_motor_cnt3, st_motor_cnt4;
};
struct platform_device *pdev_export;

enum hrtimer_restart st_motor_hr_timer1(struct hrtimer *t)
{
	struct pixxgen_gpio *pixxgen_private = from_timer(pixxgen_private, t, st_motor_hr_timer1);
	struct device *dev;
	ktime_t ktime;
	static u8 toggle = 0;

	dev = &pixxgen_private->pdev->dev;

	if(pixxgen_private->st_motor_cnt1 > 0){
		if(toggle == 0){
			gpiod_set_value(pixxgen_private->exout16, 1);
			toggle = 1;
		}else{
			gpiod_set_value(pixxgen_private->exout16, 0);
			toggle = 0;
		}
		pixxgen_private->st_motor_cnt1--;
		ktime = ktime_set(0, US_TO_NS(pixxgen_private->st_motor_period1));
		hrtimer_forward(&pixxgen_private->st_motor_hr_timer1,
					hrtimer_cb_get_time(&pixxgen_private->st_motor_hr_timer1), ktime);
		return HRTIMER_RESTART;
	}
	return HRTIMER_NORESTART;
}

enum hrtimer_restart st_motor_hr_timer2(struct hrtimer *t)
{
	struct pixxgen_gpio *pixxgen_private = from_timer(pixxgen_private, t, st_motor_hr_timer2);
	struct device *dev;
	ktime_t ktime;
	static u8 toggle = 0;

	dev = &pixxgen_private->pdev->dev;

	if(pixxgen_private->st_motor_cnt2 > 0){
		if(toggle == 0){
			gpiod_set_value(pixxgen_private->exout17, 1);
			toggle = 1;
		}else{
			gpiod_set_value(pixxgen_private->exout17, 0);
			toggle = 0;
		}
		pixxgen_private->st_motor_cnt2--;
		ktime = ktime_set(0, US_TO_NS(pixxgen_private->st_motor_period2));
		hrtimer_forward(&pixxgen_private->st_motor_hr_timer2,
					hrtimer_cb_get_time(&pixxgen_private->st_motor_hr_timer2), ktime);
		return HRTIMER_RESTART;
	}
	return HRTIMER_NORESTART;
}

enum hrtimer_restart st_motor_hr_timer3(struct hrtimer *t)
{
	struct pixxgen_gpio *pixxgen_private = from_timer(pixxgen_private, t, st_motor_hr_timer3);
	struct device *dev;
	ktime_t ktime;
	static u8 toggle = 0;

	dev = &pixxgen_private->pdev->dev;

	if(pixxgen_private->st_motor_cnt3 > 0){
		if(toggle == 0){
			gpiod_set_value(pixxgen_private->exout18, 1);
			toggle = 1;
		}else{
			gpiod_set_value(pixxgen_private->exout18, 0);
			toggle = 0;
		}
		pixxgen_private->st_motor_cnt3--;
		ktime = ktime_set(0, US_TO_NS(pixxgen_private->st_motor_period3));
		hrtimer_forward(&pixxgen_private->st_motor_hr_timer3,
					hrtimer_cb_get_time(&pixxgen_private->st_motor_hr_timer3), ktime);
		return HRTIMER_RESTART;
	}
	return HRTIMER_NORESTART;
}

enum hrtimer_restart st_motor_hr_timer4(struct hrtimer *t)
{
	struct pixxgen_gpio *pixxgen_private = from_timer(pixxgen_private, t, st_motor_hr_timer4);
	struct device *dev;
	ktime_t ktime;
	static u8 toggle = 0;

	dev = &pixxgen_private->pdev->dev;

	if(pixxgen_private->st_motor_cnt4 > 0){
		if(toggle == 0){
			gpiod_set_value(pixxgen_private->exout19, 1);
			toggle = 1;
		}else{
			gpiod_set_value(pixxgen_private->exout19, 0);
			toggle = 0;
		}
		pixxgen_private->st_motor_cnt4--;
		ktime = ktime_set(0, US_TO_NS(pixxgen_private->st_motor_period4));
		hrtimer_forward(&pixxgen_private->st_motor_hr_timer4,
					hrtimer_cb_get_time(&pixxgen_private->st_motor_hr_timer4), ktime);
		return HRTIMER_RESTART;
	}
	return HRTIMER_NORESTART;
}
static int pixxgen_open(struct inode *inode, struct file *file)
{
	struct pixxgen_gpio * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_gpio, pixxgen_miscdevice);
	dev = &pixxgen_private->pdev->dev;

	dev_info(dev, "pixxgen_dev_open() is called.\n");
	return 0;
}

static int pixxgen_close(struct inode *inode, struct file *file)
{
	struct pixxgen_gpio * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_gpio, pixxgen_miscdevice);
	dev = &pixxgen_private->pdev->dev;

	dev_info(dev, "pixxgen_dev_close() is called.\n");
	return 0;
}
//
int pixxgen_gpio_write(uint8_t gpio_number, uint8_t gpio_value)
{
	uint8_t status = 0;

	struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);

	if(gpio_number >= 16 && gpio_number <= 22){
		switch(gpio_number){
		case 16:
			gpiod_set_value(pixxgen_private->exout16, (gpio_value > 0)?1:0);
			break;
		case 17:
			gpiod_set_value(pixxgen_private->exout17, (gpio_value > 0)?1:0);
			break;
		case 18:
			gpiod_set_value(pixxgen_private->exout18, (gpio_value > 0)?1:0);
			break;
		case 19:
			gpiod_set_value(pixxgen_private->exout19, (gpio_value > 0)?1:0);
			break;
		case 20:
			gpiod_set_value(pixxgen_private->exout20, (gpio_value > 0)?1:0);
			break;
		case 21:
			gpiod_set_value(pixxgen_private->exout21, (gpio_value > 0)?1:0);
			break;
		case 22:
			gpiod_set_value(pixxgen_private->exout22, (gpio_value > 0)?1:0);
			break;
		}
	}else{
		status = -1;
	}
	return status;
}
EXPORT_SYMBOL(pixxgen_gpio_write);

int pixxgen_gpio_read(uint8_t gpio_number)
{
	uint8_t status = 0;

	struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);

	if(gpio_number == 6 || gpio_number == 7 || gpio_number == 23){
		switch(gpio_number){
		case 6:
			status = gpiod_get_value(pixxgen_private->exin6);
			break;
		case 7:
			status = gpiod_get_value(pixxgen_private->exin7);
			break;
		case 23:
			status = gpiod_get_value(pixxgen_private->exin23);
			break;
		}
	}else{
		status = -1;
	}
	return status;
}
EXPORT_SYMBOL(pixxgen_gpio_read);
//
/* declare ioctl_function */
static long pixxgen_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	static struct ioctl_info info;
	struct pixxgen_gpio *pixxgen_private;
	struct device *dev;
	ktime_t ktime;
	int ret;

	pixxgen_private = container_of(file->private_data, struct pixxgen_gpio, pixxgen_miscdevice);
	dev = &pixxgen_private->pdev->dev;

	if (copy_from_user(&info, (void __user *)arg, sizeof(info))) {
		return -EFAULT;
	}
	switch (cmd) {
		case SET_GPIO:
			dev_info(dev, "SET_GPIO\n");

			gpiod_set_value(pixxgen_private->exout16, info.exout[0]);
			gpiod_set_value(pixxgen_private->exout17, info.exout[1]);
			gpiod_set_value(pixxgen_private->exout18, info.exout[2]);
			gpiod_set_value(pixxgen_private->exout19, info.exout[3]);
			gpiod_set_value(pixxgen_private->exout20, info.exout[4]);
			gpiod_set_value(pixxgen_private->exout21, info.exout[5]);
			gpiod_set_value(pixxgen_private->exout22, info.exout[6]);
			break;
		case GET_GPIO:
			dev_info(dev, "GET_GPIO\n");

			info.exin[0] = gpiod_get_value(pixxgen_private->exin6);
			info.exin[1] = gpiod_get_value(pixxgen_private->exin7);
			info.exin[2] = gpiod_get_value(pixxgen_private->exin23);

			if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
				return -EFAULT;
			}
			break;
		case START_STEP_MOTOR1:
			dev_info(dev, "START_STEP_MOTOR1\n");
			if(info.st_motor_speed >= 5 && info.st_motor_speed < 10000){
				pixxgen_private->st_motor_period1 = info.st_motor_speed;
			}else{
				pixxgen_private->st_motor_period1 = 1000;
			}
			if(info.st_motor_count > 0){
				pixxgen_private->st_motor_cnt1 = info.st_motor_count;
			}else{
				pixxgen_private->st_motor_cnt1 = 0;
			}
			ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period1));
			hrtimer_start(&pixxgen_private->st_motor_hr_timer1, ktime, HRTIMER_MODE_REL );
			break;
		case STOP_STEP_MOTOR1:
			dev_info(dev, "STOP_STEP_MOTOR1\n");
			ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer1);
			if(ret){
				//dev_info(dev, "The timer was still in use.");
			}
			break;
		case START_STEP_MOTOR2:
			dev_info(dev, "START_STEP_MOTOR2\n");
			if(info.st_motor_speed >= 5 && info.st_motor_speed < 10000){
				pixxgen_private->st_motor_period2 = info.st_motor_speed;
			}else{
				pixxgen_private->st_motor_period2 = 1000;
			}
			if(info.st_motor_count > 0){
				pixxgen_private->st_motor_cnt2 = info.st_motor_count;
			}else{
				pixxgen_private->st_motor_cnt2 = 0;
			}
			ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period2));
			hrtimer_start(&pixxgen_private->st_motor_hr_timer2, ktime, HRTIMER_MODE_REL );
			break;
		case STOP_STEP_MOTOR2:
			dev_info(dev, "STOP_STEP_MOTOR2\n");
			ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer2);
			if(ret){
				//dev_info(dev, "The timer was still in use.");
			}
			break;
		case START_STEP_MOTOR3:
			dev_info(dev, "START_STEP_MOTOR3\n");
			if(info.st_motor_speed >= 5 && info.st_motor_speed < 10000){
				pixxgen_private->st_motor_period3 = info.st_motor_speed;
			}else{
				pixxgen_private->st_motor_period3 = 1000;
			}
			if(info.st_motor_count > 0){
				pixxgen_private->st_motor_cnt3 = info.st_motor_count;
			}else{
				pixxgen_private->st_motor_cnt3 = 0;
			}
			ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period3));
			hrtimer_start(&pixxgen_private->st_motor_hr_timer3, ktime, HRTIMER_MODE_REL );
			break;
		case STOP_STEP_MOTOR3:
			dev_info(dev, "STOP_STEP_MOTOR3\n");
			ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer3);
			if(ret){
				//dev_info(dev, "The timer was still in use.");
			}
			break;
		case START_STEP_MOTOR4:
			dev_info(dev, "START_STEP_MOTOR4\n");
			if(info.st_motor_speed >= 5 && info.st_motor_speed < 10000){
				pixxgen_private->st_motor_period4 = info.st_motor_speed;
			}else{
				pixxgen_private->st_motor_period4 = 1000;
			}
			if(info.st_motor_count > 0){
				pixxgen_private->st_motor_cnt4 = info.st_motor_count;
			}else{
				pixxgen_private->st_motor_cnt4 = 0;
			}
			ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period4));
			hrtimer_start(&pixxgen_private->st_motor_hr_timer4, ktime, HRTIMER_MODE_REL );
			break;
		case STOP_STEP_MOTOR4:
			dev_info(dev, "STOP_STEP_MOTOR4\n");
			ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer4);
			if(ret){
				//dev_info(dev, "The timer was still in use.");
			}
			break;
		default:
			dev_info(dev, "invalid command %d %d \n", cmd, SET_GPIO);
		return -EFAULT;
	}
	return 0;
}

static const struct file_operations pixxgen_fops = {
	.owner = THIS_MODULE,
	.open = pixxgen_open,
	//.read = pixxgen_read,
	.release = pixxgen_close,
	.unlocked_ioctl 	= pixxgen_ioctl,
};

static int pixxgen_probe(struct platform_device *pdev)
{
	int ret, count;
	struct device *dev = &pdev->dev;
	struct pixxgen_gpio *pixxgen_private;
	struct device_node *child;
	ktime_t ktime;

	pdev_export = pdev;
	dev_info(dev, "pixxgen_probe() function is called.\n");

	pixxgen_private = devm_kzalloc(&pdev->dev, sizeof(struct pixxgen_gpio), GFP_KERNEL);
	if(!pixxgen_private){
		dev_err(dev, "failed memory allocation");
		return -ENOMEM;
	}

	pixxgen_private->pdev = pdev;
	platform_set_drvdata(pdev, pixxgen_private);
	//
	count = device_get_child_node_count(dev);
	if (!count)
		return -ENODEV;
	dev_info(dev, "device child node count is %d\n", count);

	for_each_child_of_node(dev->of_node, child){
		const char *label_name, *index_name;

		of_property_read_string(child, "label", &label_name);

		if (!strcmp(label_name,"exout")) {
			of_property_read_string(child, "index", &index_name);
			dev_info(dev, "label name is %s", label_name);
			if(!strcmp(index_name, "16")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout16 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout16)) {
					ret = PTR_ERR(pixxgen_private->exout16);
					return ret;
				}
			}else if(!strcmp(index_name, "17")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout17 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout17)) {
					ret = PTR_ERR(pixxgen_private->exout17);
					return ret;
				}
			}else if(!strcmp(index_name, "18")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout18 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout18)) {
					ret = PTR_ERR(pixxgen_private->exout18);
					return ret;
				}
			}else if(!strcmp(index_name, "19")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout19 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout19)) {
					ret = PTR_ERR(pixxgen_private->exout19);
					return ret;
				}
			}else if(!strcmp(index_name, "20")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout20 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout20)) {
					ret = PTR_ERR(pixxgen_private->exout20);
					return ret;
				}
			}else if(!strcmp(index_name, "21")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout21 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout21)) {
					ret = PTR_ERR(pixxgen_private->exout21);
					return ret;
				}
			}else if(!strcmp(index_name, "22")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exout22 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exout22)) {
					ret = PTR_ERR(pixxgen_private->exout22);
					return ret;
				}
			}
		}else if (!strcmp(label_name,"exin")) {
			of_property_read_string(child, "index", &index_name);
			dev_info(dev, "label name is %s", label_name);
			if(!strcmp(index_name, "6")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exin6 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exin6)) {
					ret = PTR_ERR(pixxgen_private->exin6);
					return ret;
				}
			}else if(!strcmp(index_name, "7")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exin7 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exin7)) {
					ret = PTR_ERR(pixxgen_private->exin7);
					return ret;
				}
			}else if(!strcmp(index_name, "23")){
				dev_info(dev, "index is %s", index_name);
				pixxgen_private->exin23 = devm_gpiod_get_from_of_node(dev, child,
						"gpios", 0, GPIOD_ASIS, label_name);
				if (IS_ERR(pixxgen_private->exin23)) {
					ret = PTR_ERR(pixxgen_private->exin23);
					return ret;
				}
			}
		}
	}
	pixxgen_private->st_motor_period1 = 1000;				//1000uS timer
	pixxgen_private->st_motor_period2 = 1000;				//1000uS timer
	pixxgen_private->st_motor_period3 = 1000;				//1000uS timer
	pixxgen_private->st_motor_period4 = 1000;				//1000uS timer

	ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period1));
	hrtimer_init(&pixxgen_private->st_motor_hr_timer1, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	pixxgen_private->st_motor_hr_timer1.function = &st_motor_hr_timer1;

	ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period2));
	hrtimer_init(&pixxgen_private->st_motor_hr_timer2, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	pixxgen_private->st_motor_hr_timer2.function = &st_motor_hr_timer2;

	ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period3));
	hrtimer_init(&pixxgen_private->st_motor_hr_timer3, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	pixxgen_private->st_motor_hr_timer3.function = &st_motor_hr_timer3;

	ktime = ktime_set( 0, US_TO_NS(pixxgen_private->st_motor_period4));
	hrtimer_init(&pixxgen_private->st_motor_hr_timer4, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	pixxgen_private->st_motor_hr_timer4.function = &st_motor_hr_timer4;

//	gpiod_direction_output(pixxgen_private->exout0, 0);
	gpiod_set_value(pixxgen_private->exout16, 0);
	gpiod_set_value(pixxgen_private->exout17, 0);
	gpiod_set_value(pixxgen_private->exout18, 0);
	gpiod_set_value(pixxgen_private->exout19, 0);
	gpiod_set_value(pixxgen_private->exout20, 0);
	gpiod_set_value(pixxgen_private->exout21, 0);
	gpiod_set_value(pixxgen_private->exout22, 0);
	//

	pixxgen_private->pixxgen_miscdevice.name = DEVICE_NAME;
	pixxgen_private->pixxgen_miscdevice.minor = MISC_DYNAMIC_MINOR;
	pixxgen_private->pixxgen_miscdevice.fops = &pixxgen_fops;
	pixxgen_private->pixxgen_miscdevice.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	ret = misc_register(&pixxgen_private->pixxgen_miscdevice);
	if (ret){
		return ret; /* misc_register returns 0 if success */
	}
	dev_info(dev, "pixxgen_probe() function is completed.\n");
	return 0;
}

/* Add remove() function */
static int __exit pixxgen_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev);
	int ret;

	dev_info(dev, "pixxgen_remove() function is called.\n");

	ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer1);
	if(ret){
		dev_info(dev, "The timer was still in use.");
	}
	ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer2);
	if(ret){
		dev_info(dev, "The timer was still in use.");
	}
	ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer3);
	if(ret){
		dev_info(dev, "The timer was still in use.");
	}
	ret = hrtimer_cancel(&pixxgen_private->st_motor_hr_timer4);
	if(ret){
		dev_info(dev, "The timer was still in use.");
	}
	misc_deregister(&pixxgen_private->pixxgen_miscdevice);

	return 0;
}

/* Declare a list of devices supported by the driver */
static const struct of_device_id pixxgen_of_ids[] = {
	{ .compatible = "brcm,pixxgen_gpio"},
	{},
};

MODULE_DEVICE_TABLE(of, pixxgen_of_ids);

/* Define platform driver structure */
static struct platform_driver pixxgen_platform_driver = {
	.probe = pixxgen_probe,
	.remove = pixxgen_remove,
	.driver = {
		.name = "pixggen_gpio",
		.of_match_table = pixxgen_of_ids,
		.owner = THIS_MODULE,
	}
};

/* Register our platform driver */
module_platform_driver(pixxgen_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kwang Hyuk Ko");
MODULE_DESCRIPTION("This is a Pixxgen GPIO Device Driver ");
