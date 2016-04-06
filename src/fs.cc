#include <blt/fs.hh>
#include <blt/log.hh>
#include <dirent.h>
#include <string>
#include <vector>

namespace blt {
    namespace fs {

        using std::string;
        using std::vector;
        
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

    }
}

