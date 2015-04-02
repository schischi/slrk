#define STRUCT_IDT_HOOK_SZ 32
#define IDT_HOOK_SZ 128

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

.macro gen_idt_hdlr from, to
    save_regs
    callq *(idt_hook + STRUCT_IDT_HOOK_SZ * \from + 0)
    test %rax, %rax
    jnz idt_iret
    restore_regs
    jmp *(idt_hook + STRUCT_IDT_HOOK_SZ * \from + 8)
    .align IDT_HOOK_SZ, 0x90
    .if \to-\from
        gen_idt_hdlr "(\from+1)", \to
    .endif
.endm

.globl idt_fake_hdlrs
idt_fake_hdlrs:
gen_idt_hdlr 0, 49
gen_idt_hdlr 50, 99
gen_idt_hdlr 100, 199
gen_idt_hdlr 200, 255

.globl idt_hook_sz
idt_hook_sz:
    .quad IDT_HOOK_SZ

idt_iret:
    add %rax, 0x80(%rsp)
    restore_regs
    iretq
