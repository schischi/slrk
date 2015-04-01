#ifndef SLRK_TEST_H
# define SLRK_TEST_H

# include <linux/list.h>

# define USER_TESTS "/home/schischi/dev/slrk/tests/user/"
# define USER_ELF_PATH "/sys/kernel/debug/slrk/"

# define EXPORT_USER_ELF(Name)                   \
asm (                                            \
    ".section .userland_elf, \"ax\",@progbits\n" \
    ".global "#Name"\n"                          \
    ".type "#Name", @object\n"                   \
    #Name":\n"                                   \
    ".incbin \""USER_TESTS#Name"\"\n"            \
    ".global "#Name"_size\n"                     \
    ".type "#Name"_size, @object\n"              \
    ".align 4\n"                                 \
    #Name"_size:\n"                              \
    ".int "#Name"_size - " #Name"\n"             \
);                                               \
extern char *Name[];                             \
extern const size_t Name##_size

# define USER_ELF(Name)                 \
    {  .ptr = Name,                     \
       .name = #Name,                   \
       .size = (int *)&Name##_size  }

struct user_elf {
    void *ptr;
    int *size;
    const char *name;
    struct dentry *file;
};

struct unit_test {
    int n;
    char *name;
    int (*run)(void);
    struct user_elf elf;
    struct list_head list;
};

void init_unit_tests(void);
void run_unit_tests(void);
void cleanup_unit_tests(void);

#define test_init(fn)                          \
    asm (                                      \
        ".section ._tests, \"a\" ,@progbits\n" \
        ".quad "#fn"\n"                        \
    )

#endif /* !SLRK_TEST_H */
