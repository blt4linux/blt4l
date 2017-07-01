#pragma once

#include <lua.hh>
#include <string>

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

      public:
         void* newstate(bool, bool, Allocation);

         // State getters
         void* main_state(void);
         void* thread_state(void);

         /**
          * Look up lua_state object by diesel threadpool thread ID, maybe?
          */
         lua_state* state_for_thread(unsigned long);

         /**
          * Compile & load a string
          */
         int64_t load_string(char const*);
         int64_t do_string(char const*); // XXX just slightly different
         bool can_compile_string(char const*);

         int64_t clear_require_cache(void);

#if defined(BLT_USING_LIBCXX)
         int64_t do_file(std::string const&);
         int64_t require1(std::string const&);
#endif

         /**
          * What it says on the tin.
          * Result is whatever lua_gettop returns
          */
         int64_t get_global(char const*);

   };
}
