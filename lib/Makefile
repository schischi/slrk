obj-m := lib_slrk.o
lib_slrk-objs := syscall_tbl.o   \
	memory.o                     \
	hook_inline.o                \
	symbol.o                     \
	idt.o                        \
	sysenter.o
EXTRA_CFLAGS := -I$(src)/../include
