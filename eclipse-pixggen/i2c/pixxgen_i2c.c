/*
 * pixxgen_i2c.c
 *
 *  Created on: 2021. 10. 19.
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

#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/i2c.h>
#include "../ioctl.h"

#define DEVICE_NAME 		"pixxgen_i2c"
#define ADC_CHANNEL_NUM	8

#define GPIO_A			1
#define GPIO_B			2
#define GPIO_ALL			3

#define CW					1
#define CCW				2
#define STOP				3

struct pixxgen_i2c{
//	struct platform_device *pdev;
	struct i2c_client * i2c_client;
	struct miscdevice pixxgen_miscdevice;
};

struct i2c_client * i2c_client_export;
int (*fp1)(uint8_t gpio_number, uint16_t gpio_value);
int (*fp2)(uint8_t gpio_number);

static int pixxgen_open(struct inode *inode, struct file *file)
{
	struct pixxgen_i2c * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_i2c, pixxgen_miscdevice);
	//dev = &pixxgen_private->pdev->dev;
	dev = &pixxgen_private->i2c_client->dev;
	dev_info(dev, "pixxgen_dev_open() is called.\n");
	return 0;
}

static int pixxgen_close(struct inode *inode, struct file *file)
{
	struct pixxgen_i2c * pixxgen_private;
	struct device *dev;
	pixxgen_private = container_of(file->private_data, struct pixxgen_i2c, pixxgen_miscdevice);
	//dev = &pixxgen_private->pdev->dev;
	dev = &pixxgen_private->i2c_client->dev;

	dev_info(dev, "pixxgen_dev_close() is called.\n");
	return 0;
}

static int pixxgen_read(struct file *file, char __user *buff,
	               size_t count, loff_t *off)
{
	struct pixxgen_i2c * pixxgen_private;
	struct device *dev;

	pixxgen_private = container_of(file->private_data, struct pixxgen_i2c,
			pixxgen_miscdevice);
	dev = &pixxgen_private->i2c_client->dev;
	dev_info(dev, "pixxgen_read entered\n");

	*off+=count;

	return count;
}

/* declare ioctl_function */
static long pixxgen_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	static struct ioctl_info info;
	struct pixxgen_i2c *pixxgen_private;
	struct device *dev;
	u8 data[20];

	pixxgen_private = container_of(file->private_data, struct pixxgen_i2c, pixxgen_miscdevice);
	dev = &pixxgen_private->i2c_client->dev;
	if (copy_from_user(&info, (void __user *)arg, sizeof(info))) {
		return -EFAULT;
	}
	switch (cmd) {
	case SET_I2C_GPIO_A:
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x14, info.gpio_i2c_out_a);
		dev_info(dev, "SET_I2C_GPIO_A %d\n", info.gpio_i2c_out_a);
		break;
	case SET_I2C_GPIO_B:
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x15, info.gpio_i2c_out_b);
		dev_info(dev, "SET_I2C_GPIO_B %d\n", info.gpio_i2c_out_b);
		break;
	case GET_I2C_GPIO_A:
		info.gpio_i2c_in_a = i2c_smbus_read_byte_data(pixxgen_private->i2c_client, 0x12);
		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			return -EFAULT;
		}
		break;
	case GET_I2C_GPIO_B:
		info.gpio_i2c_in_b = i2c_smbus_read_byte_data(pixxgen_private->i2c_client, 0x13);
		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			return -EFAULT;
		}
		break;
	case SET_I2C_POT:
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x00, info.pot_i2c_write);
		dev_info(dev, "set pot value is %d", info.pot_i2c_write);
		break;
	case GET_I2C_POT:
		info.pot_i2c_read = i2c_smbus_read_word_data(pixxgen_private->i2c_client, 0x0C);
		info.pot_i2c_read = info.pot_i2c_read >> 8;
		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			return -EFAULT;
		}
		dev_info(dev, "get pot value is %d", info.pot_i2c_read);
		break;
	case SET_COLLIMATOR_MUX:
		i2c_smbus_write_byte(pixxgen_private->i2c_client, info.collimator_mux_i2c_write);	//0x04 = ch0, 0x05 = ch1
		dev_info(dev, "SET_COLLIMATOR_MUX %d\n", info.collimator_mux_i2c_write);
		break;
	case SET_COLLIMATOR:

		data[0] = (info.collimator_motor_count >> 24) & 0xFF;
		data[1] = (info.collimator_motor_count >> 16) & 0xFF;
		data[2] = (info.collimator_motor_count >> 8) & 0xFF;
		data[3] = info.collimator_motor_count & 0xFF;

		data[4] = info.collimator_motor_sensor;
		data[5] = info.collimator_motor_acc;
		data[6] = info.collimator_motor_speed;
		data[7] = info.collimator_motor_sleep;
		data[8] = info.collimator_motor_mode;
		data[9] = info.collimator_motor_laser;
		data[10] = info.collimator_motor_res;
		i2c_smbus_write_block_data(pixxgen_private->i2c_client, info.collimator_motor_number, 11, data);
		dev_info(dev, "SET_COLLIMATOR %d\n", info.collimator_motor_count);
		break;
	case GET_COLLIMATOR:
		info.collimator_mux_i2c_read = i2c_smbus_read_byte(pixxgen_private->i2c_client);
		dev_info(dev, "GET_COLLIMATOR %d\n", info.collimator_mux_i2c_read);

		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			return -EFAULT;
		}
		break;
