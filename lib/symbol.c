#include "symbol.h"
#include "log.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>

struct asm_symbol symbols[] = {
#include "my_symbols.c"
};
const size_t nr_symbol = sizeof (symbols) / sizeof (struct asm_symbol);

static unsigned long text;
static unsigned long etext;

void init_symbols(void)
{
    text = symbol_retrieve("_text");
    etext = symbol_retrieve("_etext");
}

static struct asm_symbol *get_asm_symbol(const char *name)
{
    int i;

    for (i = 0; i < nr_symbol; ++i) {
        if (!strcmp(name, symbols[i].name))
            return &symbols[i];
    }
    return NULL;
}

static unsigned long pattern_search(struct asm_pattern *asm_pattern)
{
    unsigned long i, s = 0, p = 0;
    unsigned long save = 0;
    uint32_t *pattern = asm_pattern->pattern;
    unsigned long start = asm_pattern->start ? asm_pattern->start : text;
    unsigned long end = asm_pattern->end ? asm_pattern->start : etext;

    for (i = start; i < end; ++i) {
        uint8_t byte = *((uint8_t*)i);
        if (pattern[p] == SKIP || pattern[p] == SAVE || byte == pattern[p]) {
            if (pattern[p] == SAVE)
                save += byte << (8 * s++);
            ++p;
        }
        else {
            i -= p;
            p = 0;
        }
        if (pattern[p] == END)
            return asm_pattern->type == E_SAVE ? save : (i - p);
    }
    return 0;
}

unsigned long symbol_retrieve(const char *name)
{
    struct asm_symbol *symbol = get_asm_symbol(name);

    if (symbol == NULL)
        return 0;

    /* symbol has already be resolved */
    if (symbol->addr != 0)
        return symbol->addr;

    //TODO: System.map ? -> gen hash table from .map

    /* Try kallsyms */
    if ((symbol->addr = (unsigned long)kallsyms_lookup_name(name)))
        return symbol->addr;

    pr_log("kallsyms failed to get %s\n", name);

    /* Custom function */
    if (symbol->fct && (symbol->addr = symbol->fct(symbol)))
        return symbol->addr;

    pr_log("Custom fct failed to get %s\n", name);

    /* Pattern search */
    if (symbol->asm_pattern.type != E_UNUSED) {
        if (symbol->asm_pattern.setup_fct == NULL
            || symbol->asm_pattern.setup_fct(&symbol->asm_pattern))
            if ((symbol->addr = pattern_search(&symbol->asm_pattern)))
                return symbol->addr;
    }

    pr_log("Pattern search failed to get %s\n", name);

    /* Use the default value, if any */
    if (symbol->default_value)
        return symbol->addr = symbol->default_value;

    pr_log("Default value failed for %s\n", name);
    /* OK, I give up */
    return 0;
}
