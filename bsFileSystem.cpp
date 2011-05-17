#include "bsFileSystem.h"

#include <boost/filesystem.hpp>

#include "bsLog.h"
#include "bsAssert.h"


bsFileSystem::bsFileSystem(const std::string& basePath)
	: mBasePath(basePath)
{
	BS_ASSERT2(basePath.length(), "Zero length base path is not ok. Use \".\" (without"
		"quotes for current path");

	buildFileSystem();
}

bsFileSystem::~bsFileSystem()
{
}

void bsFileSystem::buildFileSystem()
{
	boost::filesystem::recursive_directory_iterator itr(mBasePath);
	boost::filesystem::recursive_directory_iterator end;

	//Iterate through all sub directories and all files
	for (; itr != end; ++itr)
	{
		if (boost::filesystem::is_regular_file(itr->path()))
		{
			std::string fileName = itr->path().filename().string();
			std::string pathName = itr->path().relative_path().string();

			//Check if file already exists in the map, and log a warning message if so.
			auto findResult = mFilePaths.find(fileName);
			if (findResult == mFilePaths.end())
			{
				//Add the file and path to the map.
				mFilePaths[fileName] = pathName;
#if BS_DEBUG_LEVEL > 1
				std::string message("Adding file '");
				message += fileName + "' to resource locations (path: '"
					+ pathName + "'";
				bsLog::logMessage(message.c_str(), pantheios::SEV_DEBUG);
#endif
			}
#if BS_DEBUG_LEVEL > 0
			else
			{
				//Duplicate file name name found
				std::string message("Duplicate file found, file name: '");
				message += fileName + "' with path '" + pathName
					+ "'. Will use previously found path '" + findResult->second
					+ "' instead";

				bsLog::logMessage(message.c_str(), pantheios::SEV_WARNING);
			}
#endif
		}
	}
}

std::string bsFileSystem::getPathFromFilename(const std::string& fileName) const
{
	auto findResult = mFilePaths.find(fileName);

	if (findResult == mFilePaths.end())
	{
		//Not found
		return std::string("");
	}
	else
	{
		//Found
		return findResult->second;
	}
}
