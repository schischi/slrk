#include "slrk.h"
#include "test.h"
#include "userland.h"
#include "assert.h"
#include <linux/kernel.h>

static int hcnt = 0;
static int fcnt = 0;

static noinline void debug_reg_test_fct(void)
{
    ++fcnt;
}

static noinline void my_dr_hook(struct pt_regs *regs, long err)
{
    ++hcnt;
}

static int debug_reg_run(void)
{
    int bp;

    /* Inline hook do_debug */
    hcnt = fcnt = 0;
    debug_register_hijack_handler(INLINE_HOOK);
    bp = debug_register_add_bp(debug_reg_test_fct, my_dr_hook);
    debug_register_enable_bp(bp);
    debug_reg_test_fct();
    assert(hcnt == 1 && fcnt == 1, "debug register with inline hook");
    debug_register_del_bp(bp);
    debug_register_unhijack_handler();
    /* Remove hook */
    hcnt = fcnt = 0;
    debug_reg_test_fct();
    assert(hcnt == 0 && fcnt == 1, "debug register remove inline hook");

    return 0;
}

struct unit_test debug_reg_test = {
    .name = "debug registers",
    .n = 2,
    .run = debug_reg_run,
};

test_init(debug_reg_test);
