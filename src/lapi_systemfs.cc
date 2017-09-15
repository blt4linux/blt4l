/**
 * SystemFS lib implementation (patch for missing SystemFS, which was added in the workshop update)
 */

extern "C" {
#   include <sys/stat.h>
}

#include <blt/hook.hh>

namespace blt {
   namespace lapi {
      namespace SystemFS {

         int
         exists(lua_state* state)
         {
            // Check arguments
            if(lua_gettop(state) != 2)
               luaL_error(state, "SystemFS:exists(path) takes a single argument, not %d arguments including SystemFS", lua_gettop(state));
            if(!lua_isstring(state, -1))
               luaL_error(state, "SystemFS:exists(path) -> argument 'path' must be a string!");

            // assuming PWD is base folder
            const char* path = lua_tolstring(state, -1, NULL);
            struct stat _stat;
            lua_pushboolean(state, stat(path, &_stat) == 0);
            return 1;
         }
      }
   }
}
