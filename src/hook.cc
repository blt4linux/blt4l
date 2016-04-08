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
    lua_state*  (*luaL_newstate)    (void);
    void*       (*dsl_lua_newstate) (lua_state** /* this */, bool, bool, bool);
    void        (*lua_close)        (lua_state*);
    void        (*lua_rawset)       (lua_state*, int);
    void        (*lua_settable)     (lua_state*, int);
    void        (*lua_pushnumber)   (lua_state*, double);
    void        (*lua_pushinteger)  (lua_state*, ptrdiff_t);
    void        (*lua_pushboolean)  (lua_state*, bool);
    void        (*lua_pushcclosure) (lua_state*, lua_cfunction, int);
    void        (*lua_pushlstring)  (lua_state*, const char*, size_t);
    void        (*luaL_openlib)     (lua_state*, const char*, const luaL_reg*, int);
    int         (*luaL_ref)         (lua_state*, int);
    void        (*lua_rawgeti)      (lua_state*, int, int);
    void        (*luaL_unref)       (lua_state*, int, int);

    /**
     * This is one of those damn C++ functions
     */
    void*       (*do_game_update)   (void* /* this */);

    /*
     * Internal
     */

    std::list<lua_state*> activeStates;


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

    SubHook     gameUpdateDetour;
    SubHook     newStateDetour;
    SubHook     luaCallDetour;
    SubHook     luaCloseDetour;

    void*
    dt_Application_update(void* parentThis)
    {
        SubHook::ScopedRemove remove(&gameUpdateDetour);

        return do_game_update(parentThis);
    }

    void*
    dt_dsl_lua_newstate(lua_state** pThis, bool b1, bool b2, bool allocator)
    {
#       define lua_mapfn(name, function) \
            lua_pushcclosure(state, function, 0); \
            lua_setfield(state, LUAGlobalsIndex, name);

        SubHook::ScopedRemove remove(&newStateDetour);

        void* returnVal = dsl_lua_newstate(pThis, b1, b2, allocator);
        lua_state* state = *pThis; // wut

        if (!state)
        {
            return returnVal;
        }

        add_active_state(state);

        int stackSize = lua_gettop(state);

        /*
         * Install BLT API-extensions in to the LUA context
         */

        lua_mapfn("pcall",      lapi::pcall);
        lua_mapfn("dofile",     lapi::loadfile);
        lua_mapfn("dohttpreq",  lapi::dohttpreq);
        lua_mapfn("log",        lapi::log);
        // TODO: put back if implemented
       lua_mapfn("unzip",      lapi::unzip);


        return returnVal;
#       undef lua_mapfn
    }

    void
    dt_lua_close(lua_state* state)
    {
        SubHook::ScopedRemove remove(&luaCloseDetour);

        remove_active_state(state);
        lua_close(state);
    }

    void
    blt_init_hooks(void* dlHandle)
    {
#       define setcall(symbol,ptr) *(void**) (&ptr) = dlsym(dlHandle, #symbol); \

        cerr << "setting up lua function access\n";

        /*
         * DL Init
         */

        {
            setcall(lua_call,           lua_call);
            setcall(lua_pcall,          lua_pcall);
            setcall(lua_gettop,         lua_gettop);
            setcall(lua_settop,         lua_settop);
            setcall(lua_tolstring,      lua_tolstring);
            setcall(luaL_loadfile,      luaL_loadfile);
            setcall(lua_load,           lua_load);
            setcall(lua_setfield,       lua_setfield);
            setcall(lua_createtable,    lua_createtable);
            setcall(lua_insert,         lua_insert);
            setcall(lua_newstate,       lua_newstate);
            setcall(lua_close,          lua_close);
            setcall(lua_rawset,         lua_rawset);
            setcall(lua_settable,       lua_settable);
            setcall(lua_pushnumber,     lua_pushnumber);
            setcall(lua_pushinteger,    lua_pushinteger);
            setcall(lua_pushboolean,    lua_pushboolean);
            setcall(lua_pushcclosure,   lua_pushcclosure);
            setcall(lua_pushlstring,    lua_pushlstring);
            setcall(luaL_openlib,       luaL_openlib);
            setcall(luaL_ref,           luaL_ref);
            setcall(lua_rawgeti,        lua_rawgeti);
            setcall(luaL_unref,         luaL_unref);
            setcall(luaL_newstate,      luaL_newstate);

            // _ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE = dsl::LuaInterface::newstate(...) 
            setcall(_ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE, dsl_lua_newstate); 
            // _ZN11Application6updateEv = Application::update()
            setcall(_ZN11Application6updateEv, do_game_update); 
        }


        /*
         * Intercept Init
         */

        {
            // These function intercepts have a hidden pointer param for `this`
            gameUpdateDetour.Install((void*) do_game_update,    (void*) dt_Application_update);

            // These are proper C functions
            newStateDetour.Install((void*) dsl_lua_newstate,    (void*) dt_dsl_lua_newstate);
            luaCloseDetour.Install((void*) lua_close,           (void*) dt_lua_close);
        }

#       undef setcall
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
