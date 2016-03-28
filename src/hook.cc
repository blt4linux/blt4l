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
    void        (*luaL_openlib)     (lua_state*, const char*, const luaL_reg*, int);
    void        (*luaL_ref)         (lua_state*, int);
    void        (*lua_rawgeti)      (lua_state*, int, int);
    void        (*luaL_unref)       (lua_state*, int, int);
    void        (*do_game_update)   ();
    int         (*luaL_newstate)    (char, char, int);

    /*
     * Internal
     */

    SubHook     gameUpdateDetour;
    SubHook     newStateDetour;
    SubHook     luaCallDetour;
    SubHook     luaCloseDetour;

    void
    dslUpdateDetour()
    {
        SubHook::ScopedRemove remove(&gameUpdateDetour);
        fprintf(stderr, "dsl::EventManager::update() detour called\n");

        return do_game_update();
    }

    void
    InitLUAHooks(void* dlHandle)
    {
#	define setcall(name) \
	    ret = dlsym(dlHandle, #name); \
	    fprintf(stderr, "%s = %p\n", #name, ret); \
	    *(void **) (&name) = ret;

        fprintf(stderr, "setting up lua function access\n");

        {
            void* ret;
            setcall(lua_call);
            setcall(lua_pcall);
            setcall(lua_gettop);
            setcall(lua_settop);
            setcall(lua_tolstring);
            setcall(luaL_loadfile);
            setcall(lua_load);
            setcall(lua_setfield);
            setcall(lua_createtable);
            setcall(lua_insert);
            setcall(lua_newstate);
            setcall(lua_close);
            setcall(lua_rawset);
            setcall(lua_settable);
            setcall(lua_pushnumber);
            setcall(lua_pushinteger);
            setcall(lua_pushboolean);
            setcall(lua_pushcclosure);
            setcall(lua_pushlstring);
            setcall(luaL_openlib);
            setcall(luaL_ref);
            setcall(lua_rawgeti);
            setcall(luaL_unref);
            setcall(do_game_update);
            setcall(luaL_newstate);
        }

        fprintf(stderr, "setting up intercepts\n");

        {
            gameUpdateDetour.Install((void *)do_game_update, (void *)dslUpdateDetour);
        }

#       undef setcall
    }

}
