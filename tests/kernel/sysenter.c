#include "slrk.h"
#include "userland.h"
#include "assert.h"
#include <linux/kernel.h>

static int cnt = 0;
static int cnt2 = 0;

static noinline void sysenter_pre_hook(struct pt_regs *regs)
{
    if (regs->ax == 4)
        ++cnt;
}

static noinline void sysenter_post_hook(struct pt_regs *pre,
        struct pt_regs *post)
{
    if (pre->ax == 4 && post->ax == -9)
        ++cnt2;
}

#include <linux/delay.h>
void test_sysenter(void)
{
    char *argv[2] = { "/root/rk/sysenter_32", NULL };

    sysenter_hook_cfg(sysenter_pre_hook, sysenter_post_hook);
    sysenter_hook_enable();
    cnt = cnt2 = 0;
    user_land_exec(argv);
    assert(cnt != 0, "sysenter pre hook");
    assert(cnt2 != 0, "sysenter post hook");

    cnt = cnt2 = 0;
    sysenter_hook_disable();
    user_land_exec(argv);
    assert(cnt == 0 && cnt2 == 0, "sysenter entry restored");
}
