#pragma once

#include <dsl/idstring.hh>

namespace dsl {
   struct TransportVT;

   class Transport {
      public:
         // VTable
         struct TransportVT *vt;

         // Variables
   };

   struct TransportVT { // VTable
      void *f1;
      void *f2;
      typedef void (*open_func)(dsl::Archive*, dsl::Transport*, unsigned int);
      open_func f_open;
   };

};

/* vim: set ts=3 softtabstop=0 sw=3 expandtab: */
