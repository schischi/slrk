#include "userland.h"

#include <linux/module.h>
#include <linux/kmod.h>

int user_land_exec(char *argv[])
{
    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL
    };

    return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}

