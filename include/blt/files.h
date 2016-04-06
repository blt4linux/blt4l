#ifndef FILES_H
#define FILES_H

#include <vector>
#include <string>
#include <sstream>

namespace Files {
	std::vector<std::string> GetDirectoryContents(std::string path, bool isDirs = false);
	std::string GetFileContents(std::string filename);
	void EnsurePathWritable(std::string path);
	bool RemoveEmptyDirectory(std::string dir);
	bool DirectoryExists(std::string dir);
	bool CreateDirectoryPath(std::string dir);
	// String split from http://stackoverflow.com/a/236803
	std::vector<std::string> &SplitString(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> SplitString(const std::string &s, char delim);
}

#endif // FILES_H
