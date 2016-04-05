#ifndef _HOOK_H
#define _HOOK_H
#pragma once

#include <blt/lapi.hh>
#include <cstdlib>
#include <cstddef>

/**
 * BLT4L Linux Modding Platform for PayDay2
 * (C) 2016- Roman Hargrave 
 * 
 * Unit: Hook interface declarations
 *
 * NOTE: I usually program in C, not C++
 * NOTE: Please see multiplat_detours_ex.cc for detour documention
 */

namespace blt {


    /*
     * Internal
     */

    void InitLUAHooks(void*);

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

#endif //_HOOK_H
