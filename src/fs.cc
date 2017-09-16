#include <blt/fs.hh>
#include <blt/log.hh>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <ftw.h>
#include <string.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iomanip>
#include <stack>
#include <algorithm>

#include <openssl/sha.h>

// PATH_MAX
#include <limits.h>

namespace blt {
    namespace fs {

        using std::string;
        using std::to_string;
        using std::stringstream;
        using std::vector;
        using std::stack;
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

        string
        hash_string(string contents)
        {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, contents.c_str(), contents.size());
            SHA256_Final(hash, &sha256);
            stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }
            return ss.str();
        }

        string
        hash_file(string filename)
        {
            struct stat fileInfo;

            if (stat(filename.c_str(), &fileInfo))
            {
                throw string(strerror(errno));
            }

            if(!S_ISDIR(fileInfo.st_mode))
            {
                // Hash it twice, same as BLT windows
                // This means you always get the same result, be it
                // a single file or a file in a directory.
                return hash_string(hash_string(read_file(filename)));
            }

            if(filename.back() == '/')
                filename.erase(filename.length() - 1);

            stack<string> todo;
            vector<string> files;
            todo.push(filename);

            DIR *dp;
            struct dirent *ep;

            while(!todo.empty())
            {
                string base = todo.top();
                dp = opendir(base.c_str());
                todo.pop();

                if (dp == NULL)
                    throw "Couldn't open the directory";

                while ((ep = readdir (dp)))
                {
                    string fname = ep->d_name;
                    if(fname == "." || fname == "..") continue;

                    string name = base + "/" + fname;

                    if(ep->d_type == DT_DIR)
                        todo.push(name);
                    else if(ep->d_type == DT_REG)
                        files.push_back(name);
                }
                closedir (dp);
            }

            std::sort(files.begin(), files.end());
            string result;

            for(string & file : files)
            {
                result += hash_string(read_file(file));
            }

            result = hash_string(result);

            return result;
        }

    }
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

