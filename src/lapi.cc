#include <blt/lapi.hh>
#include <blt/hook.hh>
#include <blt/log.hh>
#include <blt/fs.hh>

#include <sys/stat.h>

#include <vector>
#include <string>
#include <cstddef> // size_t

namespace blt {
    namespace lapi {

        using std::string;
        using std::vector;

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
                size_t unused;
                log::log(lua_tolstring(state, -1, &unused), log::LOG_ERROR);
                return 0;
            }

            lua_pushboolean(state, result == 0);
            lua_insert(state, 1);

            return lua_gettop(state);
        }

        /*
         * BLT Native Filesystem LUA API
         */

        static inline int
        os_get_dir_content_impl(lua_state* state, bool listFiles)
        {
        }

        int
        getdir(lua_state* state)
        {
            return 1;
        }

        int 
        getfiles(lua_state* state)
        {
            return 1;
        }

        int
        dir_exists(lua_state* state)
        {
            size_t len;
            const char* path = lua_tolstring(state, 1, &len);
            string stdPath = string(path);

            lua_pushboolean(state, fs::path_is_dir(stdPath));
            return 1;
        }

        /**
         * Load a LUA file from a LUA script
         */
        int
        loadfile(lua_state* state)
        {
            int args = lua_gettop(state);
            size_t length = 0;
            const char* filename = lua_tolstring(state, 1, &length);
            int error = luaL_loadfile(state, filename);

            if (error = LUAErrSyntax)
            {
                size_t unused;
                log::log("LUAErrSyntax loading LUA file " + string(filename), log::LOG_ERROR);
                log::log(lua_tolstring(state, -1, &unused), log::LOG_ERROR);
            }

            error = lua_pcall(state, 0, 0, 0);

            if (error = LUAErrRun)
            {
                size_t unused;
                log::log("LUAErrRun loading LUA file " + string(filename), log::LOG_ERROR);
                log::log(lua_tolstring(state, -1, &unused), log::LOG_ERROR);
            }

            return 0;
        }

        /*
         * log impl
         */

        int
        log(lua_state* state)
        {
            size_t len;
            const char* str = lua_tolstring(state, 1, &len);
            log::log(str, log::LOG_LUA);
            return 0;
        }

        /*
         * HTTP details moved to lapi_http.cc
         */

        int
        log(lua_state* state)
        {
            size_t len;
            const char* str = lua_tolstring(state, 1, &len);
            log::log(str, log::LOG_LUA);
        }

    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

