#pragma once

#include <lua.hh>

namespace dsl {
   class LuaInterface {
      public:
         struct Allocation { 
            // will probably either be thunk'd dsl::main_heap_alloc or dsl::lua_alloc
            void* (*allocate)(void*, void*, unsigned long, unsigned long); 
         };

      public:
         // lua state appears to be at this+0 based on treatment in the original hooks
         lua_state* state;

         // typedef void* (*Allocation)(void*, void*, unsigned long, unsigned long);
         void* newstate(bool, bool, Allocation);
   };
}
