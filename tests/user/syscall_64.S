section .text
global _start
_start:
    mov rax, 1      ; use the write syscall
    mov rdi, 1      ; write to stdout
    mov rsi, msg    ; use string "Hello World"
    mov rdx, len    ; write 12 characters
    syscall         ; make syscall

    mov rax, 60     ; use the _exit syscall
    mov rdi, 0      ; error code 0
    syscall         ; make syscall

section .data
	msg db 'Hello World!',0xa ;
	len equ $ - msg ;
