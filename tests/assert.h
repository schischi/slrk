#ifndef ROOTKIT_ASSERT_H
# define ROOTKIT_ASSERT_H

# include "log.h"

#define assert(Cond, Name, ...)                   \
    pr_log("[%s]  "Name"\n",                      \
            Cond                                  \
              ? COLOR_GREEN"PASS"COLOR_RESET      \
              : COLOR_RED"FAIL"COLOR_RESET,       \
            ##__VA_ARGS__)

#endif /* !ROOTKIT_ASSERT_H */
