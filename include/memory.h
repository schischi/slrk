#ifndef ROOTKIT_MEMORY_H
# define ROOTKIT_MEMORY_H

void disable_memory_write_protect(unsigned long addr);
void enable_memory_write_protect(unsigned long addr);
void set_addr_rw(void *addr);
void set_addr_ro(void *addr);

#endif /* !ROOTKIT_MEMORY_H */
