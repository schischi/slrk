all: int_32 sysenter_32 syscall_32 syscall_64 vdso_32

int_32:
	nasm -f elf32 int_32.S
	ld -s -m elf_i386 int_32.o -o int_32

sysenter_32:
	nasm -f elf32 sysenter_32.S
	ld -s -m elf_i386 sysenter_32.o -o sysenter_32

syscall_32:
	nasm -f elf32 syscall_32.S
	ld -s -m elf_i386 syscall_32.o -o syscall_32

syscall_64:
	nasm -f elf64 syscall_64.S
	ld -s -m elf_x86_64 syscall_64.o -o syscall_64

vdso_32:
	nasm -f elf32 vdso_32.S
	ld -s -m elf_i386 vdso_32.o -o vdso_32

clean:
	rm -f *.o
	rm -f int_32 sysenter_32 syscall_32 syscall_64 vdso_32
