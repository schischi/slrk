section .text
global _start

_start:
    push  ebp ;
    mov   ebp, esp ;
# Write the string to stdout
    mov   edx, len ;
    mov   ecx, msg ;
    mov   ebx, 1 ;
    mov   eax, 4 ;
# Setting the stack for the sysenter
    push  sysenter_ret ;
    push  ecx ;
    push  edx ;
    push  ebp ;
    mov   ebp, esp ;
    sysenter ;
# and exit
sysenter_ret:
    mov   ebx, 0 ;
    mov   eax, 1 ;
# Setting the stack for the sysenter
    push  0x0 ;
    push  ecx ;
    push  edx ;
    push  ebp ;
    mov   ebp, esp ;
    sysenter ;

section .data

msg db 'Hello World!',0xa ;
len equ $ - msg ;
