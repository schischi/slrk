#include "slrk.h"
#include "assert.h"

static struct in_hook foo_hk;
static volatile int cnt;

static noinline int foo(int a)
{
    ++cnt;
    return a + 1;
}

static noinline int bar(int a)
{
    int ret;

    ++cnt;
    inline_hook_disable(&foo_hk);
    ret = foo(a + 41);
    inline_hook_enable(&foo_hk);
    return ret;
}

static void test_inline_hook(const char *name)
{
    int ret;

    cnt = 0;
    inline_hook_enable(&foo_hk);
    ret = foo(0);
    assert(cnt == 2, "inline hook with %s is executed", name);
    assert(ret == 42, "inline hook with %s modified data", name);
    cnt = 0;
    inline_hook_disable(&foo_hk);
    ret = foo(0);
    assert(cnt == 1 && ret == 1, "inline hook with %s disabled", name);
}

void test_inline_hooking(void)
{
    inline_hook_init((unsigned long)foo, (unsigned long)bar, REL_JMP, &foo_hk);
    test_inline_hook("rel_jmp");
    inline_hook_init((unsigned long)foo, (unsigned long)bar, PUSH_RET_32, &foo_hk);
    test_inline_hook("push_ret_32");
    inline_hook_init((unsigned long)foo, (unsigned long)bar, PUSH_RET, &foo_hk);
    test_inline_hook("push_ret");
    inline_hook_init((unsigned long)foo, (unsigned long)bar, RIP_REL, &foo_hk);
    test_inline_hook("rip_rel");
}
