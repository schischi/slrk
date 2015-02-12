#include "log.h"
#include <linux/module.h>
#include <linux/kernel.h>

#include "regs.h"
#include "idt.h"
#include "sysenter.h"
#include <asm/percpu.h>

# define SYSENTER_STACK_SZ 512
# define SYSENTER_STACK_TOP "511"
# define SYSENTER_MAGIC 0x42cafe42

idt_create_pre_hook(0xE);
DEFINE_PER_CPU(char[SYSENTER_STACK_SZ], stack_sysenter);

static unsigned long sysenter_orig_hdlr;
static unsigned long sysenter_orig_hdlr_no_swapgs;
static unsigned long sysenter_pre_hook;
static unsigned long sysenter_post_hook;

/* race if schedulable ?*/

/* Stack frame:
 *   - ebp
 *   - edx
 *   - ecx
 *   - eip
 */
extern asmlinkage void sysenter_fake_dispatcher(void);
extern asmlinkage void sysenter_fake_ret(void);
void __unused_sysenter_fake_dispatcher(void)
{
    asm volatile (
        ".globl sysenter_fake_dispatcher\n"
        ".align 8, 0x90\n"
        "sysenter_fake_dispatcher:\n"
        /* Obtain a valid pointer to per cpu data*/
        "swapgs\n"
        /* Setup a stack */
        "mov $stack_sysenter + ("SYSENTER_STACK_TOP"), %rsp\n"
        "add %gs:this_cpu_off, %rsp\n"
        /* Save registers on the stack */
        "sub $0x28, %rsp\n"  /* Skip exception frame */
        SAVE_REGS2
        /* Fill exception frame */
        "movl 12(%rbp), %eax\n"     /* RIP */
        "movq %rax, 0x80(%rsp)\n"
        "movq $0x23, 0x88(%rsp)\n"  /* CS */
        "movq $0x0, 0x90(%rsp)\n"   /* RFLAGS */
        "movl 0x0(%rbp), %eax\n"    /* RSP */
        "movq %rax, 0x98(%rsp)\n"
        "movq $0x2b, 0xa0(%rsp)\n"  /* SS */
        /* Set an invalid esp */
        "movl $"__stringify(SYSENTER_MAGIC)", 12(%rbp)\n"
        /* pt_regs */
        "mov %rsp, %rdi\n"
        /* Pre-hook ! */
        "call *sysenter_pre_hook\n"
        RESTORE_REGS2
        /* Call the original handler without swapgs */
        "jmp *sysenter_orig_hdlr_no_swapgs\n"
    );

    asm volatile (
        ".globl sysenter_fake_ret\n"
        ".align 8, 0x90\n"
        "sysenter_fake_ret:\n"
        "swapgs\n"
        SAVE_REGS2
        "mov $stack_sysenter + ("SYSENTER_STACK_TOP"), %rdi\n"
        "add %gs:this_cpu_off, %rdi\n"
        "sub $0x80, %rdi\n"
        "mov %rsp, %rsi\n"
        "call *sysenter_post_hook\n"
        RESTORE_REGS2
        "mov $stack_sysenter + ("SYSENTER_STACK_TOP"), %rsp\n"
        "add %gs:this_cpu_off, %rsp\n"
        "swapgs\n"
        "sub $0x28, %rsp\n"
        "iretq\n"
    );
}

noinline void fake_pf_hdlr(struct pt_regs *regs)
{
    asm volatile (
        "push %rax\n"
        "mov %cr2, %rax\n"
        "cmp $"__stringify(SYSENTER_MAGIC)", %rax\n"
        "jne out\n"
        "pop %rax\n"
        "jmp sysenter_fake_ret\n"
        "out:\n"
        "pop %rax\n"
    );
}

static void local_wrmsrl_sysenter_eip(void *data)
{
    wrmsrl(MSR_IA32_SYSENTER_EIP, (unsigned long)data);
}

void sysenter_hook_cfg(void *pre, void *post)
{
    sysenter_pre_hook = (unsigned long)pre;
    sysenter_post_hook = (unsigned long)post;
}

void sysenter_hook_enable(void)
{
    /* backup the original handler address */
    if (sysenter_orig_hdlr == 0) {
        unsigned long eip;
        rdmsrl(MSR_IA32_SYSENTER_EIP, eip);
        sysenter_orig_hdlr = eip;
        /* We need to skip the swapgs instruction */
        BUG_ON((*((unsigned long *)eip) & 0xffffff) != 0xf8010f);
        sysenter_orig_hdlr_no_swapgs = eip + 3;
    }
    idt_hook_cfg(0xE, (unsigned long)fake_pf_hdlr, 0);
    idt_hook_enable(0xE);
    on_each_cpu(local_wrmsrl_sysenter_eip, (void*)sysenter_fake_dispatcher, 1);
}

void sysenter_hook_disable(void)
{
    if (sysenter_orig_hdlr == 0)
        return;
    on_each_cpu(local_wrmsrl_sysenter_eip, (void*)(sysenter_orig_hdlr), 1);
    sysenter_orig_hdlr = 0;
    idt_hook_disable(0xE);
}
