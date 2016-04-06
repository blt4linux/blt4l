extern "C" {
#include <dlfcn.h>
}
#include <subhook.h>
#include <blt/hook.h>
#include <blt/files.h>
#include <blt/logging.h>
#include <list>

namespace blt {

    extern "C" {
        void        (*olua_call)         (lua_state*, int, int);
        int         (*olua_pcall)        (lua_state*, int, int, int);
        int         (*olua_gettop)       (lua_state*);
        void        (*olua_settop)       (lua_state*, int);
        const char* (*olua_tolstring)    (lua_state*, int, size_t*);
        int         (*oluaL_loadfile)    (lua_state*, const char*);
        int         (*olua_load)         (lua_state*, lua_reader*, void*, const char*);
        void        (*olua_setfield)     (lua_state*, int, const char*);
        void        (*olua_createtable)  (lua_state*, int, int);
        void        (*olua_insert)       (lua_state*, int);
        lua_state*  (*olua_newstate)     (lua_alloc, void*);
        void        (*olua_close)        (lua_state*);
        void        (*olua_rawset)       (lua_state*, int);
        void        (*olua_settable)     (lua_state*, int);
        void        (*olua_pushnumber)   (lua_state*, double);
        void        (*olua_pushinteger)  (lua_state*, ptrdiff_t);
        void        (*olua_pushboolean)  (lua_state*, bool);
        void        (*olua_pushcclosure) (lua_state*, lua_cfunction, int);
        void        (*olua_pushlstring)  (lua_state*, const char*, size_t);
        void        (*oluaL_openlib)     (lua_state*, const char*, const luaL_reg*, int);
        void        (*oluaL_ref)         (lua_state*, int);
        void        (*olua_rawgeti)      (lua_state*, int, int);
        void        (*oluaL_unref)       (lua_state*, int, int);
        int         (*oluaL_newstate)    (char, char, int);

        /**
         * This is one of those damn C++ functions
         */
        void*       (*do_game_update)   (void* /* this */);
    }

    /*
     * Internal
     */

    std::list<lua_state*> activeStates;

    SubHook     gameUpdateDetour;
    SubHook     newStateDetour;
    SubHook     luaCallDetour;
    SubHook     luaCloseDetour;

    /*
     * State Management
     */

    void
    add_active_state(lua_state* state)
    {
        activeStates.push_back(state);
    }

    void
    remove_active_state(lua_state* state)
    {
        activeStates.remove(state);
    }

    bool
    check_active_state(lua_state* state)
    {
        std::list<lua_state*>::iterator stateIterator;

        for (stateIterator = activeStates.begin();
             stateIterator != activeStates.end();
             ++stateIterator) // is ++operator implemented? I guess we'll find out
        {
            // is this a real pointer.
            // lol C++
            if (*stateIterator == state)
            {
                return true;
            }
        }

        return false;
    }

    void
    lua_newcall(lua_state* state, int args, int returns)
    {
    	int result = olua_pcall(state, args, returns, 0);
    	if (result != 0) {
    		size_t len;
    		Logging::Log(olua_tolstring(state, -1, &len), Logging::LOGGING_ERROR);
      }
    }

    int
    luaH_getcontents(lua_state* state, bool files)
    {
    	size_t len;
    	const char* dirc = olua_tolstring(state, 1, &len);
    	std::string dir(dirc, len);
    	std::vector<std::string> directories;

    	try
      {
    		directories = Files::GetDirectoryContents(dir, files);
  	  }
	    catch (int e)
      {
    		olua_pushboolean(state, false);
    		return 1;
	    }

  	  olua_createtable(state, 0, 0);

    	std::vector<std::string>::iterator it;
    	int index = 1;
    	for (it = directories.begin(); it < directories.end(); it++)
      {
    		if (*it == "." || *it == "..") continue;
    		olua_pushinteger(state, index);
    		olua_pushlstring(state, it->c_str(), it->length());
    		olua_settable(state, -3);
    		index++;
  	  }

      return 1;
    }

    int
    luaF_getdir(lua_state* state)
    {
    	return luaH_getcontents(state, true);
    }

    int
    luaF_getfiles(lua_state* state)
    {
    	return luaH_getcontents(state, false);
    }

    int
    luaF_directoryExists(lua_state* state)
    {
    	size_t len;
    	const char* dirc = olua_tolstring(state, 1, &len);
    	bool doesExist = Files::DirectoryExists(dirc);
    	olua_pushboolean(state, doesExist);
    	return 1;
    }

/*
    int
    luaF_unzipfile(lua_state* state)
    {
    	size_t len;
    	const char* archivePath = olua_tolstring(state, 1, &len);
    	const char* extractPath = olua_tolstring(state, 2, &len);

    	ZIPArchive* archive = new ZIPArchive(archivePath, extractPath);
    	archive->ReadArchive();
    	delete archive;
    	return 0;
    }
*/

