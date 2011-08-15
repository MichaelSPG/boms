#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <string>

#include <tbb/concurrent_queue.h>

#include "bsFileLoader.h"


/*	Manager responsible for handling file load requests.
	Supports both asynchronous (non-blocking) and synchronous (blocking) loading.
	
*/
class bsFileIoManager
{
public:
	typedef std::function<void(const bsFileLoader&)> AsyncCompletionCallback;
	typedef std::pair<std::string, AsyncCompletionCallback> FilenameCallbackPair;


	bsFileIoManager()
		: mQuit(false)
	{}

	~bsFileIoManager()
	{}

	/*	Adds a load request to the queue. The load request will be processed at an
		unspecified time after the moment the function is called.

		Note: The callback function will not be called by the same thread as the one
		calling this function, and is therefore required to be thread safe.
	*/
	inline void addAsynchronousLoadRequest(const std::string& fileName,
		const AsyncCompletionCallback& callback)
	{
		//Make sure it's a not an empty callback.
		//assert(callback);

		mAsynchronousLoadRequests.push(std::move
			(std::make_pair<std::string, AsyncCompletionCallback>(fileName, callback)));
	}

	/*	Loads a file and blocks until it is completely loaded/fails to load.
		You may want to use this function when loading a small object
	*/
	inline bsFileLoader loadBlocking(const std::string& fileName)
	{
		return std::move(bsFileLoader(fileName, bsFileLoader::SYNCHRONOUS, nullptr));
	}

	/*	Makes the thread loop exit, making it possible to join the thread running it
		shortly after calling this function.
		This function does not block, and quitting is not done immediately.
	*/
	inline void quit()
	{
		mQuit = true;
	}

	/*	The thread handling async I/O requests will use this function as its main loop,
		so do not call this function.
	*/
	void threadLoop();

private:
	/*	Processes a single load request. This means creating a new file loader from a pair
		of file name and callback function.
	*/
	void processRequest();

	/*	Removes any load requests that have finished/failed.
	*/
	void removeCompletedRequests();

	/*	Clean up any unfinished async file loaders/unstarted load requests.
	*/
	void shutdown();


	//Async requests only, no synchronous ones.
	tbb::concurrent_queue<FilenameCallbackPair>	mAsynchronousLoadRequests;

	/*	All stored async requests, in no particular order. May contain file loaders whose
		state is completed, failed and incomplete at any given time.
	*/
	std::vector<bsFileLoader*>	mAsynchronousLoaders;

	//Set by quit(), terminates the thread loop.
	bool	mQuit;
};
