#pragma once

#include <vector>
#include <string>

namespace blt {
    namespace fs {

        std::vector<std::string> list_directory(std::string, bool = false);

        std::string read_file(std::string);

        bool path_is_dir(std::string);

        bool create_directory(std::string);
    
    }
}
