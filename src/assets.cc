
#include <blt/assets.hh>

#if defined(BLT_USING_LIBCXX)

#include <dlfcn.h>

#include <dsl/FileSystem.hh>
#include <dsl/Archive.hh>
#include <dsl/DB.hh>
#include <dsl/Transport.hh>

#define hook_remove(hookName) SubHook::ScopedRemove _sh_remove_raii(&hookName)

using namespace std;

namespace blt {

    typedef pair<dsl::idstring, dsl::idstring> hash_t;
    std::map<std::pair<dsl::idstring, dsl::idstring>, std::string> custom_assets;

    // LAPI stuff

    namespace lapi { namespace assets {
        int create_entry(lua_state *L) {
            uint64_t extension = *(uint64_t*) lua_touserdata(L, 2);
            uint64_t name = *(uint64_t*) lua_touserdata(L, 3);
            hash_t hash(name, extension);

            size_t len;
            const char *filename_c = luaL_checklstring(L, 4, &len);
            string filename(filename_c, len);

            if(custom_assets.count(hash)) {
                // Appears this is actually allowed in the basegame
                // char buff[1024];
                // snprintf(buff, 1024, "File already exists in replacement DB! %lx.%lx (%s)", name, extension, filename.c_str());
                // luaL_error(L, buff);
            }

            custom_assets[hash] = filename;
            return 0;
        }

        void setup(lua_state *L)
        {
#define func(name) \
                lua_pushcclosure(L, name, 0); \
                lua_setfield(L, -2, #name);

            func(create_entry);

#undef func
        }
    }; };


    // The acual function hooks

    SubHook dslDbAddMembers; // We need this to add the entries, as we need to do it after the DB table is set up

    SubHook dslDbTryOpenDetour;
    SubHook dslDbTryOpenDetour2;

    void (*dsl_db_add_members)   (lua_state*);

    void* (*dsl_db_try_open)   (void* /* this */, void* name, void* ext, void* target, void* transport, void* z);
    void* (*dsl_db_try_open_2)   (void* /* this */, void* name, void* ext, void* target, void* transport, void* z);

    int (*dsl_db_do_resolve_1) (dsl::DB *_this, dsl::idstring*, dsl::idstring*, void *abc, void *def);
    void (*dsl_fss_open) (dsl::Archive *output, dsl::FileSystemStack **_this, std::string const*);

    static void*
    dt_dsl_db_try_open(void* _this, void* name, void* ext, void* target, void* transport, void* z) {
        hook_remove(dslDbTryOpenDetour);

        //printf("Hello, World: %ld %lx.%lx %ld\n", *((uint64_t*) name), *((uint64_t*) target), *((uint64_t*) ext), sizeof(std::string));

        return dsl_db_try_open(_this, name, ext, target, transport, z);
    }

    static void*
    dt_dsl_db_try_open_2(dsl::Archive *target, dsl::DB* db, dsl::idstring* ext, dsl::idstring* name, void* misc_object, dsl::Transport* transport) {
        hook_remove(dslDbTryOpenDetour2);

        hash_t hash(*name, *ext);
        if(custom_assets.count(hash)) {
            string str = custom_assets[hash];
            dsl_fss_open(target, &db->stack, &str);
            return target;
        }

        return dsl_db_try_open_2(target, db, ext, name, misc_object, transport);

        /* // Code for doing the same thing as the original function (minus mod_override support):
        int result = dsl_db_do_resolve_1(
            db,
            ext,
            name,
            misc_object,
            (void*) ((char*)db->ptr4 + 40)
        );

        if ( result < 0 )
        {
            // Couldn't find the asset
            *target = dsl::Archive("", 0LL, 0LL, 0LL, 1, 0LL);
        }
        else
        {
            char *ptr = (char*) db;
    #define add_and_dereference(value) ptr = *(char**)(ptr + value)
            add_and_dereference(80);
            add_and_dereference(56);
    #undef add_and_dereference
            unsigned int val = *(unsigned int*) (ptr + 24 + result * 32);
            transport->vt->f_open(target, transport, val);
        }
        return target;
        */
    }

    static void dt_dsl_db_add_members(lua_state *L)
    {
        hook_remove(dslDbAddMembers);

        dsl_db_add_members(L);

        lapi::assets::setup(L);
    }

    void init_asset_hook(void *dlHandle)
    {
#define setcall(symbol,ptr) *(void**) (&ptr) = dlsym(dlHandle, #symbol); 
        setcall(_ZN3dsl6MainDB11add_membersEP9lua_State, dsl_db_add_members);

        setcall(_ZN3dsl2DB8try_openINS_16LanguageResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE, dsl_db_try_open);
        setcall(_ZN3dsl2DB8try_openIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEENS_7ArchiveENS_8idstringESE_RKT_RKNS_9TransportE, dsl_db_try_open_2);

        setcall(_ZNK3dsl2DB10do_resolveIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEEiNS_8idstringESD_RKT_PS9_, dsl_db_do_resolve_1);
        setcall(_ZNK3dsl15FileSystemStack4openERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE, dsl_fss_open);
#undef setcall

        dslDbAddMembers.Install((void*) dsl_db_add_members, (void*) dt_dsl_db_add_members);
        dslDbTryOpenDetour.Install((void*) dsl_db_try_open, (void*) dt_dsl_db_try_open);
        dslDbTryOpenDetour2.Install((void*) dsl_db_try_open_2, (void*) dt_dsl_db_try_open_2);

    }
};

#else

// Stub implementation when not using LibC++
void blt::init_asset_hook(void *dlHandle) {}

#endif

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

