#include "slrk.h"
#include "userland.h"
#include "assert.h"
#include "test.h"
#include <linux/kernel.h>

static int cnt = 0;
static int cnt2 = 0;
static char *argv[2] = {
    USER_ELF_PATH"sysenter_32_user", NULL
};

static noinline void sysenter_pre_hook(struct slrk_regs *regs)
{
    if (regs->rax == 4)
        ++cnt;
}

static noinline void sysenter_post_hook(struct slrk_regs *pre,
        struct slrk_regs *post)
{
    if (pre->rax == 4 && post->rax == -9)
        ++cnt2;
}

static int sysenter_test_run(void)
{
    cnt = cnt2 = 0;
    sysenter_hook_cfg(sysenter_pre_hook, sysenter_post_hook);
    sysenter_hook_enable();
    user_land_exec(argv);
    assert(cnt != 0, "sysenter pre hook");
    assert(cnt2 != 0, "sysenter post hook");

    cnt = cnt2 = 0;
    sysenter_hook_disable();
    user_land_exec(argv);
    assert(cnt == 0 && cnt2 == 0, "sysenter entry restored");

    return 0;
}

EXPORT_USER_ELF(sysenter_32_user);

struct unit_test sysenter_test = {
    .name = "sysenter",
    .n = 4,
    .run = sysenter_test_run,
    .elf = USER_ELF(sysenter_32_user),
};

//test_init(sysenter_test);
