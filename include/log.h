#ifndef ROOTKIT_LOG_H
# define ROOTKIT_LOG_H

# define pr_fmt(fmt) "schischi: " fmt
# define DEBUG

# include <linux/kernel.h>
# include <linux/module.h>

# ifdef DEBUG
#  define pr_log(fmt, ...) pr_info(fmt, ##__VA_ARGS__);
# else
#  define pr_log(fmt, ...) do { } while (0);
# endif

#endif /* !ROOTKIT_LOG_H */
