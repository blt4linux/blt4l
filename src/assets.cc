
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

    typedef pair<idstring_t, idstring_t> hash_t;
    static std::map<hash_t, std::string> custom_assets;

    // LAPI stuff

    namespace lapi { namespace assets {
        int create_entry(lua_state *L)
        {
            idstring *extension = (idstring*) lua_touserdata(L, 2);
            idstring *name = (idstring*) lua_touserdata(L, 3);
            hash_t hash(name->value, extension->value);

            size_t len;
            const char *filename_c = luaL_checklstring(L, 4, &len);
            string filename(filename_c, len);

            if(custom_assets.count(hash))
            {
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

    // EACH_HOOK(func): Run a function with an argument ranging from 1 to 4
    // This corresponds to the four usages of templates in the PD2 loading functions
#define EACH_HOOK(func) \
func(1); \
func(2); \
func(3); \
func(4);

    static SubHook dslDbAddMembers; // We need this to add the entries, as we need to do it after the DB table is set up

    static void (*dsl_db_add_members)   (lua_state*);
    static void (*dsl_fss_open) (Archive *output, FileSystemStack **_this, std::string const*);

    typedef void* (*try_open_t) (Archive *target, DB *db, idstring *ext, idstring *name, void *template_obj /* Misc depends on the template type */ , Transport *transport);
    typedef void* (*do_resolve_t) (DB *_this, idstring*, idstring*, void *template_obj, void *unknown);

    // Create variables for each of the functions
    // A detour, and a pointer to the correspoinding try_open and do_resolve functions
#define HOOK_VARS(id) \
    static try_open_t dsl_db_try_open_ ## id = NULL; \
    static do_resolve_t dsl_db_do_resolve_ ## id = NULL; \
    static SubHook dslDbTryOpenDetour ## id;

EACH_HOOK(HOOK_VARS)

    // A generic hook function
    // This can be used with all four of the template values, and it passes everything through to the supplied original function if nothing has changed.
    static void*
    dt_dsl_db_try_open_hook(Archive *target, DB* db, idstring *ext, idstring *name, void* misc_object, Transport* transport, try_open_t original, do_resolve_t resolve)
    {
        hash_t hash(name->value, ext->value);
        if(custom_assets.count(hash))
        {
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

// Create hook functions for each of the original functions
// These just call the hook function above, passing in the correct function values
#define HOOK_TRY_OPEN(id) \
    static void* \
    dt_dsl_db_try_open_hook_ ## id(Archive *target, DB* db, idstring* ext, idstring* name, void* misc_object, Transport* transport) \
    { \
        hook_remove(dslDbTryOpenDetour ## id); \
        return dt_dsl_db_try_open_hook(target, db, ext, name, misc_object, transport, dsl_db_try_open_ ## id, dsl_db_do_resolve_ ## id); \
    }
EACH_HOOK(HOOK_TRY_OPEN)

    // When the members are being added to the DB table, add our own in
    static void dt_dsl_db_add_members(lua_state *L)
    {
        hook_remove(dslDbAddMembers);

        // Make sure we do ours first, so they get overwritten if the basegame
        // implements them in the future
        lapi::assets::setup(L);

        dsl_db_add_members(L);
    }

    // Initialiser function, called by hook.cc
    void init_asset_hook(void *dlHandle)
    {
#define setcall(ptr, symbol) *(void**) (&ptr) = dlsym(dlHandle, #symbol); 

        // Get the try_open functions
        setcall(dsl_db_try_open_1, _ZN3dsl2DB8try_openIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEENS_7ArchiveENS_8idstringESE_RKT_RKNS_9TransportE);
        setcall(dsl_db_try_open_2, _ZN3dsl2DB8try_openIN5sound15EnglishResolverEEENS_7ArchiveENS_8idstringES5_RKT_RKNS_9TransportE);
        setcall(dsl_db_try_open_3, _ZN3dsl2DB8try_openINS_16LanguageResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE);
        setcall(dsl_db_try_open_4, _ZN3dsl2DB8try_openINS_21PropertyMatchResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE);

        // Ge tthe do_resolve functions
        setcall(dsl_db_do_resolve_1, _ZNK3dsl2DB10do_resolveIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEEiNS_8idstringESD_RKT_PS9_);
        setcall(dsl_db_do_resolve_2, _ZNK3dsl2DB10do_resolveIN5sound15EnglishResolverEEEiNS_8idstringES4_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessISA_EENS_9AllocatorEEE);
        setcall(dsl_db_do_resolve_3, _ZNK3dsl2DB10do_resolveINS_16LanguageResolverEEEiNS_8idstringES3_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS9_EENS_9AllocatorEEE);
        setcall(dsl_db_do_resolve_4, _ZNK3dsl2DB10do_resolveINS_21PropertyMatchResolverEEEiNS_8idstringES3_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS9_EENS_9AllocatorEEE);

        // The misc. functions
        setcall(dsl_db_add_members, _ZN3dsl6MainDB11add_membersEP9lua_State);
        setcall(dsl_fss_open, _ZNK3dsl15FileSystemStack4openERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE);
#undef setcall

        // Add the 'add_members' hook
        dslDbAddMembers.Install((void*) dsl_db_add_members, (void*) dt_dsl_db_add_members);

        // Hook each of the four loading functions
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

