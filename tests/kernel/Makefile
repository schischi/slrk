obj-m := slrk_tests.o
slrk_tests-objs := ../../lib/lib_slrk.o  \
	module.o                             \
	idt.o                                \
	inline_hooking.o                     \
	syscall_tbl.o                        \
	sysenter.o                           \
	userland.o
EXTRA_CFLAGS := -I$(src)/../../include
