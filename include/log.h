#ifndef ROOTKIT_LOG_H
# define ROOTKIT_LOG_H

# define pr_fmt(fmt) "schischi: " fmt
# define DEBUG

# include <linux/kernel.h>
# include <linux/module.h>

# define COLOR_RED     "\x1b[31m"
# define COLOR_GREEN   "\x1b[32m"
# define COLOR_YELLOW  "\x1b[33m"
# define COLOR_BLUE    "\x1b[34m"
# define COLOR_MAGENTA "\x1b[35m"
# define COLOR_CYAN    "\x1b[36m"
# define COLOR_RESET   "\x1b[0m"

# ifdef DEBUG
#  define pr_log(fmt, ...) pr_info(fmt, ##__VA_ARGS__);
# else
#  define pr_log(fmt, ...) do { } while (0);
# endif

#endif /* !ROOTKIT_LOG_H */
