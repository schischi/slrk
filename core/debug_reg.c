#include "log.h"
#include "debug_reg.h"
#include "idt.h"
#include "idt.h"
#include "memory.h"
#include "hook_inline.h"

static enum slrk_dr_hijacking hook_method;

enum dr7_cond {
    DR7_COND_EXEC = 0,
    DR7_COND_WR   = 1,
    DR7_COND_RWX  = 2,
    DR7_COND_RW   = 3
};
enum dr7_len {
    DR7_LEN_1 = 0,
    DR7_LEN_2 = 1,
    DR7_LEN_4 = 3,
    DR7_LEN_8 = 2
};

static struct hw_bp {
    f_dr_hook hdlr;
    void *addr;
    enum dr7_cond cond;
    enum dr7_len len;
    enum {
        ENABLED,
        DISABLED,
        UNUSED,
    } state;
} bp[4] = {
    [0 ... 3] = { .state = UNUSED, .hdlr = NULL }
};

#define DR6_BD(DR6)          ((DR6 >> 13) & 0x1)
#define DR6_CLEAR_BD(DR6)    (DR6 &= ~(1 << 13))
#define DR6_BP(DR6, N)       ((DR6 >> (N)) & 0x1)
#define DR6_CLEAR_BP(DR6, N) (DR6 &= ~(1 << (N)))

#define DR7_LOCAL_EXACT(DR7, S)              \
    DR7 = ((DR7 & ~(1 << 8))                 \
                |  (S << 8))
#define DR7_GLOBAL_EXACT(DR7, S)             \
    DR7 = ((DR7 & ~(1 << 9))                 \
                |  (S << 9))
#define DR7_GENERAL_DETECT(DR7, S)           \
    DR7 = ((DR7 & ~(1 << 13))                \
                |  (S << 13))
#define DR7_LOCAL_BP(DR7, N, S)              \
    DR7 = ((DR7 & ~(0x1 << ((N) * 2)))       \
                |  (  S << ((N) * 2)))
#define DR7_GLOBAL_BP(DR7, N, S)             \
    DR7 = ((DR7 & ~(0x1 << ((N) * 2 + 1)))   \
                |  (  S << ((N) * 2 + 1)))
#define DR7_COND(DR7, N, COND)               \
    DR7 = ((DR7 & ~(0x3  << (20 + (N) * 4))) \
                |  (COND << (20 + (N) * 4)))
#define DR7_LEN(DR7, N, LEN)                 \
    DR7 = ((DR7 & ~(0x3 << (18 + (N) * 4)))  \
                |  (LEN << (18 + (N) * 4)))

#define __dr_set(n, val) \
    asm volatile("mov %0, %%db"#n : : "r"(val))

#define __dr_get(n, val) \
    asm volatile("mov %%db"#n", %0\n" : "=r"(val))

#define REG_LIST X(0) X(1) X(2) X(3) X(6) X(7)

#if 0
static unsigned long dr_get(int n)
{
    unsigned long ret;
    switch (n) {
#define X(N) case N: __dr_get(N, ret); break;
        REG_LIST
#undef X
    }
    return ret;
}
#endif

static void dr_set(int n, unsigned long val)
{
    switch (n) {
#define X(N) case N: __dr_set(N, val); break;
        REG_LIST
#undef X
    }
}

#if 0
static void skip_mov_instr(struct slrk_regs *regs)
{
    /* mov %reg, %reg is 3 byte long */
    regs->ip += 3;
}
#endif

#include <linux/delay.h>
static noinline int int1_pre_hook(struct slrk_regs *regs, int err)
{
    int i;
    unsigned long dr6;

    __dr_get(6, dr6);

#if 0
    /* An instruction R/W a debug register */
    if (DR6_BD(dr6)) {
        DR6_CLEAR_BD(dr6);
        __dr_set(6, dr6);
        //skip_mov_instr(regs);
        pr_log("RW dr!\n");
        return 1; //skip original handler
    }
#endif

    for (i = 0; i < 4; ++i) {
        if (bp[i].state == ENABLED && DR6_BP(dr6, i)) {
            DR6_CLEAR_BP(dr6, i);
            regs->rflags |= X86_EFLAGS_RF;
            regs->rflags &= ~X86_EFLAGS_TF;
            if (bp[i].hdlr)
                bp[i].hdlr(regs, err);
            __dr_set(6, dr6);
            return 0; //iret, not original handler
        }
    }
    return -1; //original handler, no post hook
}

int dr_hook(void *addr, f_dr_hook hook)
{
    int i;

    for (i = 0; i < 4; ++i) {
        if (bp[i].state == UNUSED) {
            bp[i].state = ENABLED;
            bp[i].addr = addr;
            bp[i].cond = DR7_COND_EXEC;
            bp[i].len = DR7_LEN_1;
            bp[i].hdlr = hook;
            return i;
        }
    }
    return -1;
}

int dr_protect_mem(void *addr, size_t n, enum slrk_dr_mem_prot p)
{
    return -1;
}

void dr_enable(int n)
{
    unsigned long dr7 = 0;
    bp[n].state = ENABLED;

    __dr_get(7, dr7);

    DR7_GLOBAL_BP(dr7, n, 1);
    DR7_LOCAL_BP(dr7, n, 0);
    DR7_COND(dr7, n, bp[n].cond);
    DR7_LEN(dr7, n, bp[n].len);

    dr_set(n, (unsigned long)bp[n].addr);
    __dr_set(7, dr7);
}

void dr_disable(int n)
{
    unsigned long dr7;
    bp[n].state = DISABLED;

    __dr_get(7, dr7);
    DR7_GLOBAL_BP(dr7, n, 0);
    DR7_LOCAL_BP(dr7, n, 0);
    __dr_set(7, dr7);
    dr_set(n, 0);
}

void dr_delete(int n)
{
    dr_disable(n);
    bp[n].state = UNUSED;
}

void dr_init(enum slrk_dr_hijacking m)
{
    unsigned long dr7 = 0;
    hook_method = m;

    DR7_LOCAL_EXACT(dr7, 1);
    DR7_GLOBAL_EXACT(dr7, 1);
    DR7_GENERAL_DETECT(dr7, 0);
    __dr_set(7, dr7);

    switch (hook_method) {
        case INLINE_HOOK:
            //inline_hook_init(do_debug, );
            break;
        case IDT_HOOK:
            idt_set_hook(0x1, int1_pre_hook, NULL);
            idt_hook_enable(0x1);
            //asm volatile ("int $1\n");
            break;
    }
}

void dr_cleanup(void)
{
    switch (hook_method) {
        case INLINE_HOOK:
            break;
        case IDT_HOOK:
            idt_hook_disable(0x1);
            break;
    }
}
