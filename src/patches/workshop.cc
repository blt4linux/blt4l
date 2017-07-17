#include <lua.hh>
#include <string>
#include <cstdio>
#include <subhook.h>

#include <blt/log.hh>
#include <blt/lapi.hh>
#include <blt/hook_support.hh>

#include <dsl/DB.hh>
#include <dsl/ScriptIdstring.hh>

extern "C" {
#   include <dlfcn.h>
}

namespace blt {
   namespace patch {
      namespace workshop {

         bool (*dsl_ScriptValue_idstring_is)(lua_state*, int);
         dsl::ScriptIdstring* (*dsl_ScriptValue_idstring_get)(lua_state*, int);
         int64_t (*dsl_ScriptValue_idstring_to_string)(lua_state*);

         SubHook sh_lua_pushstring;
         void
         dt_lua_pushstring(lua_state* s, const char* str)
         {
            hook_remove(sh_lua_pushstring);
            lua_pushstring(s, str);
            printf("lua_pushstring(%s)\n", str);
         }

         int
         db_create_entry(lua_state* state)
         {
            int tp1 = lua_type(state, 1);
            int tp2 = lua_type(state, 2);
            int tp3 = lua_type(state, 3);
            int tp4 = lua_type(state, 4);
            

            dsl::MainDB* _this = static_cast<dsl::MainDB*>(lua_touserdata(state, 1)); // or is it a wrapper???
            dsl::ScriptIdstring* entry_type  = dsl_ScriptValue_idstring_get(state, 2);
            dsl::ScriptIdstring* entry_id    = dsl_ScriptValue_idstring_get(state, 3);
            char const* entry_path  = lua_tolstring(state, 4, NULL);

            lua_pushvalue(state, 2);
            dsl_ScriptValue_idstring_to_string(state);
            size_t kys = 0;
            char const* wtf = lua_tolstring(state, 1, &kys);

            printf("DB:create_entry(%p, %s, %p, %s)\n",
                     _this, wtf, entry_id, entry_path);
            return 0;
         }

         int
         db_has(lua_state* state)
         {
            return 1;
         }

         int
         db_list(lua_state* state)
         {
            return 1;
         }

         int
         db_is_bundled(lua_state* state)
         {
            return 1;
         }

         int
         db_load_node(lua_state* state)
         {
            return 1;
         }

         int
         db_lookup(lua_state* state)
         {
            return 1;
         }

         int
         db_mods(lua_state* state)
         {
            return 1;
         }

         int
         db_open(lua_state* state)
         {
            return 1;
         }

         int
         db_reload(lua_state* state)
         {
            return 1;
         }

         int
         db_reverse_lookup(lua_state* state)
         {
            return 1;
         }

         void
         _configure_lua(lua_state* state)
         {
            lua_getfield(state, 0xFFFFD8EELL, "MainDB");
            int tp = lua_type(state, 1);
            printf("Enviorn/MainDB Type is %s\n",
                     lua_typename(state, tp));
         }


         /**
          * Since I don't currently want to figure out how to reopen the MainDB lua class, 
          * I'll just hook the setup and add requisites.
          */

         void (*dsl_MainDB_add_members)(lua_state*);
         SubHook sh_dsl_MainDB_add_members;

         void
         dt_dsl_MainDB_add_members(lua_state* state)
         {
            printf("hook: MainDB::add_members()\n");
            hook_remove(sh_dsl_MainDB_add_members);
            lua_pushcclosure(state, db_create_entry, 0);
            lua_setfield(state, 0xFFFFFFFELL, "create_entry");
            dsl_MainDB_add_members(state);
         }

         void
         _configure_hooks(void* dl)
         {
            setcall(dl, _ZN3dsl6MainDB11add_membersEP9lua_State, dsl_MainDB_add_members);
            setcall(dl, _ZN3dsl11ScriptValueINS_14ScriptIdstringEE2isEP9lua_Statei, dsl_ScriptValue_idstring_is);
            setcall(dl, _ZN3dsl11ScriptValueINS_14ScriptIdstringEE7get_varEP9lua_Statei, dsl_ScriptValue_idstring_get);
            setcall(dl, _ZN3dsl11ScriptValueINS_14ScriptIdstringEE9to_stringEP9lua_State, dsl_ScriptValue_idstring_to_string);

            sh_dsl_MainDB_add_members.Install((void*) dsl_MainDB_add_members, (void*) dt_dsl_MainDB_add_members);
            sh_lua_pushstring.Install((void*) &lua_pushstring, (void*) dt_lua_pushstring);
         }
      }
   }
}
