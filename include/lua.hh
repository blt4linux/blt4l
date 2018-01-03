#pragma once

#include <cstddef>

static int const LUARegistryIndex   = -10000;
static int const LUAGlobalsIndex    = -10002;
static int const LUAYield           = 1;
static int const LUAErrRun          = 2;
static int const LUAErrSyntax       = 3;
static int const LUAErrMem          = 4;
static int const LUAErrErr          = 5;
static int const LUAErrFile         = 6;
static int const LUAIdSize          = 60;

class lua_state;
typedef const char* (*lua_reader) (lua_state*, void*, size_t*);
typedef int (*lua_cfunction) (lua_state*);
typedef void* (*lua_alloc) (void*, void*, size_t, size_t);
typedef struct {
  const char* name;
  lua_cfunction func;
} luaL_Reg;

typedef struct lua_debug {
   int event;
   const char *name;           /* (n) */
   const char *namewhat;       /* (n) */
   const char *what;           /* (S) */
   const char *source;         /* (S) */
   int currentline;            /* (l) */
   int nups;                   /* (u) number of upvalues */
   int linedefined;            /* (S) */
   int lastlinedefined;        /* (S) */
   char short_src[LUAIdSize]; /* (S) */
} lua_Debug;

extern "C" {
   void         lua_call(lua_state*, int, int);
   int          lua_pcall(lua_state*, int, int, int);
   int          lua_gettop(lua_state*);
   void         lua_settop(lua_state*, int);
   int          luaL_loadfile(lua_state*, const char*);
   int          lua_load(lua_state*, lua_reader, void*, const char*);
   void         lua_getfield(lua_state*, int, const char*);
   void         lua_setfield(lua_state*, int, const char*);
   void         lua_createtable(lua_state*, int, int);
   void         lua_insert(lua_state*, int);
   lua_state*   lua_newstate(lua_alloc, void*);
   lua_state*   luaL_newstate(void);
   void         lua_close(lua_state*);
   void         lua_rawset(lua_state*, int);
   void         lua_settable(lua_state*, int);
   void         luaL_openlib(lua_state*, const char*, const luaL_Reg*, int);
   int          luaL_ref(lua_state*, int);
   void         luaL_unref(lua_state*, int, int);

   void         lua_rawgeti(lua_state*, int, int);
   void         lua_pushnumber(lua_state*, double);
   void         lua_pushinteger(lua_state*, ptrdiff_t);
   void         lua_pushboolean(lua_state*, bool);
   void         lua_pushcclosure(lua_state*, lua_cfunction, int);
   void         lua_pushlstring(lua_state*, const char*, size_t);

   const char*  lua_tolstring(lua_state*, int, size_t*);
   void*        lua_touserdata(lua_state*, int);

   const char*  luaL_checklstring(lua_state*, int, size_t*);
   const char*  luaL_optlstring(lua_state*, int, const char*, size_t*);
   int          luaL_loadbuffer(lua_state*, const char*, size_t, const char*);
   void         luaL_checkstack(lua_state*, int, const char*);
   int          luaL_error(lua_state*, const char*, ...);
   void         luaL_checktype(lua_state*, int, int);
   void         luaL_checkany(lua_state*, int);

   void         lua_pushnil(lua_state*);
   void         lua_pushvalue(lua_state*, int);
   int          lua_isstring(lua_state*, int);
   void         lua_replace(lua_state*, int);
   void         lua_remove(lua_state*, int);
   int          lua_error(lua_state*);
   int          lua_type(lua_state*, int);

   int          lua_getinfo(lua_state*, const char*, lua_debug*);
   int          lua_getstack(lua_state*, int, lua_debug*);

}

/* vim: set ts=3 softtabstop=0 sw=3 expandtab: */

