#include <blt/lapi.hh>
#include <blt/hook.hh>
#include <blt/log.hh>
#include <blt/fs.hh>
#include <blt/zip.hh>

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
        os_get_dir_content_impl(lua_state* state, bool listDirs)
        {
            vector<string> directories;
            string path;
            {
                size_t len;
                const char* cDir = lua_tolstring(state, 1, &len);
                path = string(cDir, len);
            }

            directories = fs::list_directory(path, listDirs);

            lua_createtable(state, 0, 0);

            {
                int idx = 1;
                vector<string>::iterator it;
                for(it = directories.begin();
                    it < directories.end();
                    ++it)
                {
                    if (*it == "." || *it == "..")
                    {
                        continue;
                    }

                    lua_pushinteger(state, idx);
                    lua_pushlstring(state, it->c_str(), it->length());
                    lua_settable(state, -3); // magic number?
                    ++idx;
                }
            }
            return 1;
        }

        int
        createdir(lua_state* state)
        {
            size_t _unused;
            const char* directory = lua_tolstring(state, 1, &_unused);
            lua_pushboolean(state, fs::create_directory(directory));
            return 1;
        }

        int
        getdir(lua_state* state)
        {
            return os_get_dir_content_impl(state, true);
        }

        int 
        getfiles(lua_state* state)
        {
            return os_get_dir_content_impl(state, false);
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

        int
        removedir(lua_state* state)
        {
            size_t len;
            const char* path = lua_tolstring(state, 1, &len);
            string stdPath = string(path);

            lua_pushboolean(state, fs::delete_directory(stdPath, false));
            return 1;
        }

        /*
         * LUA Meta-API
         */

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

            if (error == LUAErrSyntax)
            {
                size_t unused;
                log::log("LUAErrSyntax loading LUA file " + string(filename), log::LOG_ERROR);
                log::log(lua_tolstring(state, -1, &unused), log::LOG_ERROR);
            }

            error = lua_pcall(state, 0, 0, 0);

            if (error == LUAErrRun)
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
        unzip(lua_state* state)
        {
            size_t len;
            const char* arcPath = lua_tolstring(state, 1, &len);
            const char* destPath = lua_tolstring(state, 2, &len);

            zip::ZIPArchive* arc = new zip::ZIPArchive(arcPath, destPath);
            arc->read_archive();
            delete arc;
            return 0;
        }

        int
        console_noop(lua_state* state)
        {
            return 0;
        }

    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

