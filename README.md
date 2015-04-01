slrk - schischi Linux RootKit
=============================

The aim of this project is to better understand how Linux works by writing
a rootkit.

This is *not* a functional rootkit, I implemented many different ways of
manipulating the kernel (inline hooking, debug registers, IDT/sysenter/syscall
hooking, pointer subterfuge…) inside a library.
This library is then used to run some tests.

Requirements
------------
The project has been tested with the latest x86_64 version of the Linux
kernel (v4.0-rc4).

Running the rootkit
-------------------
``make  # build the library slrk.a``
``make tests  # build the test module with userland tests``
``insmod ./tests/slrk_tests.ko``
``rmmod slrk_tests``

