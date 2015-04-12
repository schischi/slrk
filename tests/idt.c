#include "slrk.h"
#include "userland.h"
#include "assert.h"
#include "test.h"
#include <linux/kernel.h>

static int cnt = 0;
static int cnt2 = 0;

static char *argv[2] = {
    USER_ELF_PATH"int80_user", NULL
};


static noinline int pre_foo(struct pt_regs *regs)
{
    ++cnt;
    return 0;
}

static noinline void post_foo(struct pt_regs *regs)
{
    ++cnt2;
}

static noinline int pre_foo2(struct pt_regs *regs)
{
    ++cnt;
    return 2;
}

static noinline int pf_hook(struct pt_regs *regs, int err)
{
    int cr2;

    asm volatile (
        "mov %%cr2, %%rax\n\t"
        "mov %%eax, %0\n\t"
        : "=m" (cr2) :: "%rax"
    );
    if (cr2 == 0x42cafe42) {
        cnt = err & 0x1f;
        return 7;
    }
    return -1;
}

int idt_test_run(void)
{
    idt_substitute(IDT_TABLE);

    idt_set_hook(0x80, pre_foo, post_foo);
    cnt = cnt2 = 0;
    idt_hook_enable(0x80);
    user_land_exec(argv);
    barrier();
    assert(cnt != 0, "IDT @0x80 pre hook, with orig");
    assert(cnt2 != 0, "IDT @0x80 post hook, with orig");
    cnt = cnt2 = 0;
    idt_hook_disable(0x80);
    user_land_exec(argv);
    assert(cnt == 0, "IDT @0x80 restored");

    idt_set_hook(0x6, pre_foo2, NULL);
    cnt = 0;
    idt_hook_enable(0x6);
    asm("ud2\n");
    barrier();
    assert(cnt != 0, "IDT @0x6 pre hook, skip orig");
    idt_hook_disable(0x6);

    idt_set_hook(0xE, pf_hook, NULL);
    cnt = 0;
    idt_hook_enable(0xE);
    asm("callq *(0x42cafe42)\n");
    barrier();
    assert(cnt == 16, "IDT @0xE pre hook, error code, orig");
    idt_hook_disable(0xE);

    idt_restore();

    return 0;
}

EXPORT_USER_ELF(int80_user);

struct unit_test idt_test = {
    .name = "idt",
    .n = 5,
    .run = idt_test_run,
    .elf = USER_ELF(int80_user),
};

test_init(idt_test);
