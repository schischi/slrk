#include "log.h"
#include "memory.h"
#include "idt.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

extern asmlinkage unsigned long idt_fake_hdlrs[];
extern asmlinkage unsigned idt_hook_sz;

static noinline int pre_do_nothing(void) { return -1; }
static noinline void post_do_nothing(void) { }
#define IDT_SZ (sizeof (struct gate_struct64) * 256)

static size_t idt_size;
static struct gate_struct64 *old_idt_table;
static struct gate_struct64 new_idt_table[IDT_SZ];
static struct gate_struct64 *cur_idt_table;
struct idt_hook {
    unsigned long pre;
    unsigned long orig;
    unsigned long post;
    unsigned long hdlr;
} idt_hook[256] = {
    [0 ... 255] = {
        .pre = (unsigned long)pre_do_nothing,
        .post = (unsigned long)post_do_nothing,
    },
};

static unsigned long idt_get_entry(int n)
{
    unsigned long addr = (((unsigned long)cur_idt_table[n].offset_high) << 32)
         + (((unsigned long)cur_idt_table[n].offset_middle) << 16)
         + (((unsigned long)cur_idt_table[n].offset_low));
    return addr;
}

static void idt_set_entry(unsigned long addr, int n)
{
    if (cur_idt_table == NULL)
        idt_substitute(IDT_TABLE);

    cur_idt_table[n].offset_high = (addr >> 32) & 0xffffffff;
    cur_idt_table[n].offset_middle = (addr >> 16) & 0xffff;
    cur_idt_table[n].offset_low = addr & 0xffff;
}

void idt_set_hook(int n, void *pre, void *post)
{
    idt_hook[n].pre = pre ? (unsigned long)pre : (unsigned long)pre_do_nothing;
    idt_hook[n].post = post ? (unsigned long)post : (unsigned long)post_do_nothing;
    idt_hook[n].hdlr = (unsigned long)idt_fake_hdlrs + n * idt_hook_sz;
}

void idt_hook_enable(int entry)
{
    idt_set_entry(idt_hook[entry].hdlr, entry);
}

void idt_hook_disable(int entry)
{
    idt_set_entry(idt_hook[entry].orig, entry);
}

static void inline local_store_idt(void *dtr)
{
    asm volatile("sidt %0":"=m" (*((struct desc_ptr *)dtr)));
}

static void inline local_load_idt(void *dtr)
{
    asm volatile("lidt %0"::"m" (*((struct desc_ptr *)dtr)));
}

void idt_substitute(enum idt_hijack_method m)
{
    int i;
    struct desc_ptr idtr;

    if (cur_idt_table && cur_idt_table == new_idt_table)
        return;

    if (m & IDT_TABLE) {
        local_store_idt(&idtr);
        old_idt_table = (struct gate_struct64 *)idtr.address;
        idt_size = idtr.size;

        cur_idt_table = old_idt_table;
        for (i = 0; i < 256; ++i)
            idt_hook[i].orig = idt_get_entry(i);

        memcpy(new_idt_table, old_idt_table, IDT_SZ);
        idtr.address = (unsigned long)new_idt_table;
        idtr.size = idt_size;
        on_each_cpu(local_load_idt, &idtr, 1);
        cur_idt_table = new_idt_table;
    }
    else if (m & IDT_ENTRY) {
        //TODO
    }
}

void idt_restore(void)
{
    struct desc_ptr idtr;

    if (!old_idt_table)
        return;
    idtr.address = (unsigned long)old_idt_table;
    idtr.size = idt_size;
    on_each_cpu(local_load_idt, &idtr, 1);
    //cur_idt_table = old_idt_table;
    cur_idt_table = NULL;
}
