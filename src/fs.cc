#include <blt/fs.hh>
#include <blt/log.hh>

#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <streambuf>

namespace blt {
    namespace fs {

        using std::string;
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
            log::log("open dir: " + path, log::LOG_INFO);

            if (directory)
            {
                struct dirent* next = readdir(directory);

                while (next)
                {
                    bool isDir = (next->d_type == DT_DIR);

                    if ((listDirs && isDir) || (!listDirs && !isDir))
                    {
                        result.push_back(next->d_name);
                    }

                    next = readdir(directory);
                }
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
            
            if (stat(path.c_str(), &fileInfo))
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
                        mkdir(pathBuilder.str().c_str(), 0655);
                    }
                }
            }    

            return true;
        }


    }
}

