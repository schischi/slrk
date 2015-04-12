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

void disable_write_protect(void)
{
    write_cr0(read_cr0() & ~CR0_WP);
}

void enable_write_protect(void)
{
    write_cr0(read_cr0() | CR0_WP);
}

void set_addr_rw(void *addr)
{
    pte_t *pte;
    unsigned int level;

    pte = lookup_address((unsigned long)addr, &level);
    if (pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;
}

void set_addr_ro(void *addr)
{
    pte_t *pte;
    unsigned int level;

    pte = lookup_address((unsigned long)addr, &level);
    pte->pte = pte->pte &~_PAGE_RW;
}

static void *foo;
void *shadow_mapping(void *addr, size_t len, void **map_addr)
{
    void *vaddr;
    int nr_pages = DIV_ROUND_UP(offset_in_page(addr) + len, PAGE_SIZE);
    struct page **pages = kmalloc(nr_pages * sizeof(*pages), GFP_KERNEL);
    void *page_addr = (void *)((unsigned long)addr & PAGE_MASK);
    int i;

    if (pages == NULL)
        return NULL;

    for (i = 0; i < nr_pages; i++) {
        if (__module_address((unsigned long)page_addr) == NULL) {
            pages[i] = virt_to_page(page_addr);
            WARN_ON(!PageReserved(pages[i]));
        } else {
            pages[i] = vmalloc_to_page(page_addr);
        }
        if (pages[i] == NULL) {
            kfree(pages);
            return NULL;
        }
        page_addr += PAGE_SIZE;
    }
    vaddr = vmap(pages, nr_pages, VM_MAP, PAGE_KERNEL);
    *map_addr = vaddr;
    kfree(pages);
    if (vaddr == NULL)
        return NULL;
    return vaddr + offset_in_page(addr);
}

void del_shadow_mapping(void *addr)
{
    vunmap(foo);
}

