/**
 * Terrible, terrible, terrible, vmt dumper for non-PIC executables. Or anything in the address space, really.
 *
 * _If you can LD_PRELOAD it, you can dump it!_
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

#include <assert.h>

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

   if (vmt_fnCount > 0)
   {
      printf("    ... entries:    %lu\n",     
            vmt_fnCount);
   }
   else
   {
      printf("    ... empty (!)\n");
      return;
   }

   // Special consideration entries

   /* printf("    ... typeinfo:   %s\n",       */
   /*         vmt_typeInfo); */

   // Special entry 0
   if (vmt_baseOffset != 0)
   {
      printf("%-4d\t%luu\n", 0, vmt_baseOffset);
   }
   else
   {
      printf("%-4d\t(int (*)(...)) 0\n", 0);
   }

   if (vmt_fnCount < 2) return;

   Dl_info dla_info;

   // Special entry 1
   vmt_readoffset += vmt_readstep;
   if (dladdr(vmt_typeInfo, &dla_info))
   {
      printf("%-4d\t(int (*)(...)) (& %s)\n", vmt_readoffset, dla_info.dli_sname);
   }
   else
   {
      if (vmt_baseOffset != 0)
      {
         printf("%-4d\t0u\n", vmt_readoffset);
      }
      else
      {
         printf("%-4d\t(int (*)(...)) 0\n", vmt_readoffset);
      }
   }

   if (vmt_fnCount < 3) return;

   // Regular entries (2 +)

   for (size_t fnIndex = 0; fnIndex < vmt_fnCount - 3; ++fnIndex)
   {
      vmt_readoffset += vmt_readstep;
      assert(vmt_readoffset <= len);


      void* fsym = vmt_functions[fnIndex];

      printf("%-4d\t", vmt_readoffset);

      if (dladdr(fsym, &dla_info))
      {
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
            printf("0u");
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
   char* symlist_path = getenv("VTDUMP_SYMLIST");
   if (symlist_path)
   {
      FILE* symlist = fopen(symlist_path, "r");
      void* dlHandle = dlopen(NULL, RTLD_LAZY);

      uint64_t size = 0;
      char* current = NULL;
      int   ret = 0;
      while ((ret = fscanf(symlist, "%lx %ms\n", &size, &current)) != EOF)
      {
         if (current)
         {
            void* sym = dlsym(dlHandle, current);
            if (sym)
            {
               printf("sym %s = %p (size = %lu)\n", current, sym, size);
               printvmt(dlHandle, sym, size);
               printf("\n");
            }
            else
            {
               fprintf(stderr, "W: dlsym returned null for sym `%s` (%lu) \n",
                        current, size);
            }
            free(current);
         }
         else
         {
            fprintf(stderr, "W: null symname, but no EOF yet! check your formatting.\n");
         }
      }
      exit(0);
   }
   else
   {
      fprintf(stderr, "No symbol list file defined in VTDUMP_SYMLIST");
      exit(1);
   }
}
