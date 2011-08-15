#pragma once

#include <functional>
#include <string>

#include <Windows.h>


/*	A file loader with support for both asynchronous and synchronous file loading.
	Which method to load a file is specified as a constructor argument.

	If using asynchronous loading, a callback must be provided which will be called
	upon loading completion/failure. This callback function is required to be thread safe.
*/
class bsFileLoader
{
public:
	//The method used for loading the file.
	enum LoadingMethod
	{
		//Blocking.
		SYNCHRONOUS = 0,

		//Non-blocking.
		ASYNCHRONOUS = FILE_FLAG_OVERLAPPED,
	};

	enum LoadState
	{
		INCOMPLETE,
		SUCCEEDED,
		FAILED,
	};


	/*	Loads a file from disk, either asynchronously (non-blocking) or
		synchronously (blocking).

		When using asynchronous loading, a completion callback function must be provided.
		The callback function will be called once the loading has completed, or if it has
		failed.
		
		When using synchronous loading, the completion callback parameter is ignored.
		If loading is successful, it will have been loaded once the constructor returns.

		Use getCurrentLoadState and getLoadedData to get the loaded data.
	*/
	bsFileLoader(const std::string& fileName, LoadingMethod loadMethod,
		const std::function<void(const bsFileLoader&)>& completionCallback = nullptr);

	/*	Never use this constructor for an asynchronous file loader, it requires to stay in
		the same memory address while loading.
	*/
	bsFileLoader(bsFileLoader&& other);

	~bsFileLoader();


	/*	Returns the current load state.
		This will be INCOMPLETE if the loader has not yet started/finished/failed, and
		SUCCEEDED or FAILED if it has completed/failed the load.
	*/
	inline LoadState getCurrentLoadState() const
	{
		return mLoadState;
	}

	/*	Gets the data loaded from the file.
		You must make sure that the data has been loaded before calling this
		(with getCurrentLoadState).

		Note: This data will go out of scope when the file loader does, which happens
		at an undefined point in time after the callback function has been called.

		Note: The returned char array is not null terminated. To initialize a std::string,
		do the following:
		std::string(getLoadedData(), getLoadedData() + getLoadedDataSize());
	*/
	inline char* getLoadedData()
	{
		return mData;
	}

	inline const char* getLoadedData() const
	{
		return mData;
	}

	/*	Returns the size of the loaded data, in bytes.
		You must make sure that the data has been loaded before calling this
		(with getCurrentLoadState).
		
		If the data has not been loaded successfully, the value returned is undefined.
	*/
	inline unsigned long getLoadedDataSize() const
	{
		//assert(mLoadState == SUCCEEDED);

		return mDataSize;
	}

	/*	Returns the file handle used for accessing the file.
		You most likely do not need to call this function.
	*/
	inline HANDLE getFileHandle() const
	{
		return mFileHandle;
	}

	/*	Returns the file name used.
	*/
	inline const std::string& getFileName() const
	{
		return mFileName;
	}
	

private:
	/*	Creates a file handle and assigns it to mFileHandle. If creation fails,
		mFileHandle is set to null.
	*/
	void createHandle();

	void loadFileAsync();

	void loadFileBlocking();

	/*	Called by Windows on file load completion/failure.
		Calls the user defined callback function given in the constructor.
	*/
	static void WINAPI loadingFinishedCallback(DWORD errorCode, DWORD numBytesTransfered,
		OVERLAPPED* overlapped);


	//Non-copyable.
	bsFileLoader(const bsFileLoader&);
	bsFileLoader& operator=(const bsFileLoader&);


	LoadState		mLoadState;
	LoadingMethod	mLoadingMethod;

	HANDLE			mFileHandle;
	OVERLAPPED		mOverlapped;
	unsigned long	mDataSize;
	char*			mData;

	const std::string	mFileName;
	const std::function<void(const bsFileLoader&)>	mCompletionCallback;
};