    int luaF_removeDirectory(lua_state* state)
    {
    	size_t len;
    	const char* directory = olua_tolstring(state, 1, &len);
    	bool success = Files::RemoveEmptyDirectory(directory);
    	olua_pushboolean(state, success);
    	return 1;
    }

    int
    luaF_pcall(lua_state* state)
    {
    	int args = olua_gettop(state);

    	int result = olua_pcall(state, args - 1, -1, 0);
    	if (result == LUA_ERRRUN){
    		size_t len;
    		Logging::Log(olua_tolstring(state, -1, &len), Logging::LOGGING_ERROR);
    		return 0;
    	}
    	olua_pushboolean(state, result == 0);
    	olua_insert(state, 1);

    	//if (result != 0) return 1;

    	return olua_gettop(state);
    }

    int luaF_dofile(lua_state* state)
    {
    	int n = olua_gettop(state);

    	size_t length = 0;
    	const char* filename = olua_tolstring(state, 1, &length);
    	int error = oluaL_loadfile(state, filename);
    	if (error == LUA_ERRSYNTAX)
      {
    		size_t len;
    		Logging::Log(filename, Logging::LOGGING_ERROR);
    		Logging::Log(olua_tolstring(state, -1, &len), Logging::LOGGING_ERROR);
    	}
    	error = olua_pcall(state, 0, 0, 0);
    	if (error == LUA_ERRRUN)
      {
    		size_t len;
    		Logging::Log(filename, Logging::LOGGING_ERROR);
    		Logging::Log(olua_tolstring(state, -1, &len), Logging::LOGGING_ERROR);
    	}
    	return 0;
    }

/*
    struct lua_http_data {
    	int funcRef;
    	int progressRef;
    	int requestIdentifier;
    	lua_state* state;
    };

    void
    return_lua_http(void* data, std::string& urlcontents)
    {
    	lua_http_data* ourData = (lua_http_data*)data;

    	if (!check_active_state(ourData->state))
      {
    		delete ourData;
    		return;
    	}

    	lua_rawgeti(ourData->state, LUA_REGISTRYINDEX, ourData->funcRef);
    	lua_pushlstring(ourData->state, urlcontents.c_str(), urlcontents.length());
    	lua_pushinteger(ourData->state, ourData->requestIdentifier);
    	lua_pcall(ourData->state, 2, 0, 0);
    	luaL_unref(ourData->state, LUA_REGISTRYINDEX, ourData->funcRef);
    	luaL_unref(ourData->state, LUA_REGISTRYINDEX, ourData->progressRef);
    	delete ourData;
    }

    void
    progress_lua_http(void* data, long progress, long total)
    {
    	lua_http_data* ourData = (lua_http_data*)data;

    	if (!check_active_state(ourData->state))
      {
    		return;
    	}

    	if (ourData->progressRef == 0)
        return;

    	lua_rawgeti(ourData->state, LUA_REGISTRYINDEX, ourData->progressRef);
    	lua_pushinteger(ourData->state, ourData->requestIdentifier);
    	lua_pushinteger(ourData->state, progress);
    	lua_pushinteger(ourData->state, total);
    	lua_pcall(ourData->state, 3, 0, 0);
    }

    static int HTTPReqIdent = 0;

    int
    luaF_dohttpreq(lua_state* state)
    {
    	Logging::Log("Incoming HTTP Request/Request");

    	int args = lua_gettop(state);
    	int progressReference = 0;
    	if (args >= 3)
    		progressReference = luaL_ref(state, LUA_REGISTRYINDEX);

    	int functionReference = luaL_ref(state, LUA_REGISTRYINDEX);
    	size_t len;
    	const char* url_c = lua_tolstring(state, 1, &len);
    	std::string url = std::string(url_c, len);

    	Logging::Log(url);
    	Logging::Log(std::to_string(functionReference));

    	lua_http_data* ourData = new lua_http_data();
    	ourData->funcRef = functionReference;
    	ourData->progressRef = progressReference;
    	ourData->state = state;

    	HTTPReqIdent++;
    	ourData->requestIdentifier = HTTPReqIdent;

    	HTTPItem* reqItem = new HTTPItem();
    	reqItem->call = return_lua_http;
    	reqItem->data = ourData;
    	reqItem->url = url;

    	if (progressReference != 0)
    		reqItem->progress = progress_lua_http;


    	HTTPManager::GetSingleton()->LaunchHTTPRequest(reqItem);
    	lua_pushinteger(state, HTTPReqIdent);
    	return 1;
    }
*/
    /*
     * Detour Impl
     */

