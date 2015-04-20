#include "userland.h"
#include "slrk.h"
#include "assert.h"
#include "test.h"
#include <asm/uaccess.h>
#include <linux/preempt.h>

static int cnt = 0;
static size_t(*orig_write)(int, const void *, size_t);
static char *argv[2] = {
   USER_ELF_PATH"syscall_tbl_64_user", NULL };

static noinline ssize_t my_write(int fd, const void *buf, size_t n)
{
    if (fd == 1 && !strncmp(buf, "{slrk: test_syscall_tbl_64}\n", n))
        ++cnt;
    return orig_write(fd, buf, n);
}

int syscall_tbl_64_run(void)
{
    if (syscall_tbl_fetch(x86_64))
        return 1;
    orig_write = (void *)syscall_tbl_orig(__NR_write, x86_64);
    syscall_tbl_set(__NR_write, my_write, x86_64);
    cnt = 0;
    user_land_exec(argv);
    assert(cnt != 0, "sys_call_table change entry");

    syscall_tbl_restore_all(x86_64);
    cnt = 0;
    user_land_exec(argv);
    assert(cnt == 0, "sys_call_table restored");

    return 0;
}

EXPORT_USER_ELF(syscall_tbl_64_user);

struct unit_test syscall_tbl_64_test = {
    .name = "syscall_tbl_64",
    .n = 2,
    .run = syscall_tbl_64_run,
    .elf = USER_ELF(syscall_tbl_64_user),
};

//test_init(syscall_tbl_64_test);
