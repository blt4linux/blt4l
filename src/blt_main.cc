/**
 * (C) 2016- Roman Hargrave
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

extern "C" {
#include <dlfcn.h>
}
#include <iostream>
#include <blt/hook.hh>

using std::cerr;

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
    cerr << "dlHandle = " << dlHandle << "\n";

    /*
     * Hack: test for presence of a known unique function amongst the libraries loaded by payday 
     * so that we don't try to init on some other image or some shit.
     *
     * lua_call will do just fine in this case, as it's only present in payday
     */
    if (dlHandle && dlsym(dlHandle, "_ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE")) {
        blt::blt_init_hooks(dlHandle);
    } else if(dlHandle) {
        cerr << "lua_call wasn't found in dlHandle, won't load!\n";
        dlclose(dlHandle);
    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
