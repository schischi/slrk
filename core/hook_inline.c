#include "hook_inline.h"
#include "memory.h"
#include "log.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

//http://www.ragestorm.net/blogs/?p=107
#define ADDR 0x00
#define OFFS 0x00

static void rel_jmp_cfg(unsigned long fct, unsigned long hook, char *payload)
{
    unsigned long offset = (hook - 5) - fct;
    //BUG_ON(offset < S32_MIN || offset > S32_MAX);
    *((int32_t *)(&payload[1])) = (int32_t)offset;
}

static void push_ret_cfg(unsigned long fct, unsigned long hook, char *payload)
{
    uint32_t high = (hook >> 32) & 0xffffffff;
    uint32_t low = hook & 0xffffffff;

    *((uint32_t *)(&payload[1])) = low;
    *((uint32_t *)(&payload[9])) = high;
}

static void push_ret_32_cfg(unsigned long fct, unsigned long hook, char *payload)
{
    uint32_t low = (uint32_t)hook;
    //BUG_ON(low  < 0x80000000 && (hook >> 32) != 0x00000000);
    //BUG_ON(low >= 0x80000000 && (hook >> 32) != 0xffffffff);
    *((uint32_t *)(&payload[1])) = low;
}

static void rip_rel_cfg(unsigned long fct, unsigned long hook, char *payload)
{
    *((unsigned long *)(&payload[6])) = hook;
}

struct inline_hook_payload {
    uint8_t payload[32];
    size_t len;
    void(*cfg_fct)(unsigned long, unsigned long, char *);
} payloads[] = {
    [REL_JMP] = {
        .payload = {
            0xe9, OFFS, OFFS, OFFS, OFFS,                   // jmpq ...
        },
        .len = 5,
        .cfg_fct = rel_jmp_cfg,
    },
    [PUSH_RET] = {
        .payload = {
            0x68, ADDR, ADDR, ADDR, ADDR,                   // pushq 0x01234567
            0xc7, 0x44, 0x24, 0x04, ADDR, ADDR, ADDR, ADDR, // movl  $0xffffffff,0x4(%rsp)
            0xc3,                                           // ret
        },
        .len = 14,
        .cfg_fct = push_ret_cfg,
    },
    /*
     * push take 32-bit value sign-extended to 64bits
     *  [0x0        - 0x7fffffff] (positive signed 32-bit) extended with 0
     *  [0x80000000 - 0xffffffff] (negative signed 32-bit) extended with f
     */
    [PUSH_RET_32] = {
        .payload = {
            0x68, ADDR, ADDR, ADDR, ADDR,                   // push 0x...
            0xc3,                                           // ret
        },
        .len = 6,
        .cfg_fct = push_ret_32_cfg,
    },
    [RIP_REL] = {
        .payload = {
            0xff, 0x25, 0x00, 0x00, 0x00, 0x00,             // jmpq *0x0(%rip)
            ADDR, ADDR, ADDR, ADDR, ADDR, ADDR, ADDR, ADDR, // 0x...
        },
        .len = 14,
        .cfg_fct = rip_rel_cfg,
    },
};

void inline_hook_init(unsigned long addr, unsigned long hook, enum payload_type t,
        struct in_hook *hk)
{
    hk->fct = addr;
    hk->hook = hook;
    hk->len = payloads[t].len;
    memcpy(hk->payload, payloads[t].payload, payloads[t].len);
    memcpy(hk->orig_code, (void*)addr, hk->len);
    payloads[t].cfg_fct(addr, hook, hk->payload);
}

void inline_hook_enable(struct in_hook *hk)
{
    slrk_write_read_only((void*)hk->fct, hk->payload, hk->len);
}

void inline_hook_disable(struct in_hook *hk)
{
    slrk_write_read_only((void*)hk->fct, hk->orig_code, hk->len);
}
