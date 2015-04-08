#include "log.h"
#include "debug_reg.h"
#include "idt.h"
#include "idt.h"
#include "memory.h"
#include "hook_inline.h"
#include <asm/debugreg.h>
#include <linux/kallsyms.h>

static unsigned long orig_int1_handler;
static struct in_hook do_debug_inline_hk;
static int hook_method;

static void(*pre_hdlr)(struct pt_regs *, long);
static void(*post_hdlr)(struct pt_regs *, long);
static void(*do_debug_fct)(struct pt_regs *, long);

static struct hw_bp {
    void(*hdlr)(struct pt_regs *regs, long err);
    unsigned long addr;
    int cond;
    int len;
    enum {
        ENABLED,
        DISABLED,
        UNUSED,
    } state;
} bp[4] = {
    [0 ... 3] = { .state = UNUSED, .hdlr = NULL }
};

#define DR6_BD (1 << 13)
#define DR7_LE (1 << 8) //local exact bp enable
#define DR7_GE (1 << 9) //global exact bp enable
#define DR7_GD (1 << 13) //general detect enable (set BD flag on mov)
#define DR7_LE_REG(N) (0x1 << (N * 2))
#define DR7_GE_REG(N) (0x2 << (N * 2))

#define __debug_reg_set(n, val) \
    asm volatile("mov %0, %%db"#n : : "r"(val))

#define __debug_reg_get(n, val) \
    asm volatile("mov %%db"#n", %0\n" : "=r"(val))

#define REG_LIST X(0) X(1) X(2) X(3) X(6) X(7)

#if 0
static unsigned long debug_reg_get(int n)
{
    unsigned long ret;
    switch (n) {
#define X(N) case N: __debug_reg_get(N, ret); break;
        REG_LIST
#undef X
    }
    return ret;
}
#endif

static void debug_reg_set(int n, unsigned long val)
{
    switch (n) {
#define X(N) case N: __debug_reg_set(N, val); break;
        REG_LIST
#undef X
    }
}

static void skip_mov_instr(struct pt_regs *regs)
{
    /* mov %reg, %reg is 3 byte long */
    regs->ip += 3;
}

#include <linux/delay.h>
static noinline void fake_int1_handler(struct pt_regs *regs, long err)
{
    int i;
    unsigned long dr6, dr7;

    if (pre_hdlr)
        pre_hdlr(regs, err);
    __debug_reg_get(6, dr6);
    __debug_reg_get(7, dr7);

#if 0
    /* Someone R/W a debug register */
    if (dr6 & DR6_BD) {
        dr6 &= ~DR6_BD;
        //skip_mov_instr(regs);
        pr_log("RW dr!\n");
    }
#endif

    for (i = 0; i < 4; ++i) {
        if (bp[i].state == ENABLED && dr6 & (1 << i)) {
            dr6 &= ~(1 << i);
            regs->flags |= X86_EFLAGS_RF;
            regs->flags &= ~X86_EFLAGS_TF;
            if (bp[i].hdlr)
                bp[i].hdlr(regs, err);
            __debug_reg_set(6, dr6);
            goto orig_hdlr;
        }
    }

orig_hdlr:
    if (post_hdlr)
        post_hdlr(regs, err);
    __debug_reg_set(7, dr7);
    //asm volatile(
    //    "push %r8\n"
    //    "pushf\n"
    //    "pop %r8\n"
    //    "or $0x10000, %r8\n"
    //    "push %r8\n"
    //    "popf\n"
    //    "pop %r8\n"
    //);
}

void debug_register_enable_bp(int n)
{
    unsigned long dr7;
    bp[n].state = ENABLED;

    __debug_reg_get(7, dr7);
    dr7 |= (bp[n].cond | (bp[n].len << 2)) << (16 + n * 4)
        | DR7_GE_REG(n)
        | DR7_GE;
    //   | DR7_GD;
    dr7 &= ~DR7_GD;
    debug_reg_set(n, bp[n].addr);
    __debug_reg_set(7, dr7);
}

void debug_register_disable_bp(int n)
{
    unsigned long dr7;
    bp[n].state = DISABLED;

    __debug_reg_get(7, dr7);
    dr7 &= ~((bp[n].cond | (bp[n].len << 2)) << (16 + n * 4) | DR7_GE_REG(n));
    debug_reg_set(n, 0);
    __debug_reg_set(7, dr7);
}

int debug_register_set_bp(void *addr,
        void(*hook)(struct pt_regs *regs, long err), int n)
{
    bp[n].state = ENABLED;
    bp[n].addr = (unsigned long)addr;
    bp[n].cond = DR_RW_EXECUTE;
    bp[n].len = DR_LEN_1;
    bp[n].hdlr = hook;
    return n;
}

int debug_register_add_bp(void *addr,
        void(*hook)(struct pt_regs *regs, long err))
{
    int i;

    for (i = 0; i < 4; ++i)
        if (bp[i].state == UNUSED)
            return debug_register_set_bp(addr, hook, i);
    return -1;
}

void debug_register_del_bp(int n)
{
    debug_register_disable_bp(n);
    bp[n].state = UNUSED;
}

static noinline void disable_hdlr_hook(struct pt_regs *regs, long err)
{
    inline_hook_disable(&do_debug_inline_hk);
}

static noinline void enable_hdlr_hook(struct pt_regs *regs, long err)
{
    do_debug_fct(regs, err);
    inline_hook_enable(&do_debug_inline_hk);
}

void debug_register_hijack_handler(int _hook_method)
{
    hook_method = _hook_method;

    switch (hook_method) {
        case INLINE_HOOK:
            //FIXME
            do_debug_fct = (void *)kallsyms_lookup_name("do_debug");
            inline_hook_init((unsigned long)do_debug_fct,
                    (unsigned long)fake_int1_handler,
                    REL_JMP,
                    &do_debug_inline_hk);
            pre_hdlr = disable_hdlr_hook;
            post_hdlr = enable_hdlr_hook;
            inline_hook_enable(&do_debug_inline_hk);
            break;
#if 0
        case IDT_TABLE:
            pre_hdlr = NULL;
            post_hdlr = NULL;
            orig_int1_handler = idt_get_entry(0x1);
            hook_config(do_debug_hk, orig_int1_handler,
                    (unsigned long)fake_int1_handler);
            if (!idt_spoofed())
                idt_substitute_table();
            //pr_log("orig = 0x%p\n", orig_int1_handler);
            //pr_log("fake = 0x%p\n", fake_int1_handler);
            //pr_log("gate = 0x%p\n", do_debug_hk);
        case IDT_ENTRY:
            idt_set_entry(do_debug_hk, 0x1);
            break;
#endif
    }
}

void debug_register_unhijack_handler(void)
{
    switch (hook_method) {
        case INLINE_HOOK:
            inline_hook_disable(&do_debug_inline_hk);
            break;
        case IDT_TABLE:
        case IDT_ENTRY:
            idt_set_entry(orig_int1_handler, 0x1);
            break;
    }
}
