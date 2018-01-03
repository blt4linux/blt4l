
#include <blt/assets.hh>

#if defined(BLT_USING_LIBCXX)

#include <dlfcn.h>

#include <dsl/FileSystem.hh>
#include <dsl/Archive.hh>
#include <dsl/DB.hh>
#include <dsl/Transport.hh>

#define hook_remove(hookName) SubHook::ScopedRemove _sh_remove_raii(&hookName)

using namespace std;
using namespace dsl;

namespace blt {

    typedef pair<idstring, idstring> hash_t;
    std::map<std::pair<idstring, idstring>, std::string> custom_assets;

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

#define EACH_HOOK(func) \
func(1); \
func(2); \
func(3); \
func(4);

    SubHook dslDbAddMembers; // We need this to add the entries, as we need to do it after the DB table is set up

    void (*dsl_db_add_members)   (lua_state*);
    void (*dsl_fss_open) (Archive *output, FileSystemStack **_this, std::string const*);

    typedef void* (*try_open_t) (Archive *target, DB *db, idstring *ext, idstring *name, void *misc, Transport *transport);
    typedef void* (*do_resolve_t) (DB *_this, idstring*, idstring*, void *abc, void *def);

#define HOOK_VARS(id) \
    try_open_t dsl_db_try_open_ ## id = NULL; \
    do_resolve_t dsl_db_do_resolve_ ## id = NULL; \
    SubHook dslDbTryOpenDetour ## id;

EACH_HOOK(HOOK_VARS)

    static void*
    dt_dsl_db_try_open_hook(Archive *target, DB* db, idstring *ext, idstring *name, void* misc_object, Transport* transport, try_open_t original, do_resolve_t resolve) {
        hash_t hash(*name, *ext);
        if(custom_assets.count(hash)) {
            string str = custom_assets[hash];
            dsl_fss_open(target, &db->stack, &str);
            return target;
        }

        return original(target, db, ext, name, misc_object, transport);

        /* // Code for doing the same thing as the original function (minus mod_override support):
        int result = resolve(
            db,
            ext,
            name,
            misc_object,
            (void*) ((char*)db->ptr4 + 40)
        );

        if ( result < 0 )
        {
            // Couldn't find the asset
            *target = Archive("", 0LL, 0LL, 0LL, 1, 0LL);
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

#define HOOK_TRY_OPEN(id) \
    static void* \
    dt_dsl_db_try_open_hook_ ## id(Archive *target, DB* db, idstring* ext, idstring* name, void* misc_object, Transport* transport) { \
        hook_remove(dslDbTryOpenDetour ## id); \
        return dt_dsl_db_try_open_hook(target, db, ext, name, misc_object, transport, dsl_db_try_open_ ## id, dsl_db_do_resolve_ ## id); \
    }

EACH_HOOK(HOOK_TRY_OPEN)

    static void dt_dsl_db_add_members(lua_state *L)
    {
        hook_remove(dslDbAddMembers);

        dsl_db_add_members(L);

        lapi::assets::setup(L);
    }

    void init_asset_hook(void *dlHandle)
    {
#define setcall(ptr, symbol) *(void**) (&ptr) = dlsym(dlHandle, #symbol); 
        setcall(dsl_db_add_members, _ZN3dsl6MainDB11add_membersEP9lua_State);

        setcall(dsl_db_try_open_1, _ZN3dsl2DB8try_openIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEENS_7ArchiveENS_8idstringESE_RKT_RKNS_9TransportE);
        setcall(dsl_db_try_open_2, _ZN3dsl2DB8try_openIN5sound15EnglishResolverEEENS_7ArchiveENS_8idstringES5_RKT_RKNS_9TransportE);
        setcall(dsl_db_try_open_3, _ZN3dsl2DB8try_openINS_16LanguageResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE);
        setcall(dsl_db_try_open_4, _ZN3dsl2DB8try_openINS_21PropertyMatchResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE);

        setcall(dsl_db_do_resolve_1, _ZNK3dsl2DB10do_resolveIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEEiNS_8idstringESD_RKT_PS9_);
        setcall(dsl_db_do_resolve_2, _ZNK3dsl2DB10do_resolveIN5sound15EnglishResolverEEEiNS_8idstringES4_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessISA_EENS_9AllocatorEEE);
        setcall(dsl_db_do_resolve_3, _ZNK3dsl2DB10do_resolveINS_16LanguageResolverEEEiNS_8idstringES3_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS9_EENS_9AllocatorEEE);
        setcall(dsl_db_do_resolve_4, _ZNK3dsl2DB10do_resolveINS_21PropertyMatchResolverEEEiNS_8idstringES3_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS9_EENS_9AllocatorEEE);

        setcall(dsl_fss_open, _ZNK3dsl15FileSystemStack4openERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE);
#undef setcall

        dslDbAddMembers.Install((void*) dsl_db_add_members, (void*) dt_dsl_db_add_members);

#define INSTALL_TRY_OPEN_HOOK(id) \
        dslDbTryOpenDetour ## id.Install((void*) dsl_db_try_open_ ## id, (void*) dt_dsl_db_try_open_hook_ ## id);
        EACH_HOOK(INSTALL_TRY_OPEN_HOOK)

    }
};

#else

// Stub implementation when not using LibC++
void blt::init_asset_hook(void *dlHandle) {}

#endif

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

