#include "log.h"
#include "memory.h"
#include "symbol.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/semaphore.h>
#include <linux/preempt.h>
#include <linux/slab.h>

# define CR0_WP 0x00010000

static enum slrk_mem mem_method = MEM_CR0;

void disable_write_protect(void)
{
    write_cr0(read_cr0() & ~CR0_WP);
}

void enable_write_protect(void)
{
    write_cr0(read_cr0() | CR0_WP);
}

int *pte_set_rw(void *addr, size_t len)
{
    int i;
    int *saved_pte;
    pte_t *pte;
    unsigned int level;
    int nr_pages = DIV_ROUND_UP(offset_in_page(addr) + len, PAGE_SIZE);

    saved_pte = kmalloc(nr_pages * sizeof(int), GFP_KERNEL);
    for (i = 0; i < nr_pages; ++i) {
        pte = lookup_address((unsigned long)addr, &level);
        saved_pte[i] = pte->pte;
        if (pte->pte &~ _PAGE_RW)
            pte->pte |= _PAGE_RW;
        addr += PAGE_SIZE;
    }
    return saved_pte;
}

void pte_restore(void *addr, size_t len, int *saved_pte)
{
    int i;
    pte_t *pte;
    unsigned int level;
    int nr_pages = DIV_ROUND_UP(offset_in_page(addr) + len, PAGE_SIZE);

    for (i = 0; i < nr_pages; ++i) {
        pte = lookup_address((unsigned long)addr, &level);
        if (pte->pte & 0x2 && !(saved_pte[i] & 0x2))
            pte->pte &= ~0x2;
        else if (!(pte->pte & 0x2) && saved_pte[i] & 0x2)
            pte->pte |= 0x2;
        addr += PAGE_SIZE;
    }
    kfree(saved_pte);
}

void *shadow_mapping(void *addr, size_t len)
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
    kfree(pages);
    if (vaddr == NULL)
        return NULL;
    return vaddr + offset_in_page(addr);
}

void del_shadow_mapping(void *addr)
{
    vunmap((void *)((unsigned long)addr & PAGE_MASK));
}

void slrk_mem_method(enum slrk_mem m)
{
    mem_method = m;
}

void slrk_write_read_only(void *dst, void *src, size_t n)
{
    void *map;
    int *saved_pte;

    switch (mem_method) {
        case MEM_CR0:
            disable_write_protect();
            memcpy(dst, src, n);
            enable_write_protect();
            break;
        case MEM_PTE:
            saved_pte = pte_set_rw(dst, n);
            memcpy(dst, src, n);
            pte_restore(dst, n, saved_pte);
            break;
        case MEM_VMAP:
            map = shadow_mapping(dst, n);
            memcpy(dst, src, n);
            del_shadow_mapping(map);
            break;
    }
}

void slrk_write_ptr_read_only(void *addr, void *ptr)
{
    slrk_write_read_only(addr, &ptr, sizeof(void *));
}
