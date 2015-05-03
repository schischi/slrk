#include "asm_utils.S"

.extern idt_hook

#define STRUCT_IDT_HOOK_SZ 32
#define HAS_ERROR_CODE     (   \from == 0x08 \
                            || \from == 0x0A \
                            || \from == 0x0B \
                            || \from == 0x0C \
                            || \from == 0x0D \
                            || \from == 0x0E \
                            || \from == 0x11 \
                            || \from == 0x1E)

/* This macro generates a new generic handler for an IDT entry
 *   - save the registers
 *   - execute the pre-hook
 *   - restore the registers
 *   - pre-hook return value:
 *       = 0 -> return from the handler (without executing the original one)
 *       > 0 -> execute the original handler without the post-hook
 *       < 0 -> push a fake exception frame that return in the post-hook
 *   - return from the interrupt
 */
.macro gen_idt_hdlr from, to
    cli
    .if !HAS_ERROR_CODE
        pushq $0x0
    .endif
    movq %rsp, %r8
    movq 0x0(%rsp), %r9
    save_regs
    movq %rsp, %rdi                         /* 1st arg: slrk_regs  */
    movq %r8, %rsi                          /* 2nd arg: err_code */
    callq *(idt_hook \
        + STRUCT_IDT_HOOK_SZ * \from + 0)
    cmp $0, %eax
    restore_regs
    je idt_iret    /* jmp if ret == 0 */
    jg 2f          /* jmp if ret > 0 */

    sub $0x28, %rsp                         /* fake cpu exception frame */
    movq $0x18, 0x20(%rsp)                  /* SS                       */
    movq %r8, 0x18(%rsp)                    /* RSP                      */
    movq $0x0, 0x10(%rsp)                   /* EFLAGS                   */
    movq $0x10, 0x8(%rsp)                   /* CS                       */
    movq $1f, %r8
    movq %r8, 0x0(%rsp)                     /* RIP                      */
    .if HAS_ERROR_CODE
        pushq %r9
    .endif
    jmp 3f
    2:
    .if !HAS_ERROR_CODE
        add $0x8, %rsp   /* Pop error code */
    .else
        .space 4, 0x90
    .endif
    3:
    jmp *(idt_hook \
        + STRUCT_IDT_HOOK_SZ * \from + 8)

    1:
    popq %rsi                               /* Pop error code */
    save_regs
    movq %rsp, %rdi                         /* 1st arg: slrk_regs  */
    callq *(idt_hook \
        + STRUCT_IDT_HOOK_SZ * \from + 16)
    restore_regs
    iretq

    .align 8, 0x90
    .if \to-\from
        gen_idt_hdlr "(\from+1)", \to
    .endif
.endm

/* Generate the 256 handlers.
 * Split the commands to avoid having a macro that nest too deeply.
 */
.align 8, 0x90
.globl idt_fake_hdlrs
idt_fake_hdlrs:
gen_idt_hdlr 0,99
gen_idt_hdlr 100, 199
gen_idt_hdlr 200, 255

.globl idt_hook_sz
idt_hook_sz:
    .quad (. - idt_fake_hdlrs) / 256

idt_iret:
    add $0x8, %rsp   /* Pop error code */
    iretq
