#include <subhook.h>
#include <blt/hook.hh>
#include <cstdio>
#include <list>
#include <dlfcn.h>

namespace blt {

    std::list<lua_state*> activeStates;

    void        (*lua_call)         (lua_state*, int, int);
    int         (*lua_pcall)        (lua_state*, int, int, int);
    int         (*lua_gettop)       (lua_state*);
    void        (*lua_settop)       (lua_state*, int);
    const char* (*lua_tolstring)    (lua_state*, int, size_t*);
    int         (*luaL_loadfile)    (lua_state*, const char*);
    int         (*lua_load)         (lua_state*, lua_reader*, void*, const char*);
    void        (*lua_setfield)     (lua_state*, int, const char*);
    void        (*lua_createtable)  (lua_state*, int, int);
    void        (*lua_insert)       (lua_state*, int);
    lua_state*  (*lua_newstate)     (lua_alloc, void*);
    void        (*lua_close)        (lua_state*);
    void        (*lua_rawset)       (lua_state*, int);
    void        (*lua_settable)     (lua_state*, int);
    void        (*lua_pushnumber)   (lua_state*, double);
    void        (*lua_pushinteger)  (lua_state*, ptrdiff_t);
    void        (*lua_pushboolean)  (lua_state*, bool);
    void        (*lua_pushcclosure) (lua_state*, lua_cfunction, int);
    void        (*lua_pushlstring)  (lua_state*, const char*, size_t);
    void        (*luaI_openlib)     (lua_state*, const char*, const luaL_reg*, int);
    void        (*luaL_ref)         (lua_state*, int);
    void        (*lua_rawgeti)      (lua_state*, int, int);
    void        (*luaL_unref)       (lua_state*, int, int);
    void*       (*do_game_update)   (int*, int*);
    int         (*luaL_newstate)    (char, char, int);

    /*
     * Internal
     */

    SubHook     gameUpdateDetour;
    SubHook     newStateDetour;
    SubHook     luaCallDetour;
    SubHook     luaCloseDetour;

    void* bltGameUpdate (int*, int*);

    void InitLUAHooks(void* dlHandle) {
#       define ffunc(name) ({ void* ret = dlsym(dlHandle, name); fprintf(stderr, "%s = %p\n", name, ret); ret; })
        fprintf(stderr, "setting up lua function access\n");

        {
            lua_call            = ffunc("lua_call");
            lua_pcall           = ffunc("lua_pcall");
            lua_gettop          = ffunc("lua_gettop");
            lua_settop          = ffunc("lua_settop");
            lua_tolstring       = ffunc("lua_tolstring");
            luaL_loadfile       = ffunc("luaL_loadfile");
            lua_load            = ffunc("lua_load");
            lua_setfield        = ffunc("lua_setfield");
            lua_createtable     = ffunc("lua_createtable");
            lua_insert          = ffunc("lua_insert");
            lua_newstate        = ffunc("lua_newstate");
            lua_close           = ffunc("lua_close");
            lua_rawset          = ffunc("lua_rawset");
            lua_settable        = ffunc("lua_settable");
            lua_pushnumber      = ffunc("lua_pushnumber");
            lua_pushinteger     = ffunc("lua_pushinteger");
            lua_pushboolean     = ffunc("lua_pushboolean");
            lua_pushcclosure    = ffunc("lua_pushcclosure");
            lua_pushlstring     = ffunc("lua_pushlstring");
            luaI_openlib        = ffunc("luaI_openlib");
            luaL_ref            = ffunc("luaL_ref");
            lua_rawgeti         = ffunc("lua_rawgeti");
            luaL_unref          = ffunc("luaL_unref");
            do_game_update      = ffunc("_ZN3dsl12EventManager6updateEv"); // dsl::EventManager::update
            luaL_newstate       = ffunc("luaL_newstate");
        }

        fprintf(stderr, "setting up intercepts\n");

        {
        }

#       define ffunc
    }

}
