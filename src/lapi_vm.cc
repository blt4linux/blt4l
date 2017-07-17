/*
 * From lbaselib.c, lua.h
 *
 * Copyright (C) 1994-2012 Lua.org, PUC-Rio.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

/*
 ** basic types
 */
#define LUA_TNONE        (-1)

#define LUA_TNIL        0
#define LUA_TBOOLEAN        1
#define LUA_TLIGHTUSERDATA    2
#define LUA_TNUMBER        3
#define LUA_TSTRING        4
#define LUA_TTABLE        5
#define LUA_TFUNCTION        6
#define LUA_TUSERDATA        7
#define LUA_TTHREAD        8

/* option for multiple returns in `lua_pcall' and `lua_call' */
#define LUA_MULTRET    (-1)

// From somewhere
#define luaL_optstring(L,n,d) (luaL_optlstring(L, (n), (d), NULL))
///////////////////////


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lbaselib_c
#define LUA_LIB
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)

#include <blt/lapi.hh>
#include <blt/hook.hh>
#include <blt/log.hh>

#include <blt/lapi_vm.hh>

typedef lua_state lua_State;

static int load_aux (lua_State *L, int status) {
    if (status == 0)  /* OK? */
        return 1;
    else {
        lua_pushnil(L);
        lua_insert(L, -2);  /* put before error message */
        return 2;  /* return nil plus error message */
    }
}

static const char *generic_reader (lua_State *L, void *ud, size_t *size) {
    (void)ud;  /* to avoid warnings */
    luaL_checkstack(L, 2, "too many nested functions");
    lua_pushvalue(L, 1);  /* get function */
    lua_call(L, 0, 1);  /* call it */
    if (lua_isnil(L, -1)) {
        *size = 0;
        return NULL;
    }
    else if (lua_isstring(L, -1)) {
        lua_replace(L, 3);  /* save string in a reserved stack slot */
        return lua_tolstring(L, 3, size);
    }
    else luaL_error(L, "reader function must return a string");
    return NULL;  /* to avoid warnings */
}

static int luaB_dofile (lua_State *L) {
    const char *fname = luaL_optstring(L, 1, NULL);
    int n = lua_gettop(L);
    if (luaL_loadfile(L, fname) != 0) lua_error(L);
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

static int luaB_loadfile (lua_State *L) {
    const char *fname = luaL_optstring(L, 1, NULL);
    return load_aux(L, luaL_loadfile(L, fname));
}

static int luaB_load (lua_State *L) {
    int status;
    const char *cname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, 3);  /* function, eventual name, plus one reserved slot */
    status = lua_load(L, generic_reader, NULL, cname);
    return load_aux(L, status);
}

static int luaB_loadstring (lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    const char *chunkname = luaL_optstring(L, 2, s);
    return load_aux(L, luaL_loadbuffer(L, s, l, chunkname));
}

static int luaB_pcall (lua_State *L) {
    int status;
    luaL_checkany(L, 1);
    status = lua_pcall(L, lua_gettop(L) - 1, LUA_MULTRET, 0);
    lua_pushboolean(L, (status == 0));
    lua_insert(L, 1);
    return lua_gettop(L);  /* return status + all results */
}

static int luaB_xpcall (lua_State *L) {
    int status;
    luaL_checkany(L, 2);
    lua_settop(L, 2);
    lua_insert(L, 1);  /* put error function under function to be called */
    status = lua_pcall(L, 0, LUA_MULTRET, 1);
    lua_pushboolean(L, (status == 0));
    lua_replace(L, 1);
    return lua_gettop(L);  /* return status + all results */
}


/*
 * Everything below this point is purely from BLT4L, and is subject to
 * the standard BLT4L copyright.
 *
 * @auther ZNixian
 */

using namespace blt;
using namespace lapi;

static const luaL_Reg base_funcs[] = {
    {"dofile",        luaB_dofile        },
    {"loadfile",    luaB_loadfile    },
    {"load",        luaB_load        },
    {"loadstring",    luaB_loadstring    },
    {"pcall",        luaB_pcall        },
    {"xpcall",        luaB_xpcall        },

    {NULL, NULL}
};

namespace blt {
    namespace lapi {
        namespace vm {
            void base_open(lua_State *L) {
                /* Set up the library */
                luaL_openlib(L, "vm", base_funcs, 0);
            }
        }
    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

