#ifndef LUADEF_H
#define LUADEF_H

#include <cstddef>

 class lua_state;
 typedef const char* (*lua_reader) (lua_state*, void*, size_t*);
 typedef int (*lua_cfunction) (lua_state*);
 typedef void* (*lua_alloc) (void*, void*, size_t, size_t);
 typedef struct {
     const char* name;
     lua_cfunction func;
 } luaL_reg;

 /*
  * Signature definitions for functions defined in the PD binary
  */

extern "C" {
    void        lua_call         (lua_state*, int, int);
    int         lua_pcall        (lua_state*, int, int, int);
    int         lua_gettop       (lua_state*);
    void        lua_settop       (lua_state*, int);
    const char* lua_tolstring    (lua_state*, int, size_t*);
    int         luaL_loadfile    (lua_state*, const char*);
    int         lua_load         (lua_state*, lua_reader*, void*, const char*);
    void        lua_setfield     (lua_state*, int, const char*);
    void        lua_createtable  (lua_state*, int, int);
    void        lua_insert       (lua_state*, int);
    lua_state*  lua_newstate     (lua_alloc, void*);
    void        lua_close        (lua_state*);
    void        lua_rawset       (lua_state*, int);
    void        lua_settable     (lua_state*, int);
    void        lua_pushnumber   (lua_state*, double);
    void        lua_pushinteger  (lua_state*, ptrdiff_t);
    void        lua_pushboolean  (lua_state*, bool);
    void        lua_pushcclosure (lua_state*, lua_cfunction, int);
    void        lua_pushlstring  (lua_state*, const char*, size_t);
    void        luaI_openlib     (lua_state*, const char*, const luaL_reg*, int);
    void        luaL_ref         (lua_state*, int);
    void        luaL_openlib     (lua_state*, const char*, const luaL_reg*, int);
    void        lua_rawgeti      (lua_state*, int, int);
    void        luaL_unref       (lua_state*, int, int);
    int         luaL_newstate    (char, char, int);
}

/*
 * pseudo-indices
 */
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* thread status; 0 is OK */
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5

#endif // LUADEF_H
