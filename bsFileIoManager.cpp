#include "StdAfx.h"

#include "bsFileIoManager.h"

#include <algorithm>

#include <Windows.h>

#include "bsLog.h"
#include "bsAssert.h"
#include "bsWindowsUtils.h"
#include "bsFileUtil.h"


void bsFileIoManager::threadLoop()
{
	while (!mQuit)
	{
		if (!mAsynchronousLoaders.empty())
		{
			removeCompletedRequests();
		}

		processRequest();

		SleepEx(1, true);
	}

	//quit() was called, make sure we don't leak memory.
	shutdown();
}

void bsFileIoManager::processRequest()
{
	FilenameCallbackPair asyncFileLoader;
	while (mAsynchronousLoadRequests.try_pop(asyncFileLoader))
	{
		//Create a new async file loader to process the popped request.
		mAsynchronousLoaders.push_back(new bsFileLoader(asyncFileLoader.first,
			bsFileLoader::ASYNCHRONOUS, asyncFileLoader.second));
	};
}

void bsFileIoManager::removeCompletedRequests()
{
	//Check state of all contained loaders, and remove the ones that are no
	//longer running.
	mAsynchronousLoaders.erase(std::remove_if(mAsynchronousLoaders.begin(),
		mAsynchronousLoaders.end(),
		[](const bsFileLoader* fileLoader) -> bool
	{
		const bsFileLoader::LoadState loadState = fileLoader->getCurrentLoadState();
		const bool removeThis = loadState == bsFileLoader::FAILED
			|| loadState == bsFileLoader::SUCCEEDED;

		if (removeThis)
		{
			delete fileLoader;
		}
		return removeThis;
	}), mAsynchronousLoaders.end());
}

void bsFileIoManager::shutdown()
{
	removeCompletedRequests();

	//Cancel any remaining async IO requests and close file handles for all loaders.
	for (size_t i = 0; i < mAsynchronousLoaders.size(); ++i)
	{
		const HANDLE currentHandle = mAsynchronousLoaders[i]->getFileHandle();

		BOOL success = CancelIo(currentHandle);
		if (success == 0)
		{
			bsLog::logf(bsLog::SEV_ERROR, "Cancellation of async I/O failed, error message: %s",
				bsWindowsUtils::winApiErrorCodeToString(GetLastError()).c_str());
		}
		success = CloseHandle(currentHandle);
		if (success == 0)
		{
			bsLog::logf(bsLog::SEV_ERROR, "Closing of handle failed, error message: %s",
				bsWindowsUtils::winApiErrorCodeToString(GetLastError()).c_str());
		}

		delete mAsynchronousLoaders[i];
	}

	mAsynchronousLoaders.clear();
	mAsynchronousLoadRequests.clear();
}

void bsFileIoManager::addAsynchronousLoadRequest(const std::string& fileName,
	const AsyncCompletionCallback& callback)
{
	//Make sure it's a not an empty callback.
	BS_ASSERT2(callback, "Invalid callback");

	BS_ASSERT2(bsFileUtil::fileExists(fileName.c_str()), "Load request for file which does"
		" not exist added");

	mAsynchronousLoadRequests.push(std::move
		(std::make_pair<std::string, AsyncCompletionCallback>(fileName, callback)));
}