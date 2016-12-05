#include <blt/fs.hh>
#include <blt/log.hh>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <ftw.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <streambuf>

// PATH_MAX
#include <limits.h>

namespace blt {
    namespace fs {

        using std::string;
        using std::to_string;
        using std::stringstream;
        using std::vector;
        using std::ifstream;

        /**
         * Compute the contents of a directory and return them in vector
         */
        vector<string>
        list_directory(string path, bool listDirs)
        {

            vector<string> result;
            
            DIR* directory = opendir(path.c_str());


            if (directory)
            {
                struct dirent* next = readdir(directory);

                while (next)
                {
                    bool isDir;
                    {
                        if(next->d_type == DT_LNK || next->d_type == DT_UNKNOWN)
                        {
                            char current_path[PATH_MAX + 1];
                            snprintf(current_path, PATH_MAX, "%s/%s", path.c_str(), next->d_name);

                            // this assumes that stat will dereference N-deep symlinks rather than single symlinks
                            struct stat estat;
                            stat(current_path, &estat);

                            isDir = S_ISDIR(estat.st_mode);
                        }
                        else 
                        {
                            isDir = (next->d_type == DT_DIR);
                        }
                    }

                    if ((listDirs && isDir) || (!listDirs && !isDir))
                    {
                        result.push_back(next->d_name);
                    }

                    next = readdir(directory);
                }

                closedir(directory);
            }


            return result;
        }

        /**
         * Read the contents of a file and return them
         */
        string
        read_file(string path)
        {
            ifstream istream(path);
            string body;

            istream.seekg(0, std::ios::end);
            body.reserve(istream.tellg());
            istream.seekg(0, std::ios::beg);
            body.assign(std::istreambuf_iterator<char>(istream), 
                        std::istreambuf_iterator<char>());

            return body;
        }

        /**
         * Check if a path exists, and is a directory.
         */
        bool
        path_is_dir(string path)
        {
            struct stat fileInfo;
            
            if (stat(path.c_str(), &fileInfo) == 0)
            {
                return S_ISDIR(fileInfo.st_mode);
            }

            return false;
        }


        /**
         * Linux mkdirs() approximation
         */
        bool
        create_directory(string path)
        {
            {
                string seg;
                stringstream pathBuilder;
                stringstream pathStream(path);
                while (std::getline(pathStream, seg, '/'))
                {
                    if (!seg.empty()) // should break on this instead of continuing...
                    {
                        pathBuilder << seg + '/';
                        mkdir(pathBuilder.str().c_str(), 0755);
                    }
                }
            }    

            return true; // should check return value of mkdir()
        }

        bool 
        create_file_parent(string path)
        {
            string parent;
            {
                int finalSlash = path.find_last_of('/');
                parent = path.substr(0, finalSlash);
            }


            if (path_is_dir(parent)) 
            {
                return true;
            }

            return create_directory(parent);
        }



        /*
         * delete_directory
         */
		int
        fs_delete_dir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
        {
            return remove( fpath );
        }

        bool
        delete_directory(string path, bool descend)
        {
            if (!descend)
            {
                return remove(path.c_str());
            }
            else
            {
                return nftw(path.c_str(), fs_delete_dir, /* max open fd's */ 128, FTW_DEPTH | FTW_PHYS);
            }
        }

    }
}

