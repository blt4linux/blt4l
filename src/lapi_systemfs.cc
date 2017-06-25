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
            // assuming PWD is base folder
            const char* path = lua_tolstring(state, 1, NULL);
            struct stat _stat;
            lua_pushboolean(state, stat(path, &_stat));
            return 1;
         }
      }
   }
}
