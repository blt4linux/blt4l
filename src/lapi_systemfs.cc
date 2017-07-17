/**
 * SystemFS lib implementation (patch for missing SystemFS, which was added in the workshop update)
 */

extern "C" {
#   include <fcntl.h>
#   include <stdio.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <sys/sendfile.h>
#   include <unistd.h>
}

#include <cstdlib>
#include <cstring>
#include <lua.hh>
#include <blt/lapi.hh>
#include <blt/hook.hh>

namespace blt {
   namespace lapi {
      namespace SystemFS {

         int
         l_exists(lua_state* state)
         {
            // assuming PWD is base folder
            const char* path = lua_tolstring(state, 1, NULL);
            struct stat _stat;
            lua_pushboolean(state, stat(path, &_stat));
            return 1;
         }

         int
         l_delete_file(lua_state* state)
         {
            return blt::lapi::removedir(state);
         }

         int
         l_make_dir(lua_state* state)
         {
            return blt::lapi::createdir(state);
         }

         int
         l_is_dir(lua_state* state)
         {
            return blt::lapi::dir_exists(state);
         }

         int
         l_copy_file(lua_state* state)
         {
            char const* from = lua_tolstring(state, 1, NULL);
            char const* to = lua_tolstring(state, 2, NULL);
            int from_fd, to_fd;

            if ((from_fd = open(from, O_RDONLY)) == -1)
            {
               lua_pushboolean(state, false);
               return 1;
            }

            if((to_fd = open(to, O_RDWR | O_CREAT)) == -1)
            {
               close(from_fd);
               lua_pushboolean(state, false);
               return 1;
            }

            struct stat _stat;
            fstat(from_fd, &_stat);
            ssize_t written = sendfile(from_fd, to_fd, NULL, _stat.st_size);

            close(from_fd);
            close(to_fd);

            lua_pushboolean(state, written > -1);

            return 1;
         }

         /**
          * TODO implement this.
          * Needs:
          * * own thread, calls back in to lvm (like http)
          * * copy each pair in a list of pairs
          */
         int
         l_copy_files_async(lua_state* state)
         {
            return 0;
         }

         // SystemFS file handles

         class FileHandle {
            public:
               constexpr static char const* _L_MT_NAME = "MT_BLT_SystemFS_FileObj";

            public:
               FILE* fp;
               size_t len;

               // side effecting constructors lol
               FileHandle(FILE* _fp)
                  : fp(_fp)
               {
                  fseek(fp, 0L, SEEK_END);
                  len = ftell(fp);
                  fseek(fp, 0L, SEEK_SET);
               }

               ~FileHandle()
               {
                  this->close();
               }

               /**
                * Appears to return a char array (for lua) containing the file contents
                */
               char const*
               read(void)
               {
                  if (this->fp)
                  {
                     // Assuming Lua won't free, I'm unsure though
                     char* contents = (char*) malloc(len * sizeof(char));
                     fread(contents, len, sizeof(char), fp);
                     return contents;
                  }
                  else
                  {
                     return NULL;
                  }
               }

               char const*
               gets(void)
               {
                  char* line = (char*) malloc(65536 * sizeof(char));
                  if (fgets(line, 65536, this->fp) == line)
                  {
                     char const* newLine = (char const*) realloc(line, strlen(line));
                     if (newLine)
                     {
                        return newLine;
                     }
                     else
                     {
                        free(line);
                        return "";
                     }
                  }
                  else
                  {
                     free(line);
                     return "";
                  }
               }

               void
               write(char const* contents)
               {
                  if (this->fp && contents)
                  {
                     fputs(contents, this->fp);
                  }
               }

               void
               puts(char const* line)
               {
                  if(this->fp && line)
                  {
                     fputs(line, this->fp);
                     fputc('\n', this->fp);
                  }
               }

               void
               close(void)
               {
                  if (this->fp)
                  {
                     fclose(this->fp);
                     this->fp = NULL;
                  }
               }

