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
	"__fentry__\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "DB1E9DE3F4D4F71087E301B");
