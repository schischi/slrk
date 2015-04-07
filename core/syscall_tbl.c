#include "log.h"
#include "symbol.h"
#include "memory.h"
#include "syscall_tbl.h"

#include <linux/kallsyms.h>

#define syscall_nr_max 512

extern int syscall_nr[];

static unsigned long *sys_call_table = NULL;
static unsigned long *ia32_sys_call_table = NULL;

static unsigned long sys_call_table_backup[syscall_nr_max];
static unsigned long ia32_sys_call_table_backup[syscall_nr_max];

int syscall_tbl_fetch(enum syscall_mode m)
{
    int ret = 0;

    if (m & x86_64) {
        if (sys_call_table)
            return 1;
        sys_call_table = (unsigned long *)symbol_retrieve("sys_call_table");
        memcpy(sys_call_table_backup, sys_call_table, syscall_nr_max);
        set_addr_rw(sys_call_table);
        ret |= !sys_call_table;
    }
    if (m & x86) {
        if (ia32_sys_call_table)
            return 1;
        ia32_sys_call_table =
            (unsigned long *)symbol_retrieve("ia32_sys_call_table");
        memcpy(ia32_sys_call_table_backup, ia32_sys_call_table, syscall_nr_max);
        set_addr_rw(ia32_sys_call_table);
        ret |= !ia32_sys_call_table;
    }
    return ret;
}

void syscall_tbl_set(int n, void *fct, enum syscall_mode m)
{
    if (m & x86_64)
        sys_call_table[n] = (unsigned long)fct;
    else if (m & x86)
        ia32_sys_call_table[syscall_nr[n]] = (unsigned long)fct;
}

unsigned long syscall_tbl_get(int n, enum syscall_mode m)
{
    if (m & x86_64)
        return sys_call_table[n];
    if (m & x86)
        return ia32_sys_call_table[syscall_nr[n]];
    return 0;
}

unsigned long syscall_tbl_orig(int n, enum syscall_mode m)
{
    if (m & x86_64)
        return sys_call_table_backup[n];
    if (m & x86)
        return ia32_sys_call_table_backup[syscall_nr[n]];
    return 0;
}

void syscall_tbl_restore(int n, enum syscall_mode m)
{
    if (m & x86_64)
        sys_call_table[n] = sys_call_table_backup[n];
    if (m & x86)
        ia32_sys_call_table[syscall_nr[n]] =
            ia32_sys_call_table_backup[syscall_nr[n]];
}

void syscall_tbl_restore_all(enum syscall_mode m)
{
    if (m & x86_64)
        memcpy(sys_call_table, sys_call_table_backup, syscall_nr_max);
    if (m & x86)
        memcpy(ia32_sys_call_table, ia32_sys_call_table_backup, syscall_nr_max);
}
