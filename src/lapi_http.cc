#include <blt/lapi.hh>
#include <blt/hook.hh>
#include <blt/http.hh>
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
                httpData->reqIdent = HTTPRequestIdent++;
            }


            HTTPItem* reqItem = new HTTPItem();
            {
                reqItem->callback   = http_completion_cb;
                reqItem->data       = httpData;
                reqItem->url        = url;
            }

            HTTPManager::get_instance()->launch_request(reqItem);
            lua_pushinteger(state, HTTPRequestIdent);
            return 1;
        }
    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

