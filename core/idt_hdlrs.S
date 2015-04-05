#define STRUCT_IDT_HOOK_SZ 32
.extern idt_hook

.macro save_regs
  push %rax
  push %rdi
  push %rsi
  push %rdx
  push %rcx
  push %rax
  push %r8
  push %r9
  push %r10
  push %r11
  push %rbx
  push %rbp
  push %r12
  push %r13
  push %r14
  push %r15
.endm

.macro restore_regs
  pop %r15
  pop %r14
  pop %r13
  pop %r12
  pop %rbp
  pop %rbx
  pop %r11
  pop %r10
  pop %r9
  pop %r8
  pop %rax
  pop %rcx
  pop %rdx
  pop %rsi
  pop %rdi
  pop %rax
.endm

/* This macro generates a new generic handler for an IDT entry
 *   - save the registers
 *   - execute the pre-hook
 *   - restore the registers
 *   - if the pre-hook return a zero value the original handler is executed
 *   - otherwise if the value is positive we increment the return address
 *     with this value (useful to skip an opcode like ud2 or int $0x3)
 *   - return from the interrupt
 */
.macro gen_idt_hdlr from, to
    save_regs
    callq *(idt_hook + STRUCT_IDT_HOOK_SZ * \from + 0)
    test %rax, %rax
    jnz idt_iret
    restore_regs
    jmp *(idt_hook + STRUCT_IDT_HOOK_SZ * \from + 8)
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
    /* Increment the return address with the result of the pre-hook */
    add %rax, 0x80(%rsp)
    restore_regs
    iretq
