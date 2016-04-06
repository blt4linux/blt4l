#include <blt/files.h>
#include <blt/logging.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>

#include <fstream>
#include <streambuf>

using namespace std;

namespace Files
{
	int GetDir(string dir, vector<string> &files)
	{
		DIR *pDir;
		struct dirent *pDirent;
		if((pDir = opendir(dir.c_str())) == NULL)
		{
				Logging::Log(static_cast<std::ostringstream*>( &(std::ostringstream() << errno) )->str() + " opening " + dir, Logging::LOGGING_ERROR);
				return errno;
		}

		while((pDirent = readdir(pDir)) != NULL)
		{
				files.push_back(string(pDirent->d_name));
		}
		closedir(pDir);
		return 0;
	}

	vector<string> GetDirectoryContents(string path, bool dirs)
	{
		vector<string> files = vector<string>();

		GetDir(path, files);

		return files;
	}

	string GetFileContents(string filename)
	{
		ifstream file(filename.c_str(), ios::binary);
		streambuf* raw_buffer = file.rdbuf();
		size_t size = file.tellg();

		char* block = new char[size];
		raw_buffer->sgetn(block, size);
		std::string str((istreambuf_iterator<char>(raw_buffer)), istreambuf_iterator<char>());
		delete[] block;

		return str;
	}

	bool DirectoryExists(string dir)
	{
		DIR *pDir;
		bool bExists = false;

		pDir = opendir(dir.c_str());

		if(pDir != NULL)
		{
			bExists = true;
			(void)closedir(pDir);
		}

		return bExists;
	}

	void EnsurePathWritable(string path)
	{
		int finalSlash = path.find_last_of('/');
		string finalPath = path.substr(0, finalSlash);
		if (DirectoryExists(finalPath))
			return;

		CreateDirectoryPath(finalPath.c_str());
	}

	bool RemoveEmptyDirectory(string dir)
	{
		if (remove(dir.c_str()) != -1)
			return true;

		return false;
	}

	bool CreateDirectoryPath(string path)
	{
		string newPath = "";
		vector<string> paths = Files::SplitString(path.c_str(), '/');
		for (vector<string>::const_iterator i = paths.begin(); i != paths.end(); ++i)
		{
			newPath = newPath + *i + "/";
			mkdir(newPath.c_str(), 0777);
		}
		return true;
	}

	vector<string> &SplitString(const string &s, char delim, vector<string> &elems)
	{
		stringstream ss(s);
		string item;
		while (getline(ss, item, delim))
		{
			if (!item.empty())
				elems.push_back(item);

		}
		return elems;
	}

	vector<string> SplitString(const string &s, char delim)
	{
		vector<string> elems;
		SplitString(s, delim, elems);
		return elems;
	}
}
