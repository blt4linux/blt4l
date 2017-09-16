#pragma once

#include <vector>
#include <string>

namespace blt {
    namespace fs {

        std::vector<std::string> list_directory(std::string, bool = false);

        std::string read_file(std::string);

        bool path_is_dir(std::string);

        bool create_directory(std::string);
        
        bool create_file_parent(std::string);

        bool delete_directory(std::string, bool = false);

        std::string hash_string(std::string);

        std::string hash_file(std::string);
    }    
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

