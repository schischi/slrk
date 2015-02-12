#include "memory.h"
#include "symbol.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <asm/cacheflush.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

# define CR0_WP 0x00010000 

void disable_memory_write_protect(unsigned long addr)
{
    write_cr0(read_cr0() & ~CR0_WP);
}

void enable_memory_write_protect(unsigned long addr)
{
    write_cr0(read_cr0() | CR0_WP);
}

void set_addr_rw(void *_addr)
{
    pte_t *pte;
    unsigned int level;
    unsigned long addr = (unsigned long)_addr;

    pte = lookup_address(addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;
}

void set_addr_ro(void *_addr)
{
    pte_t *pte;
    unsigned int level;
    unsigned long addr = (unsigned long)_addr;

    pte = lookup_address(addr, &level);
    pte->pte = pte->pte &~_PAGE_RW;
}
