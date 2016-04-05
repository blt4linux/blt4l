extern "C" {
#include <dlfcn.h>
}
#include <iostream>
#include <subhook.h>
#include <blt/hook.hh>
#include <blt/lapi.hh>
#include <list>

namespace blt {

    using std::cerr;
    using std::cout;

    extern "C" {
        void        (*blt_lua_call)         (lua_state*, int, int);
        int         (*blt_lua_pcall)        (lua_state*, int, int, int);
        int         (*blt_lua_gettop)       (lua_state*);
        void        (*blt_lua_settop)       (lua_state*, int);
        const char* (*blt_lua_tolstring)    (lua_state*, int, size_t*);
        int         (*blt_luaL_loadfile)    (lua_state*, const char*);
        int         (*blt_lua_load)         (lua_state*, lua_reader*, void*, const char*);
        void        (*blt_lua_setfield)     (lua_state*, int, const char*);
        void        (*blt_lua_createtable)  (lua_state*, int, int);
        void        (*blt_lua_insert)       (lua_state*, int);
        lua_state*  (*blt_lua_newstate)     (lua_alloc, void*);
        void        (*blt_lua_close)        (lua_state*);
        void        (*blt_lua_rawset)       (lua_state*, int);
        void        (*blt_lua_settable)     (lua_state*, int);
        void        (*blt_lua_pushnumber)   (lua_state*, double);
        void        (*blt_lua_pushinteger)  (lua_state*, ptrdiff_t);
        void        (*blt_lua_pushboolean)  (lua_state*, bool);
        void        (*blt_lua_pushcclosure) (lua_state*, lua_cfunction, int);
        void        (*blt_lua_pushlstring)  (lua_state*, const char*, size_t);
        void        (*blt_luaL_openlib)     (lua_state*, const char*, const luaL_reg*, int);
        void        (*blt_luaL_ref)         (lua_state*, int);
        void        (*blt_lua_rawgeti)      (lua_state*, int, int);
        void        (*blt_luaL_unref)       (lua_state*, int, int);
        int         (*blt_luaL_newstate)    (char, char, int);

        /**
         * This is one of those damn C++ functions
         */
        void*       (*do_game_update)   (void* /* this */);
    }

    /*
     * Internal
     */

    std::list<lua_state*> activeStates;

    SubHook     gameUpdateDetour;
    SubHook     newStateDetour;
    SubHook     luaCallDetour;
    SubHook     luaCloseDetour;

    /*
     * State Management
     */

    void
    add_active_state(lua_state* state)
    {
        activeStates.push_back(state);
    }

    void
    remove_active_state(lua_state* state)
    {
        activeStates.remove(state);
    }

    bool
    check_active_state(lua_state* state)
    {
        std::list<lua_state*>::iterator stateIterator;

        for (stateIterator = activeStates.begin();
             stateIterator != activeStates.end();
             ++stateIterator) // is ++operator implemented? I guess we'll find out
        {
            // is this a real pointer.
            // lol C++
            if (*stateIterator == state)
            {
                return true;
            }
        }

        return false;
    }

    /*
     * Detour Impl
     */

    void*
    dt_Application_update(void* parentThis)
    {
        SubHook::ScopedRemove remove(&gameUpdateDetour);

        return do_game_update(parentThis);
    }

    /*
     * lua_newstate (and thus, luaL_newstate) intercept
     */
    lua_state*
    dt_lua_newstate(lua_alloc allocator, void* data)
    {

        SubHook::ScopedRemove remove(&newStateDetour);
        lua_state* state = blt_lua_newstate(allocator, data);
#       define lua_mapfn(name, function) \
            blt_lua_pushcclosure(state, function, 0); \
            blt_lua_setfield(state, LUAGlobalsIndex, name);
       
        if (!state)
        {
            return state; // null anyways, but whatever.
        }

        add_active_state(state);

        int stackSize = blt_lua_gettop(state);

        cerr << "stackSize = " << stackSize << "\n"; // iostreams suck

       
        /*
         * Install BLT API-extensions in to the LUA context
         */

        lua_mapfn("pcall",      lapi::pcall);
        lua_mapfn("dofile",     lapi::dofile);
        lua_mapfn("dohttpreq",  lapi::dohttpreq);
        lua_mapfn("log",        lapi::log);
        lua_mapfn("unzip",      lapi::unzip);


        return state;
#       undef lua_mapfn
    }

    void
    InitLUAHooks(void* dlHandle)
    {
#       define setcall(symbol,ptr) *(void**) (&ptr) = dlsym(dlHandle, #symbol); \

        cerr << "setting up lua function access\n";

        /*
         * DL Init
         */

        {
            setcall(lua_call,           blt_lua_call);
            setcall(lua_pcall,          blt_lua_pcall);
            setcall(lua_gettop,         blt_lua_gettop);
            setcall(lua_settop,         blt_lua_settop);
            setcall(lua_tolstring,      blt_lua_tolstring);
            setcall(luaL_loadfile,      blt_luaL_loadfile);
            setcall(lua_load,           blt_lua_load);
            setcall(lua_setfield,       blt_lua_setfield);
            setcall(lua_createtable,    blt_lua_createtable);
            setcall(lua_insert,         blt_lua_insert);
            setcall(lua_newstate,       blt_lua_newstate);
            setcall(lua_close,          blt_lua_close);
            setcall(lua_rawset,         blt_lua_rawset);
            setcall(lua_settable,       blt_lua_settable);
            setcall(lua_pushnumber,     blt_lua_pushnumber);
            setcall(lua_pushinteger,    blt_lua_pushinteger);
            setcall(lua_pushboolean,    blt_lua_pushboolean);
            setcall(lua_pushcclosure,   blt_lua_pushcclosure);
            setcall(lua_pushlstring,    blt_lua_pushlstring);
            setcall(luaL_openlib,       blt_luaL_openlib);
            setcall(luaL_ref,           blt_luaL_ref);
            setcall(lua_rawgeti,        blt_lua_rawgeti);
            setcall(luaL_unref,         blt_luaL_unref);
            setcall(luaL_newstate,      blt_luaL_newstate);

            setcall(_ZN11Application6updateEv, do_game_update); // _ZN11Application6updateEv = Application::update()
        }


        /*
         * Intercept Init
         */

        {
           // These function intercepts have a hidden pointer param for `this`
           gameUpdateDetour.Install((void *) do_game_update,    (void*) dt_Application_update);

           // These are proper C functions
           newStateDetour.Install((void *) blt_lua_newstate,       (void*) dt_lua_newstate);
        }

#       undef setcall
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
