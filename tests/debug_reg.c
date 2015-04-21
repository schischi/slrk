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

static noinline void my_dr_hook(struct slrk_regs *regs, long err)
{
    ++hcnt;
}

static int debug_reg_run(void)
{
    int bp;

    /* IDT hook 0x1 */
    hcnt = fcnt = 0;
    dr_init(IDT_HOOK);
    bp = dr_hook(debug_reg_test_fct, my_dr_hook);
    dr_enable(bp);
    debug_reg_test_fct();
    assert(hcnt == 1 && fcnt == 1, "debug register with inline hook");
    /* Remove hook */
    dr_delete(bp);
    dr_cleanup();
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
