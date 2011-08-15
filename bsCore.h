#pragma once

#include "bsConfig.h"

#include <tbb/tbb_thread.h>

#include "bsAssert.h"
#include "bsCoreCInfo.h"
#include "bsFileIoManager.h"

class bsSceneGraph;
class bsDx11Renderer;
class bsResourceManager;
class bsHavokManager;
class bsWindow;
class bsRenderQueue;
class bsRenderSystem;


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
	bool update(float deltaTimeMs);
	
	inline bsSceneGraph* getSceneGraph() const
	{
		return mSceneGraph;
	}

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

	inline bsRenderSystem* getRenderSystem() const
	{
		return mRenderSystem;
	}

	/*	Sets the render system which should be used for rendering the scene.
		This must be set before calling update().
	*/
	inline void setRenderSystem(bsRenderSystem* renderSystem)
	{
		BS_ASSERT(renderSystem);

		mRenderSystem = renderSystem;
	}

private:
	//Non-copyable.
	bsCore(const bsCore&);
	void operator=(const bsCore&);


	bsWindow*			mWindow;
	bsDx11Renderer*		mDx11Renderer;
	bsSceneGraph*		mSceneGraph;
	bsResourceManager*	mResourceManager;
	bsHavokManager*		mHavokManager;
	bsRenderQueue*		mRenderQueue;

	bsRenderSystem*		mRenderSystem;

	bsCoreCInfo			mCInfo;

	bsFileIoManager		mFileIoManager;
	tbb::tbb_thread		mFileIoThread;
};
