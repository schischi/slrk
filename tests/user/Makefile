all: int80 sysenter_32 syscall_32 syscall_64 vdso_32 \
	syscall_tbl_32 syscall_tbl_64

int80:
	nasm -f elf32 int80.S
	ld -s -m elf_i386 int80.o -o int80_user

sysenter_32:
	nasm -f elf32 sysenter_32.S
	ld -s -m elf_i386 sysenter_32.o -o sysenter_32_user

syscall_32:
	nasm -f elf32 syscall_32.S
	ld -s -m elf_i386 syscall_32.o -o syscall_32_user

syscall_64:
	nasm -f elf64 syscall_64.S
	ld -s -m elf_x86_64 syscall_64.o -o syscall_64_user

vdso_32:
	nasm -f elf32 vdso_32.S
	ld -s -m elf_i386 vdso_32.o -o vdso_32_user

syscall_tbl_32:
	nasm -f elf32 syscall_tbl_32.S
	ld -s -m elf_i386 syscall_tbl_32.o -o syscall_tbl_32_user

syscall_tbl_64:
	gcc syscall_tbl.c -o syscall_tbl_64_user

clean:
	rm -f *.o
	rm -f int80_user sysenter_32_user syscall_32_user syscall_64_user \
		vdso_32_user syscall_tbl_32_user syscall_tbl_64_user
