#ifndef COMPRESSION_H
#define COMPRESSION_H
/*
#include <iostream>
#include <fstream>

class ByteStream {
public:
	ByteStream(std::string path);
	~ByteStream();

	template<typename T>
	T readType();
	std::string readString(int length);
private:

	std::ifstream mainStream;
};

struct ZIPFileData {
	std::string filepath;
	std::string compressedData;
	std::string decompressedData;
	int compressedSize;
	int uncompressedSize;
};

class ZIPArchive {
public:
	ZIPArchive(std::string path, std::string extractPath);
	~ZIPArchive();
	void ReadArchive();
private:

	bool ReadFile();
	bool WriteFile(ZIPFileData* data);
	void DecompressFile(ZIPFileData* fileToDecompress);

	ByteStream mainStream;
	std::list<ZIPFileData*> readFiles;
	std::string extractTo;
};
*/
#endif // COMPRESSION_H
