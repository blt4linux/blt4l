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


    extern   void        (*lua_call)         (lua_state*, int, int);
    extern   int         (*lua_pcall)        (lua_state*, int, int, int);
    extern   int         (*lua_gettop)       (lua_state*);
    extern   void        (*lua_settop)       (lua_state*, int);
    extern   const char* (*lua_tolstring)    (lua_state*, int, size_t*);
    extern   int         (*luaL_loadfile)    (lua_state*, const char*);
    extern   int         (*lua_load)         (lua_state*, lua_reader*, void*, const char*);
    extern   void        (*lua_setfield)     (lua_state*, int, const char*);
    extern   void        (*lua_createtable)  (lua_state*, int, int);
    extern   void        (*lua_insert)       (lua_state*, int);
    extern   lua_state*  (*lua_newstate)     (lua_alloc, void*);
    extern   void        (*lua_close)        (lua_state*);
    extern   void        (*lua_rawset)       (lua_state*, int);
    extern   void        (*lua_settable)     (lua_state*, int);
    extern   void        (*lua_pushnumber)   (lua_state*, double);
    extern   void        (*lua_pushinteger)  (lua_state*, ptrdiff_t);
    extern   void        (*lua_pushboolean)  (lua_state*, bool);
    extern   void        (*lua_pushcclosure) (lua_state*, lua_cfunction, int);
    extern   void        (*lua_pushlstring)  (lua_state*, const char*, size_t);
    extern   void        (*luaI_openlib)     (lua_state*, const char*, const luaL_reg*, int);
    extern   int         (*luaL_ref)         (lua_state*, int);
    extern   void        (*luaL_openlib)     (lua_state*, const char*, const luaL_reg*, int);
    extern   void        (*lua_rawgeti)      (lua_state*, int, int);
    extern   void        (*luaL_unref)       (lua_state*, int, int);
    extern   int         (*luaL_newstate)    (char, char, int);


    bool check_active_state(lua_state*);

    void blt_init_hooks(void*);

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

#endif //_HOOK_H
