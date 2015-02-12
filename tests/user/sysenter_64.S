section .text
global _start

_start:
    push  rbp ;
    mov   rbp, rsp ;
# Write the string to stdout
    mov   rdx, len ;
    mov   rcx, msg ;
    mov   rbx, 1 ;
    mov   rax, 1 ;
# Setting the stack for the systenter
    push  sysenter_ret ;
    push  rcx ;
    push  rdx ;
    push  rbp ;
    mov   rbp, rsp ;
    sysenter ;
# and exit
sysenter_ret:
    mov   rbx, 0 ;
    mov   rax, 60 ;
# Setting the stack for the systenter
    push  0x0 ;
    push  rcx ;
    push  rdx ;
    push  rbp ;
    mov   rbp, rsp ;
    sysenter ;

section .data

msg db 'Hello World!',0xa ;
len equ $ - msg ;
