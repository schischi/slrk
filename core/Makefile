obj-m := lib_slrk.o
lib_slrk-objs := syscall_tbl.o   \
	memory.o                     \
	hook_inline.o                \
	symbol.o                     \
	idt.o                        \
	idt_hdlrs.o                  \
	sysenter.o                   \
	syscall_nr.o
EXTRA_CFLAGS := -I$(src)/../include

$(obj)/syscall_nr.c:
	$(src)/../scripts/gen_syscall_tbl.sh $(shell pwd) $(obj)/syscall_nr.c

