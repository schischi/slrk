#include "int80.h"
#include "regs.h"
#include "idt.h"

static void do_nothing(void) {}

extern unsigned long idt_hook;
extern int idt_hook_sz;

extern void idt_fake_iret_80(void);
asm (
    ".globl idt_fake_iret_80\n"
    ".align 8, 0x90\n"
    "idt_fake_iret_80:\n"
    "callq *(idt_hook + 32 * 0x80 + 16)\n"
    "iretq\n"
);
extern void idt_fake_hdlr_80(void);
asm (
    ".globl idt_fake_hdlr_80\n"
    ".align 8, 0x90\n"
    "idt_fake_hdlr_80:\n"
    "movq %rsp, %r8\n"
    "sub $0x28, %rsp\n"                      /* fake cpu exception frame */
    "movq $0x18, 0x20(%rsp)\n"               /* SS                       */
    "movq %r8, 0x18(%rsp)\n"                 /* RSP                      */
    "movq $0x0, 0x10(%rsp)\n"                /* EFLAGS                   */
    "movq $0x10, 0x8(%rsp)\n"                /* CS                       */
    "movq $idt_fake_iret_80, 0x0(%rsp)\n"    /* RIP                      */
    SAVE_REGS2
    "mov %rsp, %rdi\n"                       /* struct pt_regs as arg    */
    "callq *(idt_hook + 32 * 0x80 + 0)\n"
    RESTORE_REGS2
    "jmp *(idt_hook + 32 * 0x80 + 8)\n"
);

void int80_hook_enable(void)
{
    idt_hook_enable(0x80);
}

void int80_set_hook(void *pre, void *post)
{
    if (!pre)
        pre = do_nothing;
    if (!post)
        post = do_nothing;
    idt_set_hdlr(0x80, idt_fake_hdlr_80, pre, post);
}

void int80_hook_disable(void)
{
    idt_hook_disable(0x80);
}
