#include <blt/lapi.hh>
#include <blt/hook.hh>
#include <blt/http.hh>
#include <blt/log.hh>

#include <cstddef> // size_t
#include <string>

namespace blt {
    namespace lapi {
        using std::string;

        struct LUAHttpData {
            int         funcRef;
            int         progressRef;
            int         reqIdent;
            lua_state*  state;
        };

        static int HTTPRequestIdent = 0; // why?

        /*
         * HTTP Callback Region
         */

        static void
        http_completion_cb(void* data, string contents)
        {
            LUAHttpData* httpData = (LUAHttpData*) data;

            // Ensure the Lua state is still valid before sending the http data
            if (check_active_state(httpData->state))
            {
                lua_rawgeti(httpData->state, LUARegistryIndex, httpData->funcRef);
                lua_pushlstring(httpData->state, contents.c_str(), contents.length());
                lua_pushinteger(httpData->state, httpData->reqIdent);
                lua_pcall(httpData->state, 2, 0, 0);
                luaL_unref(httpData->state, LUARegistryIndex, httpData->funcRef);
                luaL_unref(httpData->state, LUARegistryIndex, httpData->progressRef);
            }

            delete httpData;
        }

        static void
        http_progress_cb(void* data, long progress, long total)
        {
            LUAHttpData* httpData = (LUAHttpData*) data;

            // Ensure the Lua state is still valid before sending the http data
            if (check_active_state(httpData->state))
            {
                lua_rawgeti(httpData->state, LUARegistryIndex, httpData->progressRef);
                lua_pushinteger(httpData->state, httpData->reqIdent);
                lua_pushinteger(httpData->state, progress);
                lua_pushinteger(httpData->state, total);
                lua_pcall(httpData->state, 3, 0, 0);
            }
        }

        int
        dohttpreq(lua_state* state)
        {
            // XXX logging
            
            int args = lua_gettop(state);
            int progressRef = 0;

            if (args >= 3)
            {
                progressRef = luaL_ref(state, LUARegistryIndex);
            }

            int functionRef = luaL_ref(state, LUARegistryIndex);
            
            string url;
            {
                size_t urlLen;
                const char* urlCharArr = lua_tolstring(state, 1, &urlLen);
                url = string(urlCharArr, urlLen);
            }

            LUAHttpData* httpData = new LUAHttpData();
            {
                httpData->funcRef = functionRef;
                httpData->progressRef = progressRef;
                httpData->state = state;
                httpData->reqIdent = ++HTTPRequestIdent;
            }


            HTTPItem* reqItem = new HTTPItem();
            {
                reqItem->callback   = http_completion_cb;
                reqItem->data       = httpData;
                reqItem->url        = url;

                if (progressRef != 0)
                {
                    reqItem->progressCallback = http_progress_cb;
                }
            }

            log::log(string("Launching HTTP GET (dest: " + url + ")"), log::LOG_LUA);
            HTTPManager::get_instance()->launch_request(reqItem);
            lua_pushinteger(state, HTTPRequestIdent);
            return 1;
        }
    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

