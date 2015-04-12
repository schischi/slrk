#ifndef ROOTKIT_IDT_H
# define ROOTKIT_IDT_H

enum idt_hijack_method {
    IDT_TABLE = 1,
    IDT_ENTRY = 2,
};

void idt_substitute(enum idt_hijack_method m);
void idt_restore(void);

void idt_set_hook(int n, void *pre, void *post);
void idt_hook_enable(int entry);
void idt_hook_disable(int entry);

#endif /* !ROOTKIT_IDT_H */
