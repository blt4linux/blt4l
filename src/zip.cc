#include <blt/zip.hh>
#include <blt/log.hh>
#include <blt/fs.hh>
#include <iostream>
#include <zlib.h>
#include <string>
#include <iostream>
#include <list>

namespace blt {
    namespace zip {

        using std::string;
        using std::iostream;
        using std::list;
        using std::ofstream;
        
        /**
         * ByteStream Implementation
         */

        ByteStream::ByteStream(string path)
        {
            mainStream.open(path.c_str(), std::ios::binary);
        }

        ByteStream::~ByteStream() 
        {
            mainStream.close();
        }

        template<typename T> T
        ByteStream::read_typed()
        {
            T read;
            mainStream.read((char*)&read, sizeof(T));
            return read;
        }

        string
        ByteStream::read_string(int length)
        {
            char* readData = new char[length + 1];
            mainStream.read(readData, length);
            string strData(readData, length);
            delete[] readData;
            return strData;
        }

        /**
         * ZIPArchive Impl
         */

        ZIPArchive::ZIPArchive(string path, string extractPath) 
            : mainStream(path)
            , files()
            , extractTo(extractPath)
        {
        }

        ZIPArchive::~ZIPArchive()
        {
            list<ZIPFileData*>::iterator it;
            for (it = files.begin();
                 it != files.end();
                 ++it)
            {
                delete *it;
            }
        }

        void
        ZIPArchive::read_archive()
        {
            while (read_file());
            
            list<ZIPFileData*>::iterator it;
            for (it = files.begin();
                 it != files.end();
                 ++it)
            {
                write_file(*it);
            }
        }

        bool
        ZIPArchive::read_file()
        {
            // Format followed from: https://en.wikipedia.org/wiki/Zip_(file_format)#File_headers

            int fileHeader = mainStream.read_typed<int32_t>();

            if (fileHeader != 0x04034B50)
            {
                return false;
            }

            mainStream.read_typed<int16_t>(); // minimum version
            mainStream.read_typed<int16_t>(); // GPBF

            int compressionMethod = mainStream.read_typed<int16_t>();

            mainStream.read_typed<int16_t>(); // mtime
            mainStream.read_typed<int16_t>(); // mdate

            int crc32 = mainStream.read_typed<int32_t>();

            ZIPFileData* newFile = new ZIPFileData();
            {
                newFile->compressedSize   = mainStream.read_typed<int32_t>();
                newFile->uncompressedSize = mainStream.read_typed<int32_t>();
            }

            int fileNameLen     = mainStream.read_typed<int16_t>();
            int extraFieldLen   = mainStream.read_typed<int16_t>();

            newFile->filePath = mainStream.read_string(fileNameLen);
            string exraField = mainStream.read_string(extraFieldLen);

            newFile->compressedData = mainStream.read_string(newFile->compressedSize);

            switch (compressionMethod)
            {
                case 0:
                    newFile->decompressedData = newFile->compressedData;
                    break;

                case 8:
                    decompress_file(newFile);
                    break;
            }

            files.push_back(newFile);
            return true;
        }

        bool
        ZIPArchive::write_file(ZIPFileData* data)
        {
            string writePath = extractTo + "/" + data->filePath;
            log::log("Extracting " + writePath, log::LOG_INFO);

            fs::create_file_parent(writePath);

            ofstream outFile;
            outFile.open(writePath.c_str(), std::ios::out | std::ios::binary /* no point in this on linux, keeping for reasons */);

            if (!outFile.good())
            {
                outFile.close();
                return false;
            }

            outFile.write(data->decompressedData.c_str(), data->uncompressedSize);
            outFile.close();
            return true;
        }

        void
        ZIPArchive::decompress_file(ZIPFileData* data)
        {
            z_stream stream;
            {
                stream.zalloc   = Z_NULL;
                stream.zfree    = Z_NULL;
                stream.opaque   = Z_NULL;
                stream.avail_in = 0;
                stream.next_in  = Z_NULL;
            }

            int ret = inflateInit2(&stream, -MAX_WBITS);

            stream.avail_in = data->compressedSize;
            stream.next_in = (unsigned char*) data->compressedData.c_str();

            unsigned char* out = new unsigned char[data->uncompressedSize + 1];

            stream.avail_out = data->uncompressedSize;
            stream.next_out = out;

            ret = inflate(&stream, Z_NO_FLUSH);
            inflateEnd(&stream);

            data->decompressedData = string((char*) out, data->uncompressedSize);

            delete[] out;
        }


    }
}

