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

#define __dr_set(n, val) \
    asm volatile("mov %0, %%db"#n : : "r"(val))

#define __dr_get(n, val) \
    asm volatile("mov %%db"#n", %0\n" : "=r"(val))

#include <linux/delay.h>
static int debug_reg_run(void)
{
    int bp;

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

#if 0
    /* Memory protection */
    unsigned long protected_var = 1;
    bp = dr_protect_mem(&protected_var, 0x1, CONST_VAL);
    *((volatile unsigned long*)&protected_var) = 37;
    pr_log("Protected var is 0x%p\n", &protected_var);
    dr_enable(bp);
//    pr_log("Break at 0x%p\n", &&br);
//    mdelay(3 * 1000);
//br:
    pr_log("===> %ld\n", *((volatile unsigned long*)&protected_var));
    //assert(protected_var == 1, "dr protect address with const value");
    dr_delete(bp);
#endif

    dr_cleanup();
    return 0;
}

struct unit_test debug_reg_test = {
    .name = "debug registers",
    .n = 2,
    .run = debug_reg_run,
};

test_init(debug_reg_test);
