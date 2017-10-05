#include "blt/error.hh"

#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "blt/log.hh"

namespace blt {
    namespace error {
        using std::string;
        using std::endl;

        std::map<lua_state*, int> refs;

        // Based off the debug.traceback function
        static void traceback (lua_state *L, void (print)(string))
        {
            lua_Debug ar;

            int level = 1;

            print("stack traceback:");
            std::stringstream buff;
            while (lua_getstack(L, level++, &ar))
            {
                buff << "\t";

                lua_getinfo(L, "Snl", &ar);
                buff << ar.short_src << ":";

                if (ar.currentline > 0)
                    buff << ar.currentline << ":";

                if (*ar.namewhat != '\0')  /* is there a name? */
                    buff << " in function '" << ar.name << "'";
                else
                {
                    if (*ar.what == 'm')  /* main? */
                        buff << " in main chunk";
                    else if (*ar.what == 'C' || *ar.what == 't')
                        buff << " ?";  /* C function or tail call */
                    else
                        buff << " in function <" << ar.short_src << ":" << ar.linedefined << ">";
                }

                print(buff.str());
                buff.str("");
            }
        }

        static void errlog(string str)
        {
            log::log(str, log::LOG_ERROR);
        }

        std::ofstream* crashstream;
        static void crashlog(string str)
        {
            *crashstream << str << endl;
        }

        static int error(lua_state* L)
        {
            size_t len;

            const char* crash_mode = getenv("BLT_CRASH");
            if(crash_mode == NULL || string(crash_mode) != "CONTINUE")
            {
                std::ofstream info("mods/logs/crash.txt");
                info << "Lua runtime error: " << lua_tolstring(L, 1, &len) << endl;
                info << endl;
                crashstream = &info;
                traceback(L, crashlog);
                info.close();

                exit(1); // Does not return
            }

            log::log("lua_call: error in lua_pcall: " + string(lua_tolstring(L, 1, &len)), log::LOG_ERROR);
            traceback(L, errlog);
            log::log("End of stack trace\n", log::LOG_ERROR);

            return 0;
        }


        int check_callback(lua_state* state)
        {
            if(refs.count(state))
                return refs[state];

            lua_pushcclosure(state, &error, 0);
            int ref = luaL_ref(state, LUARegistryIndex);
            log::log("Type: " + std::to_string(lua_type(state, -1)));
            log::log("ref: " + std::to_string(ref));

            refs[state] = ref;

            return ref;
        }
    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

