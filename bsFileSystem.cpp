#include "bsFileSystem.h"

#include <assert.h>

#include <boost/filesystem.hpp>

#include "bsLog.h"


bsFileSystem::bsFileSystem(const char* basePath)
	: mBasePath(basePath)
{
	assert(basePath);

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
			//std::string fileName = itr->filename();
			//std::string pathName = itr->path().string();
			itr->path().filename().c_str();
			std::string fileName = itr->path().filename().string();
			std::string pathName = itr->path().relative_path().string();

			//Check if file already exists in the map, and log an error message if so.
			auto findResult = mFilePaths.find(fileName);
			if (findResult == mFilePaths.end())
			{
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

std::string bsFileSystem::getPath(const std::string& fileName) const
{
	auto findResult = mFilePaths.find(fileName);

	if (findResult == mFilePaths.end())
	{
		return std::string("");
	}
	else
	{
		return mFilePaths.find(fileName)->second;
	}
}
