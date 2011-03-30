#ifndef BS_FILESYSTEM_H
#define BS_FILESYSTEM_H

#include "bsConfig.h"

#include <unordered_map>
#include <string>

/**	Creates a map of <fileName, filePath> 
	
*/
class bsFileSystem
{
public:
	bsFileSystem(const char* basePath);
	~bsFileSystem();

	/*	Returns the full path of the file.
		If the file does not exist, "" is returned.
	*/
	std::string getPath(const std::string& fileName) const;

private:
	void buildFileSystem();

	std::string	mBasePath;
	std::unordered_map<std::string, std::string>	mFilePaths;
};

#endif // BS_FILESYSTEM_H
