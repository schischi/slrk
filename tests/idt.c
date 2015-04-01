#include "slrk.h"
#include "userland.h"
#include "assert.h"
#include "test.h"
#include <linux/kernel.h>

idt_create_hook(0x80);

static int cnt = 0;
static int cnt2 = 0;

noinline void pre_foo(struct pt_regs *regs)
{
    ++cnt;
}

noinline void post_foo(struct pt_regs *regs)
{
    ++cnt2;
}

void test_idt_hijacking(void)
{
    char *argv[2] = { NULL, NULL };

    idt_hook_cfg(0x80, (unsigned long)pre_foo, (unsigned long)post_foo);

    /* Completly change the table */
    idt_substitute_table();
    idt_hook_enable(0x80);
    cnt = cnt2 = 0;
    argv[0] = "/root/rk/int_32";
    user_land_exec(argv);
    assert(cnt != 0, "IDT entry 0x80 pre executed (table changed)");
    assert(cnt2 != 0, "IDT entry 0x80 post executed (table changed)");
    idt_restore_table();
}

#if 0
struct unit_test sysenter_test = {
    .name = "sysenter",
    .n = 4,
    .run = sysenter_test_run,
    .elf = USER_ELF(sysenter_32_user),
};

test_init(sysenter_test);
#endif
