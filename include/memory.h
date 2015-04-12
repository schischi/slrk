#ifndef ROOTKIT_MEMORY_H
# define ROOTKIT_MEMORY_H

# include <linux/types.h>

void set_addr_rw(void *addr);
void set_addr_ro(void *addr);

void disable_write_protect(void);
void enable_write_protect(void);

void *shadow_mapping(void *addr, size_t len, void **map_addr);
void del_shadow_mapping(void *addr);


#endif /* !ROOTKIT_MEMORY_H */
