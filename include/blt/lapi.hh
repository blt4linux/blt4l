/**
 * LUA extension and override signatures
 */

#include <cstddef>

#pragma once

namespace blt {

    static int const LUARegistryIndex   = -10000;
    static int const LUAGlobalsIndex    = -10002;
    static int const LUAYield           = 1;
    static int const LUAErrRun          = 2;
    static int const LUAErrSyntax       = 3;
    static int const LUAErrMem          = 4;
    static int const LUAErrErr          = 5;
    static int const LUAErrFile         = 6;
    
    class lua_state;
    typedef const char* (*lua_reader) (lua_state*, void*, size_t*);
    typedef int (*lua_cfunction) (lua_state*);
    typedef void* (*lua_alloc) (void*, void*, size_t, size_t);
    typedef struct {
        const char* name;
        lua_cfunction func;
    } luaL_reg;

    /*
     * LUA function implementations
     */
    namespace lapi {
        int pcall       (lua_state*);
        int dofile      (lua_state*);
        int dohttpreq   (lua_state*);
        int log         (lua_state*);
        int unzip       (lua_state*);
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

