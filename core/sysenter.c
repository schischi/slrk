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

DEFINE_PER_CPU(char[SYSENTER_STACK_SZ], stack_sysenter);

static unsigned long sysenter_orig_hdlr;
static unsigned long sysenter_orig_hdlr_no_swapgs;
static unsigned long sysenter_pre_hook;
static unsigned long sysenter_post_hook;

/* Stack frame:
 *   - ebp
 *   - edx
 *   - ecx
 *   - eip
 */
extern asmlinkage void sysenter_fake_dispatcher(void);
asm (
    ".globl sysenter_fake_dispatcher\n"
    ".align 8, 0x90\n"
    "sysenter_fake_dispatcher:\n"
    /* Obtain a valid pointer to per cpu data*/
    "swapgs\n"
    /* Setup a stack */
    "mov $stack_sysenter + ("SYSENTER_STACK_TOP"), %rsp\n"
    "add %gs:this_cpu_off, %rsp\n"
    /* Save registers on the stack */
    "sub $0x30, %rsp\n"  /* Skip exception frame */
    SAVE_REGS
    /* Fill exception frame */
    "movl 12(%rbp), %eax\n"     /* RIP */
    "movq %rax, 0x88(%rsp)\n"
    "movq $0x23, 0x90(%rsp)\n"  /* CS */
    "movq $0x0, 0x98(%rsp)\n"   /* RFLAGS */
    "movl 0x0(%rbp), %eax\n"    /* RSP */
    "movq %rax, 0xa0(%rsp)\n"
    "movq $0x2b, 0xa8(%rsp)\n"  /* SS */
    /* Set an invalid esp */
    "movl $"__stringify(SYSENTER_MAGIC)", 12(%rbp)\n"
    /* slrk_regs */
    "mov %rsp, %rdi\n"
    /* Pre-hook ! */
    "call *sysenter_pre_hook\n"
    RESTORE_REGS
    /* Call the original handler without swapgs */
    "jmp *sysenter_orig_hdlr_no_swapgs\n"
);

extern asmlinkage void sysenter_fake_ret(void);
asm (
    ".globl sysenter_fake_ret\n"
    ".align 8, 0x90\n"
    "sysenter_fake_ret:\n"
    "swapgs\n"
    SAVE_REGS
    "mov $stack_sysenter + ("SYSENTER_STACK_TOP"), %rdi\n"
    "add %gs:this_cpu_off, %rdi\n"
    "sub $0x88, %rdi\n"
    "mov %rsp, %rsi\n"
    "call *sysenter_post_hook\n"
    RESTORE_REGS
    "mov $stack_sysenter + ("SYSENTER_STACK_TOP"), %rsp\n"
    "add %gs:this_cpu_off, %rsp\n"
    "swapgs\n"
    "sub $0x28, %rsp\n"
    "iretq\n"
);

noinline int fake_pf_hdlr(struct slrk_regs *regs, int err)
{
    //TODO check error code
    asm volatile (
        "push %rax\n"
        "mov %cr2, %rax\n"
        "cmp $"__stringify(SYSENTER_MAGIC)", %rax\n"
        "pop %rax\n"
        "jne out\n"
        "jmp sysenter_fake_ret\n"
        "out:\n"
    );
    return IRET_ORIG;
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
    idt_set_hook(0xE, fake_pf_hdlr, NULL);
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
