#include "log.h"
#include "debug_reg.h"
#include "idt.h"
#include "idt.h"
#include "memory.h"
#include "hook_inline.h"

extern asmlinkage void *__start_dr_code;
extern asmlinkage void *__end_dr_code;

static enum slrk_dr_hijacking hook_method;

enum dr7_cond {
    DR7_COND_EXEC  = 0,
    DR7_COND_WRITE = 1,
    DR7_COND_IO    = 2,
    DR7_COND_RW    = 3
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
    unsigned long fake_value;
    unsigned long real_value;
    bool wait_trap;
    enum dr7_cond cond;
    enum dr7_len len;
    enum slrk_dr_hiding hiding_method;
    enum {
        ENABLED,
        DISABLED,
        UNUSED,
    } state;
} bp[4] = {
    [0 ... 3] = { .state = UNUSED, .hdlr = NULL, .fake_value = 0,
                  .wait_trap = false }
};

union dr7 {
    struct {
        unsigned long l0    : 1;
        unsigned long g0    : 1;
        unsigned long l1    : 1;
        unsigned long g1    : 1;
        unsigned long l2    : 1;
        unsigned long g2    : 1;
        unsigned long l3    : 1;
        unsigned long g3    : 1;
        unsigned long le    : 1;
        unsigned long ge    : 1;
        unsigned long       : 3;
        unsigned long gd    : 1;
        unsigned long       : 2;
        unsigned long cond0 : 2;
        unsigned long len0  : 2;
        unsigned long cond1 : 2;
        unsigned long len1  : 2;
        unsigned long cond2 : 2;
        unsigned long len2  : 2;
        unsigned long cond3 : 2;
        unsigned long len3  : 2;
        unsigned long       : 32;
    };
    unsigned long value;
};

#define DR6_BD(DR6)          ((DR6 >> 13) & 0x1)
#define DR6_CLEAR_BD(DR6)    (DR6 &= ~(1 << 13))
#define DR6_BP(DR6, N)       ((DR6 >> (N)) & 0x1)
#define DR6_CLEAR_BP(DR6, N) (DR6 &= ~(1 << (N)))

