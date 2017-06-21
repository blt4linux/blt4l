#include <blt/lapi.hh>
#include <blt/hook.hh>

#include <string>

namespace blt {
   namespace lapi {

      static std::string PlatformName = "linux";

      int /* lua fn - "blt_platform" */
      blt_platform(lua_state* state)
      {
         lua_pushlstring(state, PlatformName.c_str(), PlatformName.length());
         return 1;
      }


      // TODO: Version Function?

   }
}