    void*
    dt_Application_update(void* parentThis)
    {
        SubHook::ScopedRemove remove(&gameUpdateDetour);
        //Logging::Log("dt_Application_update() called", Logging::LOGGING_LOG);

        return do_game_update(parentThis);
    }

    /*
     * lua_newstate (and thus, luaL_newstate) intercept
     */
    lua_state*
    dt_lua_newstate(lua_alloc allocator, void* data)
    {
        SubHook::ScopedRemove remove(&newStateDetour);
        lua_state* state = olua_newstate(allocator, data);
#       define lua_mapfn(name, function) \
          olua_pushcclosure(state, function, 0); \
          olua_setfield(state, LUA_GLOBALSINDEX, name);

        if (!state)
        {
            return state; // null anyways, but whatever.
        }

        add_active_state(state);

        int stackSize = olua_gettop(state);

        Logging::Log("stackSize = " + static_cast<std::ostringstream*>( &(std::ostringstream() << stackSize) )->str(), Logging::LOGGING_LOG);

        //Install BLT API-extensions in to the LUA context

        lua_mapfn("pcall",      luaF_pcall);
        lua_mapfn("dofile",     luaF_dofile);
      //  lua_mapfn("dohttpreq",  lapi::dohttpreq);
      //  lua_mapfn("log",        lapi::log);
      //  lua_mapfn("unzip",      lapi::unzip);


        luaL_reg fileLib[] = { { "GetDirectories", luaF_getdir }, { "GetFiles", luaF_getfiles }, { "RemoveDirectory", luaF_removeDirectory }, { "DirectoryExists", luaF_directoryExists }, { NULL, NULL } };
       	oluaL_openlib(state, "file", fileLib, 0);

       	int result;
       	Logging::Log("Initiating Hook", Logging::LOGGING_LOG);

       	result = oluaL_loadfile(state, "mods/base/base.lua");
       	if (result == LUA_ERRSYNTAX){
       		size_t len;
       		Logging::Log(olua_tolstring(state, -1, &len), Logging::LOGGING_ERROR);
       		return state;
       	}
       	result = olua_pcall(state, 0, 1, 0);
       	if (result == LUA_ERRRUN){
       		size_t len;
       		Logging::Log(olua_tolstring(state, -1, &len), Logging::LOGGING_ERROR);
       		return state;
       	}

       	olua_settop(state, stackSize);

        return state;
        #undef lua_mapfn
    }

    void
    InitLUAHooks(void* dlHandle)
    {
#       define setcall(symbol, ptr) *(void**) (&ptr) = dlsym(dlHandle, #symbol);\

        Logging::Log("setting up lua function access");

        /*
         * DL Init
         */

        {
          setcall(lua_call,           olua_call);
          setcall(lua_pcall,          olua_pcall);
          setcall(lua_gettop,         olua_gettop);
          setcall(lua_settop,         olua_settop);
          setcall(lua_tolstring,      olua_tolstring);
          setcall(luaL_loadfile,      oluaL_loadfile);
          setcall(lua_load,           olua_load);
          setcall(lua_setfield,       olua_setfield);
          setcall(lua_createtable,    olua_createtable);
          setcall(lua_insert,         olua_insert);
          setcall(lua_newstate,       olua_newstate);
          setcall(lua_close,          olua_close);
          setcall(lua_rawset,         olua_rawset);
          setcall(lua_settable,       olua_settable);
          setcall(lua_pushnumber,     olua_pushnumber);
          setcall(lua_pushinteger,    olua_pushinteger);
          setcall(lua_pushboolean,    olua_pushboolean);
          setcall(lua_pushcclosure,   olua_pushcclosure);
          setcall(lua_pushlstring,    olua_pushlstring);
          setcall(luaL_openlib,       oluaL_openlib);
          setcall(luaL_ref,           oluaL_ref);
          setcall(lua_rawgeti,        olua_rawgeti);
          setcall(luaL_unref,         oluaL_unref);
          setcall(luaL_newstate,      oluaL_newstate);

          setcall(_ZN11Application6updateEv, do_game_update); // _ZN11Application6updateEv = Application::update()

        }


        /*
         * Intercept Init
         */

        {
           // These function intercepts have a hidden pointer param for `this`
           gameUpdateDetour.Install((void *) do_game_update,    (void*) dt_Application_update);

           // These are proper C functions
           newStateDetour.Install((void *) olua_newstate,       (void*) dt_lua_newstate);
        }

#       undef setcall
    }

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
