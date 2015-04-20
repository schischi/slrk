#ifndef ROOTKIT_MEMORY_H
# define ROOTKIT_MEMORY_H

# include <linux/types.h>
# include <asm/cacheflush.h>

pteval_t set_page_rw(void *addr);
void set_page_pte(void *addr, pteval_t p);

void disable_write_protect(void);
void enable_write_protect(void);

void *shadow_mapping(void *addr, size_t len, void **map_addr);
void del_shadow_mapping(void *addr);


#endif /* !ROOTKIT_MEMORY_H */
