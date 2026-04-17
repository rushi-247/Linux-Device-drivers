#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xe8213e80, "_printk" },
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0x653aa194, "class_create" },
	{ 0x223cc85c, "__platform_driver_register" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0x4695bf9b, "platform_driver_unregister" },
	{ 0xa1dacb42, "class_destroy" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xfaabfe5e, "kmalloc_caches" },
	{ 0xc064623f, "__kmalloc_cache_noprof" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0xd5f66efd, "cdev_init" },
	{ 0x8ea73856, "cdev_add" },
	{ 0xe486c4b7, "device_create" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x4e54d6ac, "cdev_del" },
	{ 0xd272d446, "__fentry__" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0xe8213e80,
	0x9f222e1e,
	0x653aa194,
	0x223cc85c,
	0x0bc5fb0d,
	0x4695bf9b,
	0xa1dacb42,
	0xbd03ed67,
	0xfaabfe5e,
	0xc064623f,
	0xd710adbf,
	0xd5f66efd,
	0x8ea73856,
	0xe486c4b7,
	0xcb8b6ec6,
	0x4e54d6ac,
	0xd272d446,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__x86_return_thunk\0"
	"_printk\0"
	"alloc_chrdev_region\0"
	"class_create\0"
	"__platform_driver_register\0"
	"unregister_chrdev_region\0"
	"platform_driver_unregister\0"
	"class_destroy\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"__kmalloc_noprof\0"
	"cdev_init\0"
	"cdev_add\0"
	"device_create\0"
	"kfree\0"
	"cdev_del\0"
	"__fentry__\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "448BA4C33A7EF4389A2FF6C");
