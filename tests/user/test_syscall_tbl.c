#include <unistd.h>

#if defined(__i386__)
    const char str[] = "{schischi: test_syscall_tbl_32}\n";
#elif defined(__x86_64__)
    const char str[] = "{schischi: test_syscall_tbl_64}\n";
#else
    #error x86 only
#endif

int main() {
    write(1, str, sizeof (str));
    return 0;
}
