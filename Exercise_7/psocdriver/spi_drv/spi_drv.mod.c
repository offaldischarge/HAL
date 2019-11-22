#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x7305f8c7, "module_layout" },
	{ 0x9c88b607, "spi_bus_type" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0xb81960ca, "snprintf" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x28cc25db, "arm_copy_from_user" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0x8e66a3cd, "spi_sync" },
	{ 0x5f754e5a, "memset" },
	{ 0x504fcc42, "driver_unregister" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x427acfe4, "cdev_del" },
	{ 0x27e95276, "class_destroy" },
	{ 0xcc334da2, "__spi_register_driver" },
	{ 0x88c3bdb4, "__class_create" },
	{ 0x511f2cfb, "cdev_add" },
	{ 0x76cff1fa, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x6ba78371, "device_create" },
	{ 0x17cefbca, "spi_setup" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x78dd2cab, "device_destroy" },
	{ 0x7c32d0f0, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "41F831C255DFB6216F00859");
