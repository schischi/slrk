#include "log.h"
#include "symbol.h"
#include "memory.h"
#include "syscall_tbl.h"

#include <linux/kallsyms.h>

#define syscall_nr_max 512

extern int syscall_nr[];

static void *map_shadow;
static void *map_shadow_32;
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
        sys_call_table = shadow_mapping(sys_call_table, syscall_nr_max *
                sizeof(unsigned long), &map_shadow);
        memcpy(sys_call_table_backup, sys_call_table, syscall_nr_max);
        ret |= !sys_call_table;
    }
    if (m & x86) {
        if (ia32_sys_call_table)
            return 1;
        ia32_sys_call_table =
            (unsigned long *)symbol_retrieve("ia32_sys_call_table");
        ia32_sys_call_table = shadow_mapping(ia32_sys_call_table,
                syscall_nr_max * sizeof(unsigned long), &map_shadow_32);
        memcpy(ia32_sys_call_table_backup, ia32_sys_call_table, syscall_nr_max);
        ret |= !ia32_sys_call_table;
    }
    return ret;
}

void syscall_tbl_set(int n, void *fct, enum syscall_mode m)
{
    if (m & x86_64) {
        set_addr_rw(sys_call_table);
        sys_call_table[n] = (unsigned long)fct;
        set_addr_ro(sys_call_table);
    }
    else if (m & x86) {
        set_addr_rw(ia32_sys_call_table);
        ia32_sys_call_table[syscall_nr[n]] = (unsigned long)fct;
        set_addr_ro(ia32_sys_call_table);
    }
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
    if (m & x86_64) {
        set_addr_rw(sys_call_table);
        sys_call_table[n] = sys_call_table_backup[n];
        del_shadow_mapping(map_shadow);
    }
    if (m & x86) {
        set_addr_rw(ia32_sys_call_table);
        ia32_sys_call_table[syscall_nr[n]] =
            ia32_sys_call_table_backup[syscall_nr[n]];
        del_shadow_mapping(map_shadow_32);
    }
}

void syscall_tbl_restore_all(enum syscall_mode m)
{
    if (m & x86_64) {
        set_addr_rw(sys_call_table);
        memcpy(sys_call_table, sys_call_table_backup, syscall_nr_max);
        set_addr_ro(sys_call_table);
    }
    if (m & x86) {
        set_addr_rw(ia32_sys_call_table);
        memcpy(ia32_sys_call_table, ia32_sys_call_table_backup, syscall_nr_max);
        set_addr_ro(ia32_sys_call_table);
    }
}
