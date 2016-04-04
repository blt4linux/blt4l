extern "C" {
#include <dlfcn.h>
}
#include <iostream>
#include <subhook.h>
#include <blt/hook.hh>
#include <list>

namespace blt {

    using std::cerr;

    std::list<lua_state*> activeStates;

extern "C" {
    void        (*olua_call)         (lua_state*, int, int);
    int         (*olua_pcall)        (lua_state*, int, int, int);
    int         (*olua_gettop)       (lua_state*);
    void        (*olua_settop)       (lua_state*, int);
    const char* (*olua_tolstring)    (lua_state*, int, size_t*);
    int         (*oluaL_loadfile)    (lua_state*, const char*);
    int         (*olua_load)         (lua_state*, lua_reader*, void*, const char*);
    void        (*olua_setfield)     (lua_state*, int, const char*);
    void        (*olua_createtable)  (lua_state*, int, int);
    void        (*olua_insert)       (lua_state*, int);
    lua_state*  (*olua_newstate)     (lua_alloc, void*);
    void        (*olua_close)        (lua_state*);
    void        (*olua_rawset)       (lua_state*, int);
    void        (*olua_settable)     (lua_state*, int);
    void        (*olua_pushnumber)   (lua_state*, double);
    void        (*olua_pushinteger)  (lua_state*, ptrdiff_t);
    void        (*olua_pushboolean)  (lua_state*, bool);
    void        (*olua_pushcclosure) (lua_state*, lua_cfunction, int);
    void        (*olua_pushlstring)  (lua_state*, const char*, size_t);
    void        (*oluaL_openlib)     (lua_state*, const char*, const luaL_reg*, int);
    void        (*oluaL_ref)         (lua_state*, int);
    void        (*olua_rawgeti)      (lua_state*, int, int);
    void        (*oluaL_unref)       (lua_state*, int, int);
    int         (*oluaL_newstate)    (char, char, int);

    void        (*do_game_update)   ();
}

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
        cerr << "dsl::EventManager::update() detour called\n";

        return do_game_update();
    }

    void
    InitLUAHooks(void* dlHandle)
    {
#       define setcall(name) \
            ret = dlsym(dlHandle, #name); \
            cerr << #name << " = " << ret << "\n"; \
            *(void **) (&o ## name) = ret;

        cerr << "setting up lua function access\n";

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

        ret = dlsym(dlHandle, "_ZN3dsl12EventManager6updateEv");    // dsl::EventManager::update
        cerr << "_ZN3dsl12EventManager6updateEv" << " = " << ret << "\n";
        *(void **) (&do_game_update) = ret;

            setcall(luaL_newstate);
        }

// TODO: fix this!
//        cerr << "setting up intercepts\n";
//
//        {
//            gameUpdateDetour.Install((void *)do_game_update, (void *)dslUpdateDetour);
//        }

#       undef setcall
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
