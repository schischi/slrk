#ifndef ROOTKIT_TESTS_H
# define ROOTKIT_TESTS_H

void test_syscall_tbl(void);
void test_idt_hijacking(void);
void test_sysenter(void);
void test_inline_hooking(void);
void test_debug_reg(void);

#endif /* !ROOTKIT_TESTS__H */
