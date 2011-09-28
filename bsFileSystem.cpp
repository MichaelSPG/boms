#include "StdAfx.h"

#include "bsFileSystem.h"

#include <boost/filesystem.hpp>

#include "bsLog.h"
#include "bsAssert.h"


bsFileSystem::bsFileSystem(const std::string& basePath)
	: mBasePath(basePath)
{
	BS_ASSERT2(mBasePath.length(), "Zero length base path is not OK.");

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
			}
			else
			{
				//Duplicate file name name found, which is probably not intended.
				std::string message("Duplicate file found, file name: '");
				message += fileName + "' with path '" + pathName
					+ "'. Will use previously found path '" + findResult->second
					+ "' instead";

				bsLog::logMessage(message.c_str(), bsLog::SEV_WARNING);
			}
		}
	}
}

std::string bsFileSystem::getPathFromFilename(const std::string& fileName) const
{
	auto findResult = mFilePaths.find(fileName);

	//Return empty string if not found, else the actual path
	return findResult == mFilePaths.end() ? "" : findResult->second;
}
