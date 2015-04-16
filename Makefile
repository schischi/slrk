KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
CORE_SLRK := $(PWD)/core
KTESTS := $(PWD)/tests/
UTESTS := $(PWD)/tests/user

slrk:
	$(MAKE) -C $(KDIR) SUBDIRS=$(CORE_SLRK)

slrk_clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(CORE_SLRK) clean
	rm -f core/syscall_nr.c

tests: tests_kernel

tests_kernel: slrk tests_user
	$(MAKE) -C $(KDIR) M=$(KTESTS)

tests_kernel_clean:
	$(MAKE) -C $(KDIR) M=$(KTESTS) clean

tests_user:
	$(MAKE) -C $(UTESTS)

tests_user_clean:
	$(MAKE) -C $(UTESTS) clean

clean: tests_user_clean tests_kernel_clean slrk_clean

.PHONY: core
.PHONY: tests
