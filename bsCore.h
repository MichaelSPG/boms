#pragma once

#include <tbb/tbb_thread.h>

#include "bsAssert.h"
#include "bsCoreCInfo.h"
#include "bsFileIoManager.h"

class bsScene;
class bsDx11Renderer;
class bsResourceManager;
class bsHavokManager;
class bsWindow;
class bsRenderQueue;
class bsDeferredRenderer;
struct bsFrameStatistics;


/*	This class contains all the core components of the engine and is responsible for the
	creation and destruction of them.
	
	Use of this class is not required; it is very possible to create all the objects this
	class creates manually without ever touching this class.
*/
class bsCore
{
public:
	bsCore(const bsCoreCInfo& cInfo);

	~bsCore();

	/*	Updates all the component of the engine.
		A render system must be set before calling this function.

		Returns false if the program should exit (Alt+F4 was pressed,
		close button clicked, etc).
	*/
	bool update(float deltaTimeMs, bsFrameStatistics& framStatistics);
	
	inline bsResourceManager* getResourceManager() const
	{
		return mResourceManager;
	}

	inline bsHavokManager* getHavokManager() const
	{
		return mHavokManager;
	}

	inline bsDx11Renderer* getDx11Renderer() const
	{
		return mDx11Renderer;
	}

	inline bsWindow* getWindow() const
	{
		return mWindow;
	}

	inline bsRenderQueue* getRenderQueue() const
	{
		return mRenderQueue;
	}

	inline bsDeferredRenderer* getRenderSystem() const
	{
		return mRenderSystem;
	}

private:
	//Non-copyable.
	bsCore(const bsCore&);
	void operator=(const bsCore&);


	bsWindow*			mWindow;
	bsDx11Renderer*		mDx11Renderer;
	bsResourceManager*	mResourceManager;
	bsHavokManager*		mHavokManager;
	bsRenderQueue*		mRenderQueue;

	bsDeferredRenderer*	mRenderSystem;

	bsCoreCInfo			mCInfo;

	bsFileIoManager		mFileIoManager;
	tbb::tbb_thread*	mFileIoThread;
};
