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