            // Lua ------------------------------------------------------------

            private:

               static int
               l_read(lua_state* state)
               {
                  FileHandle* _this = static_cast<FileHandle*>(luaL_checkudata(state, 1, _L_MT_NAME));
                  char const* contents = _this->read();
                  if (contents)
                  {
                     lua_pushstring(state, contents);
                     free((void*) contents);
                  }
                  else
                  {
                     // XXX silent failure
                     lua_pushstring(state, "");
                  }

                  return 1;
               }

               static int
               l_gets(lua_state* state)
               {
                  FileHandle* _this = static_cast<FileHandle*>(luaL_checkudata(state, 1, _L_MT_NAME));
                  char const* contents = _this->gets();
                  lua_pushstring(state, contents);
                  free((void*) contents);
                  return 1;
               }

               static int
               l_write(lua_state* state)
               {
                  FileHandle* _this = static_cast<FileHandle*>(luaL_checkudata(state, 1, _L_MT_NAME));
                  char const* contents = lua_tolstring(state, 2, NULL);
                  if (contents)
                  {
                     _this->write(contents);
                  }

                  return 0;
               }

               static int
               l_puts(lua_state* state)
               {
                  FileHandle* _this = static_cast<FileHandle*>(luaL_checkudata(state, 1, _L_MT_NAME));
                  char const* contents = lua_tolstring(state, 2, NULL);
                  if (contents)
                  {
                     _this->write(contents);
                  }

                  return 0;
               }

               static int
               l_close(lua_state* state)
               {
                  FileHandle* _this = static_cast<FileHandle*>(luaL_checkudata(state, 1, _L_MT_NAME));
                  _this->close();
                  return 0;
               }

               static int
               l_gc(lua_state* state)
               {
                  return l_close(state);
               }

            public:
               static void
               l_register_type(lua_state* s)
               {
                  luaL_newmetatable(s, _L_MT_NAME);

                  luaL_Reg fns[] = {
                     { "read",      FileHandle::l_read  },
                     { "gets",      FileHandle::l_gets  },
                     { "write",     FileHandle::l_write },
                     { "print",     FileHandle::l_write },
                     { "puts",      FileHandle::l_puts  },
                     { "close",     FileHandle::l_close },
                     { "__gc",      FileHandle::l_gc    },
                     { NULL, NULL }
                  };
                  luaL_register(s, NULL, fns);

                  lua_pushvalue(s, -1);
                  lua_setfield(s, -1, "__index");
               }
         };

         int
         l_open(lua_state* state)
         {
            char const* filename = lua_tolstring(state, 1, NULL);
            char const* mode = lua_tolstring(state, 2, NULL);

            FILE* filep = fopen(filename, mode);

            if (filep)
            {
               FileHandle* udfh = static_cast<FileHandle*>(lua_newuserdata(state, sizeof(FileHandle)));
               *udfh = FileHandle(filep);
               luaL_getmetatable(state, FileHandle::_L_MT_NAME);
               lua_setmetatable(state, -2);
            }

            return 1;
         }

         int
         l_close(lua_state* state)
         {
            FileHandle* fh = static_cast<FileHandle*>(luaL_checkudata(state, 1, FileHandle::_L_MT_NAME));
            if (fh)
            {
               fh->close();
            }
            return 0;
         }


         void
         _configure_lua(lua_state* state)
         {
            FileHandle::l_register_type(state);

            luaL_Reg lib_SystemFS[] = {
                { "exists",         l_exists        },
                { "delete_file",    l_delete_file   },
                { "make_dir",       l_make_dir      },
                { "is_dir",         l_is_dir        },
                { "copy_file",      l_copy_file     },
                { "open",           l_open          },
                { "close",          l_close         },
                { NULL, NULL }
            };
            luaL_openlib(state, "SystemFS", lib_SystemFS, 0);
         }
      }
   }
}
