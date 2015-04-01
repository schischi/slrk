#include "log.h"
#include "symbol.h"
#include "memory.h"

#include <linux/kallsyms.h>
#define __NR_syscall_tbl_max 220 //FIXME

unsigned long *sys_call_table = NULL;
unsigned long *ia32_sys_call_table = NULL;

unsigned long sys_call_table_backup[__NR_syscall_tbl_max + 1];
unsigned long ia32_sys_call_table_backup[__NR_syscall_tbl_max + 1];

void syscall_tbl_fetch(void)
{
    if (sys_call_table)
        return;
    sys_call_table = (unsigned long *)symbol_retrieve("sys_call_table");
    memcpy(sys_call_table_backup, sys_call_table, __NR_syscall_tbl_max + 1);
    set_addr_rw(sys_call_table);
}

void syscall_tbl_set_entry(int n, void *fct)
{
    sys_call_table[n] = (unsigned long)fct;
}

unsigned long syscall_tbl_get_entry(int n)
{
    return sys_call_table[n];
}

unsigned long syscall_tbl_orig_entry(int n)
{
    return sys_call_table_backup[n];
}

void syscall_tbl_restore_entry(int n)
{
    sys_call_table[n] = sys_call_table_backup[n];
}

void syscall_tbl_restore(void)
{
    memcpy(sys_call_table, sys_call_table_backup, __NR_syscall_tbl_max + 1);
}

void ia32_syscall_tbl_fetch(void)
{
    if (ia32_sys_call_table)
        return;
    ia32_sys_call_table = (unsigned long *)symbol_retrieve("ia32_sys_call_table");
    memcpy(ia32_sys_call_table_backup, ia32_sys_call_table, __NR_syscall_tbl_max + 1);
    set_addr_rw(ia32_sys_call_table);
}

void ia32_syscall_tbl_set_entry(int n, void *fct)
{
    ia32_sys_call_table[n] = (unsigned long)fct;
}

unsigned long ia32_syscall_tbl_get_entry(int n)
{
    return ia32_sys_call_table[n];
}

unsigned long ia32_syscall_tbl_orig_entry(int n)
{
    return ia32_sys_call_table_backup[n];
}

void ia32_syscall_tbl_restore_entry(int n)
{
    ia32_sys_call_table[n] = ia32_sys_call_table_backup[n];
}

void ia32_syscall_tbl_restore(void)
{
    memcpy(ia32_sys_call_table, ia32_sys_call_table_backup, __NR_syscall_tbl_max + 1);
}
