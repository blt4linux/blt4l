#pragma once
#include <string>
#include <vector>
namespace dsl {
   using std::vector;

   class FileSystem {
      public:
         /**
          * Does what it says on the tin
          */
         static void delete_recursive(std::string const&);

         /**
          * My guess is that this is like Glob, based on the single_match function.
          * First param would be path, second would be pattern.
          *
          * XXX disassembly looks like these are actually reversed?
          */
         static vector<char const*> match(std::string const&, std::string const&);

         /**
          * Similar to match, but returns a single result, probably the first file?
          */
         static std::string single_match(std::string const&, std::string const&);

      public:
         virtual bool exists_ex(std::string const&) const;
         void update_index(std::string const&);
   };

   /**
    * Only known FileSystem implementation (guessed by name...)
    */
   class DiskFileSystem {
      public:
         int8_t ___padding0[1]; // 8 bytes unknown space

         std::string base_path; // this+8

      public:
         void set_base_path(std::string const&);
         void list_all(vector<std::string>*, vector<std::string>*, std::string const&) const;
   };

   class FileSystemStack {
      public:
         void* get_top_fs(void) const;
         void* get_bottom_fs(void) const;

         /**
          * Add filesystem to the stack,
          * Numeric param is probably priority
          */
         void add_fs(dsl::FileSystem*, unsigned int const&);
   };
}