#define __dr_set(n, val) \
    asm volatile("mov %0, %%db"#n : : "r"(val))

#define __dr_get(n, val) \
    asm volatile("mov %%db"#n", %0\n" : "=r"(val))

#define REG_LIST X(0) X(1) X(2) X(3) X(6) X(7)

static unsigned long dr_get(int n)
{
    unsigned long ret = 0;
    switch (n) {
#define X(N) case N: __dr_get(N, ret); break;
        REG_LIST
#undef X
    }
    return ret;
}

static void dr_set(int n, unsigned long val)
{
    switch (n) {
#define X(N) case N: __dr_set(N, val); break;
        REG_LIST
#undef X
    }
}

#if 0
static void exec_opcode(int read, unsigned long *reg, unsigned int dr_number)
{
    //int dr_value;

    if (read) {
        *reg = dr_get(dr_number);
        pr_log("Read dr%u = 0x%lx\n", dr_number, *reg);
    }
    else { //write
        unsigned long val = *reg;
        if (dr_number == 7)
            DR7_GENERAL_DETECT(val, 0);
        dr_set(dr_number, val);
        pr_log("Write dr%d = 0x%lx\n", dr_number, *reg);
    }
}

static int decode_dr_move(struct slrk_regs *regs, unsigned int *dr_number,
        unsigned long *value, int *read)
{
    unsigned long reg;
    unsigned char *addr = (char *)regs->rip;
    unsigned long *reg_ptr;
    int base = addr[0] == 0x41;

    pr_log("Opcode: 0x%.2x 0x%.2x 0x%.2x 0x%.2x (0x%p)\n",
            addr[0], addr[1], addr[2], addr[3], addr);

    *read = addr[base + 1] == 0x21;
    *dr_number = ((addr[base + 2] - 0xc0) / 0x8);
    reg = (addr[base + 2] - 0xc0) % 0x8;
    switch (reg) {
        case 0: reg_ptr = base ? &regs->r8  : &regs->rax; break;
        case 1: reg_ptr = base ? &regs->r9  : &regs->rcx; break;
        case 2: reg_ptr = base ? &regs->r10 : &regs->rdx; break;
        case 3: reg_ptr = base ? &regs->r11 : &regs->rbx; break;
        case 4: reg_ptr = base ? &regs->r12 : &regs->rsp; break;
        case 5: reg_ptr = base ? &regs->r13 : &regs->rbp; break;
        case 6: reg_ptr = base ? &regs->r14 : &regs->rsi; break;
        case 7: reg_ptr = base ? &regs->r15 : &regs->rdi; break;
    }
    if (addr[base] != 0x0f) {
        return 0;
    }
    exec_opcode(*read, reg_ptr, *dr_number);
    //int self = (regs->rip >= (unsigned long)&__start_dr_code
    //            && regs->rip <= (unsigned long)&__end_dr_code);
    //pr_log("Instruction was %s dr%d (self=%d)\n",
    //        *read ? "read" : "write",
    //        *dr_number,
    //        self);
    return 3 + base;
}
#endif

static void dr_mem_access_hdlr(struct slrk_regs *regs, long err);

static int int1_pre_hook(struct slrk_regs *regs, int err)
{
    int i;
    union dr7 dr7;
    unsigned long dr6;

    __dr_get(6, dr6);
    __dr_get(7, dr7);

#if 0
    /* An instruction R/W a debug register */
    if (DR6_BD(dr6)) {
        unsigned int dr_number;
        unsigned long value;
        int read;
        len = decode_dr_move(regs, &dr_number, &value, &read);
        regs->rip += len;
        pr_log("Inc 0x%lx, %u\n", regs->rip, len);
            //regs->rflags |= X86_EFLAGS_RF;
            regs->rflags &= ~X86_EFLAGS_TF;
        DR6_CLEAR_BD(dr6); //remove ?
        __dr_set(6, dr6);
        DR7_GENERAL_DETECT(dr7, 1);
        __dr_set(7, dr7);
        return IRET;
    }
#endif

    /* A breakpoint has been triggered */
    for (i = 0; i < 4; ++i) {
        if (bp[i].state == ENABLED && DR6_BP(dr6, i)) {
            //pr_log("BP %d triggered\n", i);
            DR6_CLEAR_BP(dr6, i);
            if (bp[i].hdlr)
                bp[i].hdlr(regs, err);
            /* Access to memory ? */
            if (bp[i].hdlr == dr_mem_access_hdlr) {
                bp[i].wait_trap = true;
                regs->rflags |= X86_EFLAGS_TF;
                dr_disable(i);
                switch (bp[i].len) {
                    case DR7_LEN_1:
                        bp[i].real_value = *((u8*)bp[i].addr);
                        *((u8*)bp[i].addr) = (u8)bp[i].fake_value;
                        break;
                    case DR7_LEN_2:
                        bp[i].real_value = *((u16*)bp[i].addr);
                        *((u16*)bp[i].addr) = (u16)bp[i].fake_value;
                        break;
                    case DR7_LEN_4:
                        bp[i].real_value = *((u32*)bp[i].addr);
                        *((u32*)bp[i].addr) = (u32)bp[i].fake_value;
                        break;
                    case DR7_LEN_8:
                        bp[i].real_value = *((u64*)bp[i].addr);
                        *((u64*)bp[i].addr) = (u64)bp[i].fake_value;
                        break;
                }
                //pr_log("Restored value 0x%lx\n", bp[i].fake_value);
                dr_enable(i);
            }
            else {
                bp[i].wait_trap = false;
                regs->rflags &= ~X86_EFLAGS_TF;
            }
            regs->rflags |= X86_EFLAGS_RF;
            //pr_log("RSI = 0x%lx\n", regs->rsi);
            __dr_set(6, dr6);
            dr7.gd = 0;
            __dr_set(7, dr7);
            return IRET;
        }
    }
    /* Got a Trap */
    //FIXME: wait trap from processus %d
    for (i = 0; i < 4; ++i) {
        if (bp[i].wait_trap) {
            //pr_log("Trap of BP %d triggered\n", i);
            /* Restore the original value */
            dr_disable(i);
            switch (bp[i].len) {
                case DR7_LEN_1:
                    *((u8*)bp[i].addr) = (u8)bp[i].real_value;
                    break;
                case DR7_LEN_2:
                    *((u16*)bp[i].addr) = (u16)bp[i].real_value;
                    break;
                case DR7_LEN_4:
                    *((u32*)bp[i].addr) = (u32)bp[i].real_value;
                    break;
                case DR7_LEN_8:
                    *((u64*)bp[i].addr) = (u64)bp[i].real_value;
                    break;
            }
                //pr_log("Restored value 0x%lx\n", bp[i].real_value);
            dr_enable(i);
            /* Resume the execution */
            regs->rflags |= X86_EFLAGS_RF;
            regs->rflags &= ~X86_EFLAGS_TF;
            bp[i].wait_trap = false;
            dr7.gd = 0;
            __dr_set(7, dr7);
            return IRET;
        }
    }

    dr7.gd = 0;
    __dr_set(7, dr7);
    return IRET_ORIG;
}

int dr_hook(void *addr, f_dr_hook hook, enum slrk_dr_hiding h)
{
    int i;

    for (i = 0; i < 4; ++i) {
        if (bp[i].state == UNUSED) {
            bp[i].state = ENABLED;
            bp[i].hiding_method = h;
            bp[i].fake_value = 0;
            bp[i].addr = addr;
            bp[i].cond = DR7_COND_EXEC;
            bp[i].len = DR7_LEN_1; //len *must* be 1 with COND_EXEC
            bp[i].hdlr = hook;
            return i;
        }
    }
    return -1;
}

static void dr_mem_access_hdlr(struct slrk_regs *regs, long err)
{
    int i;
    pr_log("Memory access at 0x%lx\n", regs->rip);
    for (i = 0; i < 8; ++i) {
        pr_log("\t0x%.2x\n", ((unsigned char *)regs->rip)[i]);
    }
}

static int dr_protect_mem_add(void *addr, size_t len)
{
    int i;

    /* Address must be aligned */
    BUG_ON((unsigned long)addr % len != 0);

    for (i = 0; i < 4; ++i) {
        if (bp[i].state == UNUSED) {
            bp[i].state = ENABLED;
            bp[i].fake_value = 0;
            bp[i].addr = addr;
            bp[i].cond = DR7_COND_RW;
            switch (len) {
                case 8:  bp[i].len = DR7_LEN_8; break;
                case 4:  bp[i].len = DR7_LEN_4; break;
                case 2:  bp[i].len = DR7_LEN_2; break;
                default: bp[i].len = DR7_LEN_1; break;
            }
            bp[i].hdlr = dr_mem_access_hdlr;
            return i;
        }
    }
    return -1;
}

int dr_protect_mem(void *addr, size_t n, enum slrk_dr_mem_prot p)
{
    int i;

    i = dr_protect_mem_add(addr, n);

    if (i != -1) {
        switch (p) {
            case CONST_VAL:
                bp[i].fake_value = *((unsigned long *)addr)
                                 & ((1 << (n * 8)) - 1);
                break;
            case DYN_VAL:
                break;
        }
    }
    return i;
}

void dr_enable(int n)
{
    union dr7 dr7;
    bp[n].state = ENABLED;

    __dr_get(7, dr7);

    switch (n) {
        case 0: dr7.g0 = 1; dr7.cond0 = bp[n].cond; dr7.len0 = bp[n].len; break;
        case 1: dr7.g1 = 1; dr7.cond1 = bp[n].cond; dr7.len1 = bp[n].len; break;
        case 2: dr7.g2 = 1; dr7.cond2 = bp[n].cond; dr7.len2 = bp[n].len; break;
        case 3: dr7.g3 = 1; dr7.cond3 = bp[n].cond; dr7.len3 = bp[n].len; break;
    }

    dr_set(n, (unsigned long)bp[n].addr);
    __dr_set(7, dr7);
}

void dr_disable(int n)
{
    union dr7 dr7;

    bp[n].state = DISABLED;
    __dr_get(7, dr7);
    switch (n) {
        case 0: dr7.l0 = dr7.g0 = 0; break;
        case 1: dr7.l1 = dr7.g1 = 0; break;
        case 2: dr7.l2 = dr7.g2 = 0; break;
        case 3: dr7.l3 = dr7.g3 = 0; break;
    }
    dr_set(n, 0);
    __dr_set(7, dr7);
}

void dr_delete(int n)
{
    dr_disable(n);
    dr_set(n, 0);
    bp[n].state = UNUSED;
}

void dr_init(enum slrk_dr_hijacking m)
{
    union dr7 dr7;
    unsigned long zero = 0;
    hook_method = m;

    dr7.value = 0;
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

    //TODO: enable DE in cr4 ? (enable COND_IO)
    dr7.le = 1;
    dr7.ge = 1;
    //FIXME: GD seems buggy under kvm... Disable it for the moment.
    dr7.gd = 0;
    __dr_set(0, zero);
    __dr_set(1, zero);
    __dr_set(2, zero);
    __dr_set(3, zero);
    __dr_set(7, dr7);

}

void dr_cleanup(void)
{
    unsigned long zero = 0;

    switch (hook_method) {
        case INLINE_HOOK:
            break;
        case IDT_HOOK:
            idt_hook_disable(0x1);
            break;
    }
    __dr_set(7, zero);
}

void dr_print(void)
{
    unsigned int i;
    unsigned long dr, dr6, dr7;

    for (i = 0; i < 4; ++i) {
        dr = dr_get(i);
        pr_log("DR%d: 0x%lx\n", i, dr);
    }
    __dr_get(6, dr6);
    __dr_get(7, dr7);
    pr_log("DR7: 0x%lx", dr7);
    for (i = 0; i < 3; ++i) {
        pr_log("BP %u:\n"
           "\tLE   : 0x%x\n"
           "\tGE   : 0x%x\n"
           "\tCOND : 0x%x\n"
           "\tLEN  : 0x%x\n",
           i,
           !!(dr7 & (0x1 << (i * 2))),
           !!(dr7 & (0x2 << (i * 2))),
           !!(dr7 & (0x3 << (16 + 4 * i))),
           !!(dr7 & (0x3 << (18 + 4 * i))));
    }
}
