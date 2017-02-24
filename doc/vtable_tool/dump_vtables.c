/**
 * Terrible, terrible, terrible, vmt dumper for non-PIC executables. Or anything in the address space, really.
 * 
 * Some portions of (paraphrased) code (C) 2013-2016 Andrey Ponomarenko
 *
 * No rights reserved, 2017 Roman Hargrave <roman@hargrave.info>
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stddef.h>
#include <string.h>

// NOTE Ripped from vtable-dumper
// type vdump_VTable_C1
// type vdump_VTable_C2
// type vdump_VTable_U
typedef void* vdump_FunctionPointer;
struct vdump_VTable_C1 {
    uint64_t                baseoffset;
    const char*             typeinfo;
    vdump_FunctionPointer   virtualFuncs[0];
};

struct vdump_VTable_C2 {
    uint64_t                vcalloffset;
    uint64_t                baseoffset;
    const char*             typeinfo;
    vdump_FunctionPointer   virtualFuncs[0];
};

union vdump_VTable_U {
    struct vdump_VTable_C1 const cat1;
    struct vdump_VTable_C2 const cat2;
};
// end 

void
printvmt(void* dlh, void* ptr, uint64_t len)
{
    union vdump_VTable_U* table = (union vdump_VTable_U*) ptr;

    uint64_t    vmt_baseOffset = table->cat1.baseoffset;
    const char* vmt_typeInfo   = table->cat1.typeinfo;
    uint64_t*   vmt_functions  = (uint64_t*) table->cat1.virtualFuncs;
    size_t      vmt_fnCount    = len / sizeof(ptrdiff_t);

    size_t      vmt_readoffset = 0;
    size_t      vmt_readstep   = sizeof(ptrdiff_t);

    printf("    ... entries:    %lu\n",     
            vmt_fnCount);
    /* printf("    ... typeinfo:   %s\n",       */
    /*         vmt_typeInfo); */

    if (vmt_baseOffset != 0)
    {
        printf("%d      %luu\n", 0, vmt_baseOffset);
    }
    else
    {
        printf("0       (int (*)(...)) 0\n");
    }

    Dl_info dla_info;

    vmt_readoffset += vmt_readstep;
    if (dladdr(vmt_typeInfo, &dla_info))
    {
        printf("%d      (int (*)(...)) (& %s)\n", vmt_readoffset, dla_info.dli_sname);
    }
    else
    {
        if (vmt_baseOffset != 0)
        {
            printf("%d      0u\n", vmt_readoffset);
        }
        else
        {
            printf("%d      (int (*)(...)) 0\n", vmt_readoffset);
        }
    }

    for (size_t fnIndex = 0; fnIndex < vmt_fnCount - 3; ++fnIndex)
    {
        vmt_readoffset += vmt_readstep;
        /* memset(&dla_info, 0, sizeof(dla_info)); */


        void* fsym = vmt_functions[fnIndex];
        
        if (dladdr(fsym, &dla_info))
        {
            printf("%d      ", vmt_readoffset);
            if (dla_info.dli_sname == NULL)
            {
                printf("(int (*)(...)) %p\n", (void*) (((ptrdiff_t) fsym) - ((ptrdiff_t) dla_info.dli_fbase)));
            }
            else if (strstr(dla_info.dli_sname, "__cxa_pure"))
            {
                printf("(int (*)(...)) %s\n", dla_info.dli_sname);
            }
            else /* if (strstr(dla_info.dli_sname, "_ZTI")) */
            {
                printf("(int (*)(...)) (& %s)\n", dla_info.dli_sname);
            }
        }
        else 
        {
            if (fsym == NULL)
            {
                printf("%d      0u", vmt_readoffset);
            }
            else
            {
                if (((ptrdiff_t) fsym) < 0)
                {
                    printf("(int (*)(...)) -%016p\n", (void*) (-((ptrdiff_t) fsym)));
                }
                else
                {
                    printf("(int (*)(...)) %016p\n", fsym);
                }
            }
        }

    }
}

__attribute__((constructor)) void
_test (void)
{
    FILE* symlist = fopen("/tmp/symlist", "r");
    void* dlHandle = dlopen(NULL, RTLD_LAZY);  

    uint64_t size = 0;
    char* current = NULL;
    int   ret = 0;
    while ((ret = fscanf(symlist, "%lu %ms\n", &size, &current)) != EOF)
    {
        if (current)
        {
            void* sym = dlsym(dlHandle, current);
            printf("sym %s = %p (size = %lu)\n", current, sym, size);
            if (sym)
            {
                printvmt(dlHandle, sym, size);
                printf("\n");
            }
            free(current);
        }
    }
   exit(1);
}
