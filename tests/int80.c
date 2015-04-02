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

static noinline int post_foo(struct pt_regs *regs)
{
    ++cnt2;
    return 0;
}

#include <linux/delay.h>
int int80_test_run(void)
{
    int80_set_hook(pre_foo, post_foo);

    int80_hook_enable();

    cnt = cnt2 = 0;
    user_land_exec(argv);
    assert(cnt != 0, "IDT entry 0x80 pre executed");
    assert(cnt2 != 0, "IDT entry 0x80 post executed");

    int80_hook_disable();

    cnt = cnt2 = 0;
    user_land_exec(argv);
    assert(cnt == 0 && cnt2 == 0, "IDT entry 0x80 restored");

    return 0;
}

EXPORT_USER_ELF(int80_user);

struct unit_test int80_test = {
    .name = "int 0x80",
    .n = 3,
    .run = int80_test_run,
    .elf = USER_ELF(int80_user),
};

test_init(int80_test);
