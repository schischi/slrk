section     .text
global      _start
_start:
    mov     edx, len
    mov     ecx, msg
    mov     ebx, 1
    mov     eax, 4
    call    [gs:0x10]
    mov     eax, 1
    call    [gs:0x10]

section     .data
    msg     db  'Hello schischi',0xa
    len     equ $ - msg
