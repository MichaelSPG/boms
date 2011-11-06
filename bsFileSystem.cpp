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
				
				bsLog::logf(bsLog::SEV_WARNING, "Duplicate file found, file name: '%s'"
					" with path '%s'. Will use previously found path '%s' instead",
					fileName.c_str(), pathName.c_str(), findResult->second.c_str());
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
