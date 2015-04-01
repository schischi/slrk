#ifndef ROOTKIT_ASSERT_H
# define ROOTKIT_ASSERT_H

# include "log.h"

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

#define assert(Cond, Name, ...)                   \
    pr_log("[%s]  "Name"\n",                      \
            Cond                                  \
              ? COLOR_GREEN"PASS"COLOR_RESET      \
              : COLOR_RED"FAIL"COLOR_RESET,       \
            ##__VA_ARGS__)

#endif /* !ROOTKIT_ASSERT_H */
