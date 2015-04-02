#include "slrk.h"
#include "userland.h"
#include "assert.h"
#include "test.h"
#include <linux/kernel.h>

static int cnt = 0;

static char *argv[2] = {
    USER_ELF_PATH"int80_user", NULL
};


static noinline int pre_foo(struct pt_regs *regs)
{
    ++cnt;
    return 0;
}

static noinline int pre_foo2(struct pt_regs *regs)
{
    ++cnt;
    return 2;
}

int idt_test_run(void)
{
    idt_set_pre_hook(0x80, pre_foo);
    cnt = 0;
    idt_hook_enable(0x80);
    user_land_exec(argv);
    assert(cnt != 0, "IDT @0x80 pre hook, with orig");
    cnt = 0;
    idt_hook_disable(0x80);
    user_land_exec(argv);
    assert(cnt == 0, "IDT @0x80 restored");

    idt_set_pre_hook(0x6, pre_foo2);
    cnt = 0;
    idt_hook_enable(0x6);
    asm("ud2\n");
    assert(cnt != 0, "IDT @0x6 pre hook, skip orig");
    idt_hook_disable(0x6);

    return 0;
}

struct unit_test idt_test = {
    .name = "idt",
    .n = 3,
    .run = idt_test_run,
};

test_init(idt_test);
