#pragma once

#include <zlib.h>
#include <list>
#include <string>
#include <fstream>

namespace blt {
    namespace zip {

        struct ZIPFileData {
            std::string filePath;
            std::string compressedData;
            std::string decompressedData;
            
            int compressedSize;
            int uncompressedSize;
        };

        class ByteStream {
            public:
                ByteStream(std::string);
                ~ByteStream();

                template<typename T> T read_typed();
                std::string read_string(int);
            private:
                std::ifstream mainStream;
        };

        class ZIPArchive {
            public:
                ZIPArchive(std::string, std::string);
                ~ZIPArchive();

                void read_archive();
            private:
                bool read_file();
                bool write_file(ZIPFileData*);
                void decompress_file(ZIPFileData*);

                ByteStream mainStream;
                std::list<ZIPFileData*> files;
                std::string extractTo;
        };

    }
}
