#ifndef ROOTKIT_REGS_H
# define ROOTKIT_REGS_H

# define SAVE_REGS2                                  \
    "push      %rax       \n"                       \
    "push      %rdi       \n"                       \
    "push      %rsi       \n"                       \
    "push      %rdx       \n"                       \
    "push      %rcx       \n"                       \
    "push      %rax       \n"                       \
    "push      %r8        \n"                       \
    "push      %r9        \n"                       \
    "push      %r10       \n"                       \
    "push      %r11       \n"                       \
    "push      %rbx       \n"                       \
    "push      %rbp       \n"                       \
    "push      %r12       \n"                       \
    "push      %r13       \n"                       \
    "push      %r14       \n"                       \
    "push      %r15       \n"

# define RESTORE_REGS2                               \
    "pop       %r15       \n"                       \
    "pop       %r14       \n"                       \
    "pop       %r13       \n"                       \
    "pop       %r12       \n"                       \
    "pop       %rbp       \n"                       \
    "pop       %rbx       \n"                       \
    "pop       %r11       \n"                       \
    "pop       %r10       \n"                       \
    "pop       %r9        \n"                       \
    "pop       %r8        \n"                       \
    "pop       %rax       \n"                       \
    "pop       %rcx       \n"                       \
    "pop       %rdx       \n"                       \
    "pop       %rsi       \n"                       \
    "pop       %rdi       \n"                       \
    "pop       %rax       \n"

#endif /* !ROOTKIT_REGS_H */
