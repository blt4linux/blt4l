#include <blt/lapi.hh>
#include <blt/hook.hh>
#include <string>
#include <cstddef> // size_t

namespace blt {
    namespace lapi {
        using std::string;

        /*
         * pcall impl
         */

        int
        pcall(lua_state* state)
        {
            int args = lua_gettop(state);
            int result = lua_pcall(state, args - 1, -1, 0);

            if (result == LUAErrRun) 
            {
                // TODO Logging::...
                return 0;
            }

            lua_pushboolean(state, result == 0);
            lua_insert(state, 1);

            return lua_gettop(state);
        }

        /*
         * dofile impl
         */

        int
        dofile(lua_state* state)
        {
            int args = lua_gettop(state);
            size_t length = 0;
            const char* filename = lua_tolstring(state, 1, &length);
            int error = luaL_loadfile(state, filename);

            if (error = LUAErrSyntax)
            {
                // TODO Logging::...
            }

            error = lua_pcall(state, 0, 0, 0);

            if (error = LUAErrRun)
            {
                // TODO logging
            }

            return 0;
        }

        /*
         * HTTP details moved to lapi_http.cc
         */

    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

