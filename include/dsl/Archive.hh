#pragma once

namespace dsl {
   class DataStore;

   class Archive {
      public:
         Archive(std::string const&, dsl::DataStore*, uint64_t, uint64_t, bool, uint64_t);
         Archive(dsl::Archive const&, uint64_t, uint64_t, bool, uint64_t const&);
         Archive(dsl::Archive const&);
         ~Archive();

         Archive &operator=(dsl::Archive const&);

         // void set_position(long long, dsl::Archive::SeekMethod);
         void _chk_decompress();
         void checked_read_raw(unsigned char*, unsigned long);
         void checked_write_raw(unsigned char const*, unsigned long);
         void read_ln(std::string*);
         void read_str(std::string*);
         void size() const;
   };
};

/* vim: set ts=3 softtabstop=0 sw=3 expandtab: */
