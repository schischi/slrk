#ifndef ROOTKIT_HOOK_H
# define ROOTKIT_HOOK_H

# include <linux/types.h>

struct in_hook {
    uint8_t payload[32];
    uint8_t orig_code[32];
    size_t len;
    unsigned long fct;
    unsigned long hook;
};
enum payload_type { REL_JMP, PUSH_RET, PUSH_RET_32, RIP_REL, };

void inline_hook_init(unsigned long addr, unsigned long hook,
        enum payload_type t, struct in_hook *hk);
void inline_hook_enable(struct in_hook *);
void inline_hook_disable(struct in_hook *);

#endif /*! ROOTKIT_HOOK_H */
