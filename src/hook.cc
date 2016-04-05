extern "C" {
#include <dlfcn.h>
#include <stdio.h>
}
#include <iostream>
#include <subhook.h>
#include <blt/hook.hh>
#include <list>

namespace blt {

    using std::cerr;

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
        lua_state* state = olua_newstate(allocator, data);
       
        if (!state)
        {
            return state; // null anyways, but whatever. 
        }

        add_active_state(state);

        int stackSize = olua_gettop(state);

        cerr << "stackSize = " << stackSize << "\n"; // iostreams suck

        /*
         * TODO pcall
         * TODO dofile
         * TODO dohttpreq
         * TODO log
         * TODO unzip
         * TODO these should be confined to a subroutine to prevent polution
         */

        // TODO install BLT ext here

        return state;
    }

    void
    InitLUAHooks(void* dlHandle)
    {
#       define setcall(name) \
            ret = dlsym(dlHandle, #name); \
            cerr << #name << " = " << ret << "\n"; \
            *(void **) (&o ## name) = ret;

        cerr << "setting up lua function access\n";

        /*
         * DL Init
         */

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


        /*
         * Intercept Init
         */

        {
           // These function intercepts have a hidden pointer param for `this`
           gameUpdateDetour.Install((void *) do_game_update,    (void*) dt_Application_update);

           // These are proper C functions
           newStateDetour.Install((void *) olua_newstate,       (void*) dt_lua_newstate);
        }

#       undef setcall
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
