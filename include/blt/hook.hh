#ifndef _HOOK_H
#define _HOOK_H
#pragma once

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

    class lua_state;
    typedef const char* (*lua_reader) (lua_state*, void*, size_t*);
    typedef int (*lua_cfunction) (lua_state*);
    typedef void* (*lua_alloc) (void*, void*, size_t, size_t);
    typedef struct {
        const char* name;
        lua_cfunction func;
    } luaL_reg;

    /*
     * Forward-declarations of our hooked functions
     */

extern "C" {
    void        hlua_call         (lua_state*, int, int);
    int         hlua_pcall        (lua_state*, int, int, int);
    int         hlua_gettop       (lua_state*);
    void        hlua_settop       (lua_state*, int);
    const char* hlua_tolstring    (lua_state*, int, size_t*);
    int         hluaL_loadfile    (lua_state*, const char*);
    int         hlua_load         (lua_state*, lua_reader*, void*, const char*);
    void        hlua_setfield     (lua_state*, int, const char*);
    void        hlua_createtable  (lua_state*, int, int);
    void        hlua_insert       (lua_state*, int);
    lua_state*  hlua_newstate     (lua_alloc, void*);
    void        hlua_close        (lua_state*);
    void        hlua_rawset       (lua_state*, int);
    void        hlua_settable     (lua_state*, int);
    void        hlua_pushnumber   (lua_state*, double);
    void        hlua_pushinteger  (lua_state*, ptrdiff_t);
    void        hlua_pushboolean  (lua_state*, bool);
    void        hlua_pushcclosure (lua_state*, lua_cfunction, int);
    void        hlua_pushlstring  (lua_state*, const char*, size_t);
    void        hluaI_openlib     (lua_state*, const char*, const luaL_reg*, int);
    void        hluaL_ref         (lua_state*, int);
    void        hluaL_openlib     (lua_state*, const char*, const luaL_reg*, int);
    void        hlua_rawgeti      (lua_state*, int, int);
    void        hluaL_unref       (lua_state*, int, int);
    int         hluaL_newstate    (char, char, int);
}
    void        dslUpdateDetour   ();

    /*
     * Internal
     */

    void InitLUAHooks(void*);

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

#endif //_HOOK_H
