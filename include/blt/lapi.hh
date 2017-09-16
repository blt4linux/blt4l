/**
 * LUA extension and override signatures
 */

#include <cstddef>
#pragma once

#include <lua.hh>

namespace blt {

    /*
     * LUA function implementations
     */
    namespace lapi {
        /*
         * LUA Meta-API 
         */
        int pcall           (lua_state*);
        int loadfile        (lua_state*);

        /*
         * FS API
         */
        int getdir          (lua_state*);
        int getfiles        (lua_state*);
        int dir_exists      (lua_state*);
        int removedir       (lua_state*);
        int createdir       (lua_state*);
        int movedir         (lua_state*);
        int hash            (lua_state*);

        /*
         * BLT API
         */
        int dohttpreq       (lua_state*);
        int log             (lua_state*);
        int unzip           (lua_state*);
        int console_noop    (lua_state*);
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

