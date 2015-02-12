section .text
global _start
_start:
    mov eax, 1   ; use the write syscall
    mov edi, 1   ; write to stdout
    mov esi, msg ; use string "Hello World"
    mov edx, len  ; write 12 characters
    syscall         ; make syscall

    mov eax, 60  ; use the exit syscall
    mov edi, 0   ; error code 0
    syscall         ; make syscall

section .data
	msg db 'Hello World!',0xa ;
	len equ $ - msg ;
