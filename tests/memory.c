#include "userland.h"
#include "slrk.h"
#include "assert.h"
#include "test.h"
#include <asm/uaccess.h>

static volatile const unsigned long var;
static volatile const unsigned long const big_array[4096];

int memory_protection_run(void)
{
    int *saved_pte;
    unsigned long *shadow_addr;

    assert(var == 0, "const var is set to 0");

    saved_pte = pte_set_rw((void *)&var, sizeof(unsigned long));
    *((int *)&var) = 13;
    pte_restore((void *)&var, sizeof(unsigned long), saved_pte);
    assert(var == 13, "Modify const var (pte)");

    saved_pte = pte_set_rw((void *)big_array, sizeof(big_array));
    ((unsigned long *)big_array)[0] = 13;
    ((unsigned long *)big_array)[4095] = 13;
    pte_restore((void *)big_array, sizeof(big_array), saved_pte);
    assert(big_array[0] == 13 && big_array[4095] == 13,
            "Modify const var, multiple pages(pte)");

    enable_write_protect();
    *((int *)&var) = 42;
    disable_write_protect();
    assert(var == 42, "Modify const var (cr0)");

    shadow_addr = shadow_mapping((void *)&var, sizeof (unsigned long));
    *shadow_addr = 37;
    assert(var == 37, "Modify const var (shadow mapping)");
    del_shadow_mapping(shadow_addr);

    return 0;
}

struct unit_test memory_protection_test = {
    .name = "memory protection",
    .n = 3,
    .run = memory_protection_run,
};

test_init(memory_protection_test);
