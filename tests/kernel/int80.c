#include "slrk.h"
#include "userland.h"
#include "assert.h"
#include "test.h"
#include <linux/kernel.h>

idt_create_hook(0x80);

static int cnt = 0;
static int cnt2 = 0;

static char *argv[2] = {
    USER_ELF_PATH"int80_user", NULL
};

static noinline void pre_foo(struct pt_regs *regs)
{
    ++cnt;
}

static noinline void post_foo(struct pt_regs *regs)
{
    ++cnt2;
}

int int80_test_run(void)
{
    idt_hook_cfg(0x80, (unsigned long)pre_foo, (unsigned long)post_foo);

    /* Completly change the table */
    idt_substitute_table();
    idt_hook_enable(0x80);
    cnt = cnt2 = 0;
    user_land_exec(argv);
    assert(cnt != 0, "IDT entry 0x80 pre executed (table changed)");
    assert(cnt2 != 0, "IDT entry 0x80 post executed (table changed)");
    idt_restore_table();

    return 0;
}

EXPORT_USER_ELF(int80_user);

struct unit_test int80_test = {
    .name = "int 0x80",
    .n = 2,
    .run = int80_test_run,
    .elf = USER_ELF(int80_user),
};

test_init(int80_test);
