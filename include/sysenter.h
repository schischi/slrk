#ifndef ROOTKIT_SYSENTER_H
# define ROOTKIT_SYSENTER_H

void sysenter_hook_cfg(void *pre, void *post);
void sysenter_hook_enable(void);
void sysenter_hook_disable(void);

#endif /* !ROOTKIT_SYSENTER_H */
