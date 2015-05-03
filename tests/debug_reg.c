#include "slrk.h"
#include "test.h"
#include "userland.h"
#include "assert.h"
#include <linux/kernel.h>

static int hcnt = 0;
static int fcnt = 0;
static volatile unsigned long protected_var = 0;

static noinline void debug_reg_test_fct(void)
{
    ++fcnt;
}

static noinline void my_dr_hook(struct slrk_regs *regs, long err)
{
    ++hcnt;
}

#define __dr_set(n, val) \
    asm volatile("mov %0, %%db"#n : : "r"(val))

#define __dr_get(n, val) \
    asm volatile("mov %%db"#n", %0\n" : "=r"(val))

static int debug_reg_run(void)
{
    int bp;
    unsigned long bp_value = 42;

    dr_init(IDT_HOOK);
    /* Function hooking */
    hcnt = fcnt = 0;
    bp = dr_hook(debug_reg_test_fct, my_dr_hook, HIDE_VOID);
    dr_enable(bp);
    debug_reg_test_fct();
    assert(hcnt == 1 && fcnt == 1, "debug register hook (IDT)");
    /* Remove hook */
    dr_delete(bp);
    hcnt = fcnt = 0;
    debug_reg_test_fct();
    assert(hcnt == 0 && fcnt == 1, "debug register cleanup (IDT)");


    dr_cleanup();
    return 0;
}

struct unit_test debug_reg_test = {
    .name = "debug registers",
    .n = 2,
    .run = debug_reg_run,
};

test_init(debug_reg_test);
