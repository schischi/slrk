#include "userland.h"
#include "slrk.h"
#include "assert.h"
#include "test.h"
#include <asm/uaccess.h>

static volatile const unsigned long var;

int memory_protection_run(void)
{
    assert(var == 0, "const var is set to 0");
#if 0
    memory_prot_bypass(MEM_PTE);
    set_addr_rw((void *)&var);
    *((int *)&var) = 13;
    set_addr_ro((void *)&var);
    assert(var == 13, "Modify const var (pte)");
#endif
#if 1
    memory_prot_bypass(MEM_CR);
    set_addr_rw((void *)&var);
    *((int *)&var) = 42;
    set_addr_ro((void *)&var);
    assert(var == 42, "Modify const var (cr0)");
#endif

    return 0;
}

struct unit_test memory_protection_test = {
    .name = "memory protection",
    .n = 2,
    .run = memory_protection_run,
};

//test_init(memory_protection_test);
