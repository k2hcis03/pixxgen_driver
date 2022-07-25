#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xa5d46ab, "module_layout" },
	{ 0xf6370acd, "i2c_smbus_read_byte_data" },
	{ 0x3f6e182, "i2c_del_driver" },
	{ 0xb3128c7f, "i2c_smbus_write_byte_data" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xc9b34804, "i2c_smbus_read_word_data" },
	{ 0x412aa2e3, "misc_register" },
	{ 0x5f754e5a, "memset" },
	{ 0xc5850110, "printk" },
	{ 0xc6f927e8, "of_property_read_string" },
	{ 0x9f786b7d, "i2c_smbus_read_byte" },
	{ 0x64b6fcc2, "i2c_smbus_write_byte" },
	{ 0xd28c2e41, "_dev_err" },
	{ 0x23ce832f, "i2c_register_driver" },
	{ 0x5c5c0f9d, "_dev_info" },
	{ 0x121353f7, "i2c_smbus_write_block_data" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x7a182ae7, "devm_kmalloc" },
	{ 0xa82b8f02, "misc_deregister" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:MCP23017");
MODULE_ALIAS("i2c:MCP4551");
MODULE_ALIAS("i2c:PCA9540BD");
MODULE_ALIAS("of:N*T*Cbrcm,pixxgen_i2c");
MODULE_ALIAS("of:N*T*Cbrcm,pixxgen_i2cC*");

MODULE_INFO(srcversion, "5AA1E02CE6DEDC31CEC29FA");