//	case SET_COLLIMATOR_2:
//		i2c_smbus_write_byte(pixxgen_private->i2c_client, 0x05);
//		dev_info(dev, "SET_I2C_COLLIMATOR_2 %d\n", 0x05);
//		break;
//	case GET_COLLIMATOR_2:
//		info.collimator_mux_i2c_read = i2c_smbus_read_byte(pixxgen_private->i2c_client);
//		dev_info(dev, "GET_I2C_COLLIMATOR_2 %d\n", info.collimator_mux_i2c_read);
//
//		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
//			return -EFAULT;
//		}
		break;
	default:
		dev_info(dev, "invalid command %d\n", cmd);
	return -EFAULT;
	}
	return 0;
}
// gpio_number is GPIO_A, GPIO_B, GPIO_ALL
// gpio_value MSB GPIO_B, LSB GPIO_A
int pixxgen_gpio_i2c_write_0(uint8_t gpio_number, uint16_t gpio_value)
{
	int status = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		pr_info("SET_I2C_GPIO_A %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		break;
	case GPIO_B:
		pr_info("SET_I2C_GPIO_B %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value);
		break;
	case GPIO_ALL:
		pr_info("SET_I2C_GPIO_ALL %X\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value>>8);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}
int pixxgen_gpio_i2c_write_1(uint8_t gpio_number, uint16_t gpio_value)
{
	int status = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		pr_info("SET_I2C_GPIO_A %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		break;
	case GPIO_B:
		pr_info("SET_I2C_GPIO_B %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value);
		break;
	case GPIO_ALL:
		pr_info("SET_I2C_GPIO_ALL %X\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value>>8);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}
int pixxgen_gpio_i2c_write_2(uint8_t gpio_number, uint16_t gpio_value)
{
	int status = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		pr_info("SET_I2C_GPIO_A %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		break;
	case GPIO_B:
		pr_info("SET_I2C_GPIO_B %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value);
		break;
	case GPIO_ALL:
		pr_info("SET_I2C_GPIO_ALL %X\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value>>8);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}
int pixxgen_gpio_i2c_write_3(uint8_t gpio_number, uint16_t gpio_value)
{
	int status = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		pr_info("SET_I2C_GPIO_A %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		break;
	case GPIO_B:
		pr_info("SET_I2C_GPIO_B %d\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value);
		break;
	case GPIO_ALL:
		pr_info("SET_I2C_GPIO_ALL %X\n", gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x14, (uint8_t)gpio_value);
		i2c_smbus_write_byte_data(i2c_client_export, 0x15, (uint8_t)gpio_value>>8);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}
EXPORT_SYMBOL(fp1);

// gpio_number is GPIO_A, GPIO_B, GPIO_ALL
int pixxgen_gpio_i2c_read_0(uint8_t gpio_number)
{
	int status = 0;
	int temp = 0, temp2 = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		pr_info("GET_I2C_GPIO_A %d\n", status);
		break;
	case GPIO_B:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		pr_info("GET_I2C_GPIO_B %d\n", status);
		break;
	case GPIO_ALL:
		temp = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		temp2 = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		status = temp2 << 8 | temp;
		pr_info("GET_I2C_GPIO_ALL %X\n", status);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}

int pixxgen_gpio_i2c_read_1(uint8_t gpio_number)
{
	int status = 0;
	int temp = 0, temp2 = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		pr_info("GET_I2C_GPIO_A %d\n", status);
		break;
	case GPIO_B:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		pr_info("GET_I2C_GPIO_B %d\n", status);
		break;
	case GPIO_ALL:
		temp = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		temp2 = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		status = temp2 << 8 | temp;
		pr_info("GET_I2C_GPIO_ALL %X\n", status);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}

int pixxgen_gpio_i2c_read_2(uint8_t gpio_number)
{
	int status = 0;
	int temp = 0, temp2 = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		pr_info("GET_I2C_GPIO_A %d\n", status);
		break;
	case GPIO_B:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		pr_info("GET_I2C_GPIO_B %d\n", status);
		break;
	case GPIO_ALL:
		temp = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		temp2 = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		status = temp2 << 8 | temp;
		pr_info("GET_I2C_GPIO_ALL %X\n", status);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}

int pixxgen_gpio_i2c_read_3(uint8_t gpio_number)
{
	int status = 0;
	int temp = 0, temp2 = 0;
	//struct pixxgen_gpio *pixxgen_private = platform_get_drvdata(pdev_export);
	switch(gpio_number){
	case GPIO_A:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		pr_info("GET_I2C_GPIO_A %d\n", status);
		break;
	case GPIO_B:
		status = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		pr_info("GET_I2C_GPIO_B %d\n", status);
		break;
	case GPIO_ALL:
		temp = i2c_smbus_read_byte_data(i2c_client_export, 0x12);
		temp2 = i2c_smbus_read_byte_data(i2c_client_export, 0x13);
		status = temp2 << 8 | temp;
		pr_info("GET_I2C_GPIO_ALL %X\n", status);
		break;
	default:
		status = -1;
		break;
	}
	return status;
}
EXPORT_SYMBOL(fp2);

static const struct file_operations pixxgen_fops = {
	.owner = THIS_MODULE,
	.open = pixxgen_open,
	.read = pixxgen_read,
	.release = pixxgen_close,
	.unlocked_ioctl 	= pixxgen_ioctl,
};

static int pixxgen_probe(struct i2c_client * client, const struct i2c_device_id * id)
{
	int ret;
	struct device *dev = &client->dev;
	struct pixxgen_i2c *pixxgen_private;
	const char *labe_name;
	dev_info(dev, "pixxgen_probe() function is called. \n");

	pixxgen_private = devm_kzalloc(&client->dev, sizeof(struct pixxgen_i2c), GFP_KERNEL);
	if(!pixxgen_private){
		dev_err(dev, "failed memory allocation");
		return -ENOMEM;
	}

	pixxgen_private->i2c_client = client;
	i2c_set_clientdata(client, pixxgen_private);

	i2c_client_export = client;
//
	of_property_read_string(dev->of_node, "label", &labe_name);
	if(!strcmp(labe_name, "pixxgen_gpio_i2c_0")){
		fp1 = pixxgen_gpio_i2c_write_0;
		fp2 = pixxgen_gpio_i2c_read_0;
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x00, 0xFF);		//all input
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x01, 0xFF);		//all input
	}else if(!strcmp(labe_name, "pixxgen_gpio_i2c_1")){
		fp1 = pixxgen_gpio_i2c_write_1;
		fp2 = pixxgen_gpio_i2c_read_1;
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x00, 0xFF);		//all input
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x01, 0xFF);		//all input
	}else if(!strcmp(labe_name, "pixxgen_gpio_i2c_2")){
		fp1 = pixxgen_gpio_i2c_write_2;
		fp2 = pixxgen_gpio_i2c_read_2;
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x00, 0x00);		//all output
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x01, 0x00);		//all output
	}else if(!strcmp(labe_name, "pixxgen_gpio_i2c_3")){
		fp1 = pixxgen_gpio_i2c_write_3;
		fp2 = pixxgen_gpio_i2c_read_3;
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x00, 0x00);		//all output
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x01, 0x0F);		//input, output
	}else if(!strcmp(labe_name, "pixxgen_pot")){
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x40, 0x0F);		// general call mode enable
		i2c_smbus_write_byte_data(pixxgen_private->i2c_client, 0x00, 0x50);		//pot -> 0
	}else if(!strcmp(labe_name, "pixxgen_collimator_mux")){

	}else if(!strcmp(labe_name, "pixxgen_collimator")){

	}else{
		return -ENODEV;
	}
	//dev_info(dev, "fp1 and fp2 function pointer is %s", (char *)fp1);
//
	/* Attach the I2C device to the private structure */
	pixxgen_private->pixxgen_miscdevice.name = labe_name;
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
static int __exit pixxgen_remove(struct i2c_client * client)
{
	struct device *dev = &client->dev;
	struct pixxgen_i2c *pixxgen_private = i2c_get_clientdata(client);
	misc_deregister(&pixxgen_private->pixxgen_miscdevice);
	dev_info(dev, "pixxgen_remove() misc_deregister is called.\n");
	return 0;
}

static const struct of_device_id pixxgen_of_ids[] = {
	{ .compatible = "brcm,pixxgen_i2c", },
	{ }
};
MODULE_DEVICE_TABLE(of, pixxgen_of_ids);

static const struct i2c_device_id pixxgen_id[] = {
	{ .name = "MCP23017", },
	{ .name = "MCP4551",},
	{ .name = "PCA9540BD",},
	{ }
};
MODULE_DEVICE_TABLE(i2c, pixxgen_id);

static struct i2c_driver pixxgen_platform_driver = {
	.driver = {
		.name = "pixxgen_i2c",
		.owner = THIS_MODULE,
		.of_match_table = pixxgen_of_ids,
	},
	.probe   = pixxgen_probe,
	.remove  = pixxgen_remove,
	.id_table	= pixxgen_id,
};
/* Register our platform driver */
module_i2c_driver(pixxgen_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kwang Hyuk Ko");
MODULE_DESCRIPTION("This is a pixxgen i2c Device Driver ");
