#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "bsConfig.h"

#include <unordered_map>
#include <string>

/**	Creates a map of <fileName, filePath> 
	
*/
class FileSystem
{
public:
	FileSystem(const char* basePath);
	~FileSystem();

	/*	Returns the full path of the file.
		If the file does not exist, "" is returned.
	*/
	std::string getPath(const std::string& fileName) const;

private:
	void buildFileSystem();

	std::string	mBasePath;
	std::unordered_map<std::string, std::string>	mFilePaths;
};

#endif // FILESYSTEM_H