obj-m := slrk_tests.o
slrk_tests-objs := ../lib/lib_slrk.o  \
	module.o                             \
	int80.o                              \
	inline_hooking.o                     \
	syscall_tbl.o                        \
	sysenter.o                           \
	userland.o                           \
	test.o
	#idt.o
ccflags-y := -I$(src)/../include -DUSER_TESTS=\"$(src)/user/\"
ldflags-y := -T$(src)/tests.lds
