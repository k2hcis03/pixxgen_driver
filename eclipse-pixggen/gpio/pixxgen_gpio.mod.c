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
	{ 0x97229b46, "devm_gpiod_get_from_of_node" },
	{ 0x5cc2a511, "hrtimer_forward" },
	{ 0x695bf5e9, "hrtimer_cancel" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x8861c53a, "__platform_driver_register" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x412aa2e3, "misc_register" },
	{ 0xec523f88, "hrtimer_start_range_ns" },
	{ 0x5f754e5a, "memset" },
	{ 0x2d07bea4, "device_get_child_node_count" },
	{ 0xc6f927e8, "of_property_read_string" },
	{ 0xd28c2e41, "_dev_err" },
	{ 0x5c5c0f9d, "_dev_info" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0xc9caf271, "of_get_next_child" },
	{ 0xfe0d2bde, "gpiod_get_value" },
	{ 0xa362bf8f, "hrtimer_init" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x5025d193, "platform_driver_unregister" },
	{ 0x214a501, "gpiod_set_value" },
	{ 0x7a182ae7, "devm_kmalloc" },
	{ 0xa82b8f02, "misc_deregister" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cbrcm,pixxgen_gpio");
MODULE_ALIAS("of:N*T*Cbrcm,pixxgen_gpioC*");

MODULE_INFO(srcversion, "1D8F60F4C52521CE55327ED");
