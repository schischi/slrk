#ifndef INT80_H
# define INT80_H

void int80_hook_enable(void);
void int80_set_hook(void *pre, void *post);
void int80_hook_disable(void);

#endif /* !INT80_H */
