#include "bsFileLoader.h"

#include <string>

#include "bsWindowsUtils.h"


bsFileLoader::bsFileLoader(const std::string& fileName, LoadingMethod loadingMethod,
	const std::function<void(const bsFileLoader&)>& completionCallback)
	: mLoadState(INCOMPLETE)
	, mLoadingMethod(loadingMethod)
	, mFileHandle(nullptr)
	, mDataSize(0)
	, mFileName(fileName)
	, mCompletionCallback(completionCallback)
{
	//assert(!fileName.empty());

	//assert(loadingMethod == BLOCKING || loadingMethod == ASYNCHRONOUS && "Invalid loading method.");
	if (mLoadingMethod == ASYNCHRONOUS)
	{
		//assert(completionCallback && "A callback function must be provided when loading asynchronously.");
	}

	memset(&mOverlapped, 0, sizeof(mOverlapped));

	createHandle();

	if (mFileHandle == nullptr)
	{
		//Invalid handle, can't use for loading
		mCompletionCallback(*this);

		return;
	}

	if (mLoadingMethod == ASYNCHRONOUS)
	{
		loadFileAsync();
	}
	else
	{
		loadFileBlocking();
	}
}

bsFileLoader::bsFileLoader(bsFileLoader&& other)
	: mLoadState(other.mLoadState)
	, mLoadingMethod(other.mLoadingMethod)
	, mFileHandle(nullptr)//Already closed.
	, mOverlapped()//Unused for synchronous.
	, mDataSize(other.mDataSize)
	, mData(other.mData)
	, mFileName(std::move(other.mFileName))
	, mCompletionCallback(nullptr)//Unused for synchronous.
{
	//Move constructor is not valid for async loading, since it requires the 'this'
	//pointer's address to be constant while the loading is being performed.
	//assert(other.mLoadingMethod == SYNCHRONOUS);

	other.mDataSize = 0;
	other.mData = nullptr;
}

bsFileLoader::~bsFileLoader()
{
	free(mData);
}

void bsFileLoader::createHandle()
{
	//Contains the WinAPI overlapped flag only when using asynchronous loading.
	const unsigned long overlappedFlag =
		(mLoadingMethod == ASYNCHRONOUS) ? FILE_FLAG_OVERLAPPED : 0;
	
	mFileHandle = CreateFileA(mFileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY | overlappedFlag, nullptr);

	if (mFileHandle == INVALID_HANDLE_VALUE)
	{
		//Probably invalid file/not permission to access.

		printf("Failed to create file handle: %s", errorCodeToString(GetLastError()).c_str());

		mLoadState = FAILED;

		CloseHandle(mFileHandle);

		mFileHandle = nullptr;
	}
}

void bsFileLoader::loadFileAsync()
{
	LARGE_INTEGER fileSize;
	const BOOL fileSizeSuccess = GetFileSizeEx(mFileHandle, &fileSize);

	if (fileSizeSuccess != 0)
	{
		if (fileSize.LowPart == 0)
		{
			//Empty file, can skip the ReadFile call and avoid calling operator[] on a
			//zero element vector.
			mLoadState = SUCCEEDED;
			mDataSize = 0;
			CloseHandle(mFileHandle);

			return;
		}

		/*	MSDN says: The ReadFileEx function ignores the OVERLAPPED structure's hEvent
			member. An application is free to use that member for its own purposes in the
			context of a ReadFileEx call.
			http://msdn.microsoft.com/en-us/library/aa365468%28v=vs.85%29.aspx

			Using it as a way to get 'this' from the callback function here.
		*/
		mOverlapped.hEvent = this;

		//Only using LowPart, file sizes over 4 GB will not work.
		mData = static_cast<char*>(malloc(fileSize.LowPart));

		const BOOL readFileSuccess = ReadFileEx(mFileHandle, mData, fileSize.LowPart,
			&mOverlapped, &loadingFinishedCallback);

		if (readFileSuccess == 0)
		{
			printf("Failed to read file: %s", errorCodeToString(GetLastError()).c_str());

			mLoadState = FAILED;
			mCompletionCallback(*this);
		}
	}
}

void bsFileLoader::loadFileBlocking()
{
	LARGE_INTEGER fileSize;
	const BOOL fileSizeSuccess = GetFileSizeEx(mFileHandle, &fileSize);

	if (fileSizeSuccess != 0)
	{
		if (fileSize.LowPart == 0)
		{
			//Empty file, can skip the ReadFile call and avoid calling operator[] on a
			//zero element vector.
			mLoadState = SUCCEEDED;
			mDataSize = 0;
			CloseHandle(mFileHandle);

			return;
		}

		//Only using LowPart, file sizes over 4 GB will not work.
		mData = static_cast<char*>(malloc(fileSize.LowPart));

		//ReadFile blocks until it completes.
		const BOOL readFileSuccess = ReadFile(mFileHandle, mData, fileSize.LowPart,
			&mDataSize, nullptr);

		if (readFileSuccess == 0)
		{
			printf("Failed to read file: %s", errorCodeToString(GetLastError()).c_str());

			mLoadState = FAILED;
		}
		else
		{
			mLoadState = SUCCEEDED;
		}
	}

	CloseHandle(mFileHandle);
}

void WINAPI bsFileLoader::loadingFinishedCallback(DWORD errorCode, DWORD numBytesTransfered,
	OVERLAPPED* overlapped)
{
	//Stored 'this' in hEvent in loadFileAsync, cast it back to use it.
	bsFileLoader& loader = *static_cast<bsFileLoader*>(overlapped->hEvent);
	CloseHandle(loader.mFileHandle);

	loader.mDataSize = numBytesTransfered;

	if (errorCode != 0)
	{
		//Something went wrong while loading.

		printf(errorCodeToString(errorCode).c_str());

		loader.mLoadState = FAILED;
		loader.mCompletionCallback(loader);

		return;
	}

	//Get the result of the overlapped load operation.
	const BOOL overlappedResultSuccess = GetOverlappedResult(loader.mFileHandle,
		&loader.mOverlapped, &loader.mDataSize, true);

	if (overlappedResultSuccess == 0)
	{
		//Overlapped operation failed.

		printf(errorCodeToString(GetLastError()).c_str());

		loader.mLoadState = FAILED;
		loader.mCompletionCallback(loader);

		return;
	}

	//All went well.
	loader.mLoadState = SUCCEEDED;
	loader.mCompletionCallback(loader);
}
