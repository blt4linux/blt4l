extern "C" {
#include <dlfcn.h>
}
#include <iostream>
#include <subhook.h>
#include <blt/hook.hh>
#include <blt/http.hh>
#include <blt/lapi.hh>
#include <blt/log.hh>
#include <blt/event.hh>
#include <list>
#include <string>

#define hook_remove(hookName) SubHook::ScopedRemove _sh_remove_raii(&hookName)

namespace blt {

    using std::cerr;
    using std::cout;
    using std::string;

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
    void        (*luaL_openlib)     (lua_state*, const char*, const luaL_Reg*, int);
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
    SubHook     luaNewStateDetour;
    SubHook     luaCallDetour;
    SubHook     luaCloseDetour;

    void*
    dt_Application_update(void* parentThis)
    {
        hook_remove(gameUpdateDetour);

        if (HTTPManager::get_instance()->locks_initd() == false)
        {
            HTTPManager::get_instance()->init_locks();
        }

        event::EventQueue::get_instance()->process_events();

        return do_game_update(parentThis);
    }

    void
    dt_lua_call(lua_state* state, int argCount, int resultCount)
    {
        hook_remove(luaNewStateDetour);

        int result = lua_pcall(state, argCount, resultCount, 0);

        if (result != 0)
        {
            size_t len;
            log::log("lua_call: error in lua_pcall: " + string(lua_tolstring(state, -1, &len)), log::LOG_ERROR);
        }
    }

    void*
    dt_dsl_lua_newstate(lua_state** pThis, bool b1, bool b2, bool allocator)
    {
#       define lua_mapfn(name, function) \
            lua_pushcclosure(state, function, 0); \
            lua_setfield(state, LUAGlobalsIndex, name);

        hook_remove(luaNewStateDetour);

        void* returnVal = dsl_lua_newstate(pThis, b1, b2, allocator);
        lua_state* state = *pThis; // wut

        if (!state)
        {
            return returnVal;
        }

        add_active_state(state);

        int stackSize = lua_gettop(state);

        log::log("installing BLT LUA API", log::LOG_INFO);

        /*
         * Install BLT API-extensions in to the LUA context
         */

        lua_mapfn("pcall",      lapi::pcall);
        lua_mapfn("dofile",     lapi::loadfile);
        lua_mapfn("dohttpreq",  lapi::dohttpreq);
        lua_mapfn("log",        lapi::log);
        lua_mapfn("unzip",      lapi::unzip);

        /*
         * Map native libraries
         */

        {
            luaL_Reg consoleLib[] = {
                { "CreateConsole",  lapi::console_noop },
                { "DestroyConsole", lapi::console_noop },
                { NULL, NULL }
            };
            luaL_openlib(state, "console", consoleLib, 0);

            luaL_Reg fileLib[] = {
                { "GetDirectories",     lapi::getdir        },
                { "GetFiles",           lapi::getfiles      },
                { "RemoveDirectory",    lapi::removedir     },
                { "DirectoryExists",    lapi::dir_exists    },
                { NULL, NULL }
            };
            luaL_openlib(state, "file", fileLib, 0);
        }


        log::log("Loading BLT Base");

        {
            int result;

            result = luaL_loadfile(state, "mods/base/base.lua");
            log::log("luaL_loadfile() = " + std::to_string(result), log::LOG_INFO);

            if (result == LUAErrSyntax) 
            {
                size_t len;
                log::log("Loading BLT Base failed (Syntax Error)", log::LOG_ERROR);
                log::log(lua_tolstring(state, -1, &len), log::LOG_ERROR);
                return returnVal;
            }

            result = lua_pcall(state, 0, 1, 0);
            log::log("lua_pcall(...) = " + std::to_string(result), log::LOG_INFO);

            if (result == LUAErrRun)
            {
                size_t len;
                log::log("Loading BLT Base failed (Runtime Error)", log::LOG_ERROR);
                log::log(lua_tolstring(state, -1, &len), log::LOG_ERROR);
                return returnVal;
            }
        }

        lua_settop(state, stackSize);

        return returnVal;
#       undef lua_mapfn
    }

    void
    dt_lua_close(lua_state* state)
    {
        hook_remove(luaCloseDetour);

        remove_active_state(state);
        lua_close(state);
    }

    void
    blt_init_hooks(void* dlHandle)
    {
#       define setcall(symbol,ptr) *(void**) (&ptr) = dlsym(dlHandle, #symbol); \

        log::log("finding lua functions", log::LOG_INFO);

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

        log::log("installing hooks", log::LOG_INFO);

        /*
         * Intercept Init
         */

        {
            // These function intercepts have a hidden pointer param for `this`
            gameUpdateDetour.Install    ((void*) do_game_update,    (void*) dt_Application_update);

            // These are proper C functions
            luaNewStateDetour.Install   ((void*) dsl_lua_newstate,  (void*) dt_dsl_lua_newstate);
            luaCloseDetour.Install      ((void*) lua_close,         (void*) dt_lua_close);
            luaCallDetour.Install       ((void*) lua_call,          (void*) dt_lua_call);
        }

#       undef setcall
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
