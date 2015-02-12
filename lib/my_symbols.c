{
    .name = "_text",
    .fct  = NULL,
    .asm_pattern = {
        .type      = E_UNUSED,
    },
    .default_value = 0xffffffff81000000,
},
{
    .name = "_etext",
    .fct  = NULL,
    .asm_pattern = {
        .type      = E_UNUSED,
    },
    .default_value = 0xffffffff815191aa, //FIXME
},
{
    .name = "sys_call_table",
        .fct  = NULL,
        .asm_pattern = {
            .type      = E_SAVE,
            .pattern   = {                          // <system_call_fastpath>:
                0x48, 0x3d, 0x3c, 0x01, 0x00, 0x00,         // cmp    $0x13c,%rax
                0x0f, 0x87, SKIP, SKIP, SKIP, SKIP,         // ja     257 <badsys>
                0x4c, 0x89, 0xd1,                           // mov    %r10,%rcx
                0xff, 0x14, 0xc5, SAVE, SAVE, SAVE, SAVE,   // callq  *0x0(,%rax,8)
                0x48, 0x89, 0x44, 0x24, 0x20,               // mov    %rax,0x20(%rsp)
                END,
            },
            .start     = DEFAULT,
            .end       = DEFAULT,
        },
},
{
    .name = "ia32_sys_call_table",
        .fct  = NULL,
        .asm_pattern = {
            .type      = E_SAVE,
            .pattern   = {                         // <sysenter_do_call>:
                0x41, 0x89, 0xf8,                          // mov    %edi,%r8d
                0x41, 0x89, 0xe9,                          // mov    %ebp,%r9d
                0x87, 0xce,                                // xchg   %ecx,%esi
                0x89, 0xdf,                                // mov    %ebx,%edi
                0x89, 0xd2,                                // mov    %edx,%edx
                0xff, 0x14, 0xc5, SAVE, SAVE, SAVE, SAVE,  // callq  *addr(,%rax,8)
                0x48, 0x89, 0x44, 0x24, 0x20,              // mov    %rax,0x20(%rsp)
                END,
            },
            .start     = DEFAULT,
            .end       = DEFAULT,
        },
},
{
    .name = "filldir",
    .fct  = NULL,
    .asm_pattern = {
        .type      = E_UNUSED,
    },
},
