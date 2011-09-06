#pragma once


#include <unordered_map>
#include <string>

/*	Contains a map of files, using the filename as the key and the full/relative path
	as the value.
	This makes it possible to refer to something like "assets\images\forest\tree.jpg" as
	just "tree.jpg" and the file system will be able to convert the filename to the
	full/relative path.
*/
class bsFileSystem
{
public:
	/*	The base path is the root of the scanned directories.
		All subfolders of the base path will be scanned, and every contained file will
		be mapped.
	*/
	bsFileSystem(const std::string& basePath);

	~bsFileSystem();

	/*	Returns the full path of the file.
		If the file does not exist, "" is returned.
	*/
	std::string getPathFromFilename(const std::string& fileName) const;

private:
	/*	Builds the file system by scanning the base path and all the folders it contains
		and maps file names to file paths.
	*/
	void buildFileSystem();


	std::string	mBasePath;
	std::unordered_map<std::string, std::string>	mFilePaths;
};
