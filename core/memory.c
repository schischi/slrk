#include "memory.h"
#include "symbol.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <asm/cacheflush.h>
#include <linux/semaphore.h>
#include <linux/preempt.h>
#include <linux/slab.h>

# define CR0_WP 0x00010000 

static void disable_wp(unsigned long addr)
{
    write_cr0(read_cr0() & ~CR0_WP);
}

static void enable_wp(unsigned long addr)
{
    write_cr0(read_cr0() | CR0_WP);
}

static void pte_set_rw(unsigned long addr)
{
    pte_t *pte;
    unsigned int level;

    pte = lookup_address(addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;
}

static void pte_set_ro(unsigned long addr)
{
    pte_t *pte;
    unsigned int level;

    pte = lookup_address(addr, &level);
    pte->pte = pte->pte &~_PAGE_RW;
}

#if 0
static void (*mem_rw)(unsigned long) = pte_set_rw;
static void (*mem_restore)(unsigned long) = pte_set_ro;
#else
static void (*mem_rw)(unsigned long) = disable_wp;
static void (*mem_restore)(unsigned long) = enable_wp;
#endif

void memory_prot_bypass(enum memory_prot_bypass_method m)
{
    if (m == MEM_CR) {
        mem_rw = disable_wp;
        mem_restore = enable_wp;
    }
    else if (m == MEM_PTE) {
        mem_rw = pte_set_rw;
        mem_restore = pte_set_ro;
    }
}

void set_addr_rw(void *addr)
{
    preempt_disable();
    barrier();
    mem_rw((unsigned long)addr);
}

void set_addr_ro(void *addr)
{
    mem_restore((unsigned long)addr);
    barrier();
    preempt_enable();
}
