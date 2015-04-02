obj-m := slrk_tests.o
slrk_tests-objs := ../core/lib_slrk.o    \
	module.o                             \
	inline_hooking.o                     \
	syscall_tbl.o                        \
	sysenter.o                           \
	userland.o                           \
	test.o                               \
	idt.o                                \
	int80.o
ccflags-y := -I$(src)/../include -DUSER_TESTS=\"$(src)/user/\"
ldflags-y := -T$(src)/tests.lds
