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
	{ 0x2630c30c, "driver_unregister" },
	{ 0xae5cadc7, "__spi_register_driver" },
	{ 0x412aa2e3, "misc_register" },
	{ 0xcb7e04f6, "sysfs_create_group" },
	{ 0x5f754e5a, "memset" },
	{ 0x9409d179, "kmem_cache_alloc_trace" },
	{ 0x6a021660, "kmalloc_caches" },
	{ 0x24bb1780, "wake_up_process" },
	{ 0x9def985f, "kthread_create_on_node" },
	{ 0x870d5a1c, "__init_swait_queue_head" },
	{ 0xa362bf8f, "hrtimer_init" },
	{ 0xd28c2e41, "_dev_err" },
	{ 0x7a182ae7, "devm_kmalloc" },
	{ 0xec523f88, "hrtimer_start_range_ns" },
	{ 0xfd238c07, "remap_pfn_range" },
	{ 0x97cae7bd, "fasync_helper" },
	{ 0x314b20c8, "scnprintf" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xa82b8f02, "misc_deregister" },
	{ 0xac1d46, "sysfs_remove_group" },
	{ 0x695bf5e9, "hrtimer_cancel" },
	{ 0x37a0cba, "kfree" },
	{ 0xa36ab4bb, "kthread_stop" },
	{ 0x5cc2a511, "hrtimer_forward" },
	{ 0xc37335b0, "complete" },
	{ 0xb969fd6b, "kill_fasync" },
	{ 0x80169eff, "spi_sync" },
	{ 0x30745185, "wait_for_completion_interruptible" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xff178f6, "__aeabi_idivmod" },
	{ 0x5c5c0f9d, "_dev_info" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:MCP3208");
MODULE_ALIAS("of:N*T*Cbrcm,pixggen_adc");
MODULE_ALIAS("of:N*T*Cbrcm,pixggen_adcC*");

MODULE_INFO(srcversion, "B579351198A109C1EC08A72");
