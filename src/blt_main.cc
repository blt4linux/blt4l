/**
 * (C) 2016- Roman Hargrave
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>

#include <blt/hook.hh>

/*
 * Test for a GCC-compatible compiler, because we need 
 * the compiler to support the constructor attribute
 */

#if (!defined(__GNUC__))
#   warn GCC is required for this program to work. ignore this if your compiler supports the constructor attribute
#endif 

/*
 * This is run (several) times during pd2 startup. 
 * This implements a check to only run once.
 * It will try to use subhook to detour necessary functions 
 * and get a dl pointer to the process image
 * we (should) be running in the same process memory as payday2_release, but
 * the kernel VM does a good job of keeping your hands out of another library's shit,
 * unlike windows. This is problematic here since we need to get a linker handle
 * to the payday2_release process image, but can't necessarily do that.
 */
__attribute__((constructor))
static void
blt_main ()
{
    void* dlHandle = dlopen(NULL, RTLD_LAZY);
    fprintf(stderr, "dlHandle = %p\n", dlHandle);

    /*
     * Hack: test for presence of a known unique function amongst the libraries loaded by payday 
     * so that we don't try to init on some other image or some shit.
     *
     * lua_call will do just fine in this case, as it's only present in payday
     */
    if (dlHandle && dlsym(dlHandle, "lua_call")) {
        blt::InitLUAHooks(dlHandle);
    } else if(dlHandle) {
        dlclose(dlHandle);
    }
}

