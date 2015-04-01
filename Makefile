KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
LIB_SLRK := $(PWD)/lib
KTESTS := $(PWD)/tests/
UTESTS := $(PWD)/tests/user

slrk:
	$(MAKE) -C $(KDIR) SUBDIRS=$(LIB_SLRK)

slrk_clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(LIB_SLRK) clean

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

.PHONY: lib
.PHONY: tests
