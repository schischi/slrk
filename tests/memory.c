#include "userland.h"
#include "slrk.h"
#include "assert.h"
#include "test.h"
#include <asm/uaccess.h>

static volatile const unsigned long var;

int memory_protection_run(void)
{
    pteval_t pte;
    void *shadow_map_addr;
    unsigned long *shadow_addr;

    assert(var == 0, "const var is set to 0");

    pte = set_page_rw((void *)&var);
    *((int *)&var) = 13;
    set_page_pte((void *)&var, pte);
    assert(var == 13, "Modify const var (pte)");

    enable_write_protect();
    *((int *)&var) = 42;
    disable_write_protect();
    assert(var == 42, "Modify const var (cr0)");

    shadow_addr = shadow_mapping((void *)&var, sizeof (unsigned long),
            &shadow_map_addr);
    *shadow_addr = 37;
    assert(var == 37, "Modify const var (shadow mapping)");
    del_shadow_mapping(shadow_map_addr);

    return 0;
}

struct unit_test memory_protection_test = {
    .name = "memory protection",
    .n = 3,
    .run = memory_protection_run,
};

test_init(memory_protection_test);
